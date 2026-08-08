#include "playback_probe_runner.h"

#include <QMetaObject>
#include <QTextStream>

#include <variant>

namespace player::tools::playback_probe {
namespace {

constexpr quint64 kLoadRequestId = 1;
constexpr quint64 kPauseRequestId = 2;
constexpr quint64 kPlayRequestId = 3;
constexpr quint64 kSeekRequestId = 4;
constexpr quint64 kStopRequestId = 5;
constexpr int kStepTimeoutMs = 15000;

QString endReasonName(player::playback::mpv::MpvEndFileReason reason)
{
    using player::playback::mpv::MpvEndFileReason;

    switch (reason) {
    case MpvEndFileReason::Eof:
        return QStringLiteral("eof");
    case MpvEndFileReason::Stop:
        return QStringLiteral("stop");
    case MpvEndFileReason::Quit:
        return QStringLiteral("quit");
    case MpvEndFileReason::Error:
        return QStringLiteral("error");
    case MpvEndFileReason::Redirect:
        return QStringLiteral("redirect");
    case MpvEndFileReason::Unknown:
        return QStringLiteral("unknown");
    }

    return QStringLiteral("unknown");
}

QString commandName(quint64 requestId)
{
    switch (requestId) {
    case kLoadRequestId:
        return QStringLiteral("load");
    case kPauseRequestId:
        return QStringLiteral("pause");
    case kPlayRequestId:
        return QStringLiteral("play");
    case kSeekRequestId:
        return QStringLiteral("seek");
    case kStopRequestId:
        return QStringLiteral("stop");
    default:
        return QStringLiteral("unknown");
    }
}

} // namespace

PlaybackProbeRunner::PlaybackProbeRunner(QObject* parent)
    : QObject(parent)
{
    stepTimer_.setSingleShot(true);
    stepTimer_.setInterval(kStepTimeoutMs);

    connect(&runtime_, &PlaybackProbeRuntime::eventDecoded, this, &PlaybackProbeRunner::handleEvent);
    connect(&stepTimer_, &QTimer::timeout, this, &PlaybackProbeRunner::handleStepTimeout);
}

bool PlaybackProbeRunner::start(const QString& source, QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (phase_ != Phase::Idle || finished_) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback probe can only be started once.");
        }
        return false;
    }

    if (source.trimmed().isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Media source must not be empty.");
        }
        return false;
    }

    QString error;
    if (!runtime_.initialize(&error)) {
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    log(QStringLiteral("initialized headless libmpv core (config=no, vo=null, ao=null)"));
    if (!submit(
            kLoadRequestId,
            player::playback::mpv::MpvLoadRequest{source},
            QStringLiteral("load"),
            Phase::Loading,
            &error)) {
        runtime_.shutdown();
        if (errorMessage != nullptr) {
            *errorMessage = error;
        }
        return false;
    }

    return true;
}

void PlaybackProbeRunner::handleEvent(const player::playback::mpv::MpvEvent& event)
{
    using namespace player::playback::mpv;

    if (finished_) {
        return;
    }

    switch (event.type) {
    case MpvEventType::StartFile:
        log(QStringLiteral("event start-file"));
        return;

    case MpvEventType::FileLoaded: {
        fileLoaded_ = true;
        log(QStringLiteral("event file-loaded"));
        if (phase_ == Phase::WaitingForFileLoaded) {
            QString error;
            if (!submitPlay(&error)) {
                fail(error);
            }
        }
        return;
    }

    case MpvEventType::EndFile: {
        const auto* endData = std::get_if<MpvEndFileData>(&event.payload);
        if (endData == nullptr) {
            fail(QStringLiteral("end-file event was missing typed payload data."));
            return;
        }

        const QString reason = endReasonName(endData->reason);
        log(QStringLiteral("event end-file reason=%1").arg(reason));

        if (endData->reason == MpvEndFileReason::Error) {
            fail(QStringLiteral("media ended with an error: %1")
                     .arg(event.error.message.isEmpty()
                              ? QStringLiteral("unknown libmpv load/playback error")
                              : event.error.message));
            return;
        }

        if (phase_ == Phase::Stopping || phase_ == Phase::WaitingForEndFile) {
            endFileReceived_ = true;
            if (stopReplyReceived_) {
                succeed();
            }
            return;
        }

        if (endData->reason == MpvEndFileReason::Eof) {
            fail(QStringLiteral("media reached EOF before pause/play/seek/stop completed."));
            return;
        }

        fail(QStringLiteral("unexpected end-file event before probe completion: %1").arg(reason));
        return;
    }

    case MpvEventType::CommandReply: {
        const QString name = commandName(event.replyUserdata);
        if (!event.error.isSuccess()) {
            fail(QStringLiteral("%1 command failed: %2")
                     .arg(name, event.error.message.isEmpty()
                                    ? QStringLiteral("unknown libmpv command error")
                                    : event.error.message));
            return;
        }

        log(QStringLiteral("command-reply %1 ok").arg(name));
        QString error;

        switch (event.replyUserdata) {
        case kLoadRequestId:
            if (phase_ != Phase::Loading) {
                fail(QStringLiteral("load command reply arrived in an invalid probe phase."));
            } else if (!submitPause(&error)) {
                fail(error);
            }
            return;

        case kPauseRequestId:
            if (phase_ != Phase::Pausing) {
                fail(QStringLiteral("pause command reply arrived in an invalid probe phase."));
            } else if (fileLoaded_) {
                if (!submitPlay(&error)) {
                    fail(error);
                }
            } else {
                phase_ = Phase::WaitingForFileLoaded;
                armTimeout(QStringLiteral("file-loaded"));
                log(QStringLiteral("waiting for file-loaded after pause"));
            }
            return;

        case kPlayRequestId:
            if (phase_ != Phase::Playing) {
                fail(QStringLiteral("play command reply arrived in an invalid probe phase."));
            } else if (!submitSeek(&error)) {
                fail(error);
            }
            return;

        case kSeekRequestId:
            if (phase_ != Phase::Seeking) {
                fail(QStringLiteral("seek command reply arrived in an invalid probe phase."));
            } else if (!submitStop(&error)) {
                fail(error);
            }
            return;

        case kStopRequestId:
            if (phase_ != Phase::Stopping && phase_ != Phase::WaitingForEndFile) {
                fail(QStringLiteral("stop command reply arrived in an invalid probe phase."));
                return;
            }
            stopReplyReceived_ = true;
            if (endFileReceived_) {
                succeed();
            } else {
                phase_ = Phase::WaitingForEndFile;
                armTimeout(QStringLiteral("end-file after stop"));
                log(QStringLiteral("waiting for end-file after stop"));
            }
            return;

        default:
            fail(QStringLiteral("received command reply for unexpected request id %1.")
                     .arg(event.replyUserdata));
            return;
        }
    }

    case MpvEventType::DecodeFailure: {
        const auto* failure = std::get_if<MpvDecodeFailureData>(&event.payload);
        fail(failure == nullptr
                 ? QStringLiteral("libmpv event decoding failed without diagnostic payload.")
                 : QStringLiteral("libmpv event decoding failed: %1").arg(failure->diagnostic));
        return;
    }

    case MpvEventType::Shutdown:
        fail(QStringLiteral("libmpv shutdown arrived before probe completion."));
        return;

    case MpvEventType::PropertyChange:
    case MpvEventType::LogMessage:
    case MpvEventType::Unknown:
        return;
    }
}

void PlaybackProbeRunner::handleStepTimeout()
{
    fail(QStringLiteral("timed out after %1 ms while waiting for %2.")
             .arg(kStepTimeoutMs)
             .arg(currentStep_.isEmpty() ? QStringLiteral("the current probe step") : currentStep_));
}

bool PlaybackProbeRunner::submit(
    quint64 requestId,
    const player::playback::mpv::MpvCommandRequest& request,
    const QString& stepName,
    Phase phase,
    QString* errorMessage)
{
    QString error;
    if (!runtime_.submit(requestId, request, &error)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("failed to submit %1 command: %2").arg(stepName, error);
        }
        return false;
    }

    phase_ = phase;
    armTimeout(stepName);
    log(QStringLiteral("submitted %1").arg(stepName));
    return true;
}

bool PlaybackProbeRunner::submitPause(QString* errorMessage)
{
    return submit(
        kPauseRequestId,
        player::playback::mpv::MpvPauseRequest{},
        QStringLiteral("pause"),
        Phase::Pausing,
        errorMessage);
}

bool PlaybackProbeRunner::submitPlay(QString* errorMessage)
{
    return submit(
        kPlayRequestId,
        player::playback::mpv::MpvPlayRequest{},
        QStringLiteral("play"),
        Phase::Playing,
        errorMessage);
}

bool PlaybackProbeRunner::submitSeek(QString* errorMessage)
{
    return submit(
        kSeekRequestId,
        player::playback::mpv::MpvSeekRequest{0.25, player::playback::mpv::MpvSeekMode::Relative},
        QStringLiteral("seek +0.25s"),
        Phase::Seeking,
        errorMessage);
}

bool PlaybackProbeRunner::submitStop(QString* errorMessage)
{
    return submit(
        kStopRequestId,
        player::playback::mpv::MpvStopRequest{},
        QStringLiteral("stop"),
        Phase::Stopping,
        errorMessage);
}

void PlaybackProbeRunner::armTimeout(const QString& stepName)
{
    currentStep_ = stepName;
    stepTimer_.start();
}

void PlaybackProbeRunner::log(const QString& message) const
{
    QTextStream output(stdout);
    output << "[playback_probe] " << message << Qt::endl;
}

void PlaybackProbeRunner::fail(const QString& message, int exitCode)
{
    if (finished_) {
        return;
    }

    finished_ = true;
    phase_ = Phase::Finished;
    stepTimer_.stop();

    QTextStream error(stderr);
    error << "[playback_probe] FAIL: " << message << Qt::endl;

    finalize(exitCode);
}

void PlaybackProbeRunner::succeed()
{
    if (finished_) {
        return;
    }

    finished_ = true;
    phase_ = Phase::Finished;
    stepTimer_.stop();
    log(QStringLiteral("PASS: load -> pause -> play -> seek -> stop -> end-file -> close"));

    finalize(0);
}

void PlaybackProbeRunner::finalize(int exitCode)
{
    QMetaObject::invokeMethod(
        this,
        [this, exitCode]() {
            runtime_.shutdown();
            emit finished(exitCode);
        },
        Qt::QueuedConnection);
}

} // namespace player::tools::playback_probe
