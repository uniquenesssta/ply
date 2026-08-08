#include "playback_probe_scenario_runner.h"

#include <QDir>
#include <QMetaObject>

#include <variant>

namespace player::tools::playback_probe {
namespace {

template <typename Predicate>
bool takeFirstMatching(
    QList<player::playback::mpv::MpvEvent>& events,
    Predicate&& predicate)
{
    for (qsizetype index = 0; index < events.size(); ++index) {
        if (!predicate(events.at(index))) {
            continue;
        }
        events.removeAt(index);
        return true;
    }
    return false;
}

bool stringValuesMatch(
    player::playback::mpv::MpvPropertyId propertyId,
    const QString& actual,
    const QString& expected)
{
    if (propertyId != player::playback::mpv::MpvPropertyId::Path) {
        return actual == expected;
    }

    return QDir::cleanPath(QDir::fromNativeSeparators(actual))
        == QDir::cleanPath(QDir::fromNativeSeparators(expected));
}

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

} // namespace

PlaybackProbeScenarioRunner::PlaybackProbeScenarioRunner(QObject* parent)
    : QObject(parent)
{
    stepTimer_.setSingleShot(true);
    stepTimer_.setInterval(kStepTimeoutMs);

    connect(&runtime_, &PlaybackProbeRuntime::eventDecoded, this, &PlaybackProbeScenarioRunner::handleEvent);
    connect(&stepTimer_, &QTimer::timeout, this, &PlaybackProbeScenarioRunner::handleStepTimeout);
}

bool PlaybackProbeScenarioRunner::start(
    const PlaybackProbeScenario& scenario,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (running_ || finalizing_) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("A playback probe scenario is already running.");
        }
        return false;
    }
    if (scenario.id.trimmed().isEmpty() || scenario.steps.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Playback probe scenario must have an id and at least one step.");
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

    scenario_ = scenario;
    pendingEvents_.clear();
    submittedRequestIds_.clear();
    stepIndex_ = 0;
    currentStep_.clear();
    running_ = true;
    finalizing_ = false;
    trace_.reset(scenario_.id);
    trace_.recordMarker(QStringLiteral("runtime initialized: config=no vo=null ao=null"));

    advance();
    return true;
}

void PlaybackProbeScenarioRunner::handleEvent(const player::playback::mpv::MpvEvent& event)
{
    using namespace player::playback::mpv;

    trace_.recordEvent(event);
    if (finalizing_ || !running_) {
        return;
    }

    if (event.type == MpvEventType::DecodeFailure) {
        const auto* failure = std::get_if<MpvDecodeFailureData>(&event.payload);
        fail(failure == nullptr
                 ? QStringLiteral("libmpv event decoding failed without diagnostic payload.")
                 : QStringLiteral("libmpv event decoding failed: %1").arg(failure->diagnostic));
        return;
    }

    if (event.type == MpvEventType::Shutdown) {
        fail(QStringLiteral("libmpv shutdown event arrived before the scripted shutdown boundary."));
        return;
    }

    if (event.type == MpvEventType::CommandReply) {
        if (!submittedRequestIds_.contains(event.replyUserdata)) {
            fail(QStringLiteral("received command reply for unknown request id %1.")
                     .arg(event.replyUserdata));
            return;
        }
        if (!event.error.isSuccess()) {
            fail(QStringLiteral("command request %1 failed: %2")
                     .arg(event.replyUserdata)
                     .arg(event.error.message.isEmpty()
                              ? QStringLiteral("unknown libmpv command error")
                              : event.error.message));
            return;
        }
    }

    if (event.type == MpvEventType::EndFile) {
        const auto* endFile = std::get_if<MpvEndFileData>(&event.payload);
        if (endFile == nullptr) {
            fail(QStringLiteral("end-file event was missing typed payload data."));
            return;
        }
        if (!scenarioExpectsEndReason(endFile->reason)) {
            fail(QStringLiteral("unexpected end-file reason '%1' in scenario '%2': %3")
                     .arg(
                         endReasonName(endFile->reason),
                         scenario_.id,
                         event.error.message.isEmpty() ? QStringLiteral("no diagnostic")
                                                       : event.error.message));
            return;
        }
    }

    if (pendingEvents_.size() >= kMaximumPendingEvents) {
        pendingEvents_.removeFirst();
        trace_.recordMarker(QStringLiteral("oldest pending event dropped at bounded queue limit"));
    }
    pendingEvents_.append(event);
    advance();
}

void PlaybackProbeScenarioRunner::handleStepTimeout()
{
    fail(QStringLiteral("timed out after %1 ms while waiting for '%2' in scenario '%3'.")
             .arg(kStepTimeoutMs)
             .arg(currentStep_.isEmpty() ? QStringLiteral("current step") : currentStep_)
             .arg(scenario_.id));
}

void PlaybackProbeScenarioRunner::advance()
{
    if (!running_ || finalizing_) {
        return;
    }

    while (stepIndex_ < scenario_.steps.size()) {
        const PlaybackProbeStep& step = scenario_.steps.at(stepIndex_);

        if (const auto* submitStep = std::get_if<ProbeSubmitCommandStep>(&step)) {
            if (submitStep->requestId == 0 || submittedRequestIds_.contains(submitStep->requestId)) {
                fail(QStringLiteral("scenario '%1' contains an invalid or duplicate request id %2.")
                         .arg(scenario_.id)
                         .arg(submitStep->requestId));
                return;
            }

            QString error;
            trace_.recordCommand(submitStep->requestId, submitStep->label);
            submittedRequestIds_.insert(submitStep->requestId);
            if (!runtime_.submit(submitStep->requestId, submitStep->request, &error)) {
                submittedRequestIds_.remove(submitStep->requestId);
                fail(QStringLiteral("unable to submit '%1': %2").arg(submitStep->label, error));
                return;
            }
            ++stepIndex_;
            continue;
        }

        if (const auto* replyStep = std::get_if<ProbeWaitCommandReplyStep>(&step)) {
            if (takeCommandReply(replyStep->requestId)) {
                stepTimer_.stop();
                currentStep_.clear();
                ++stepIndex_;
                continue;
            }
            armTimeout(replyStep->label);
            return;
        }

        if (const auto* eventStep = std::get_if<ProbeWaitEventStep>(&step)) {
            if (takeEvent(*eventStep)) {
                stepTimer_.stop();
                currentStep_.clear();
                ++stepIndex_;
                continue;
            }
            armTimeout(eventStep->label);
            return;
        }

        if (const auto* flagStep = std::get_if<ProbeWaitFlagPropertyStep>(&step)) {
            if (takeFlagProperty(*flagStep)) {
                stepTimer_.stop();
                currentStep_.clear();
                ++stepIndex_;
                continue;
            }
            armTimeout(flagStep->label);
            return;
        }

        if (const auto* stringStep = std::get_if<ProbeWaitStringPropertyStep>(&step)) {
            if (takeStringProperty(*stringStep)) {
                stepTimer_.stop();
                currentStep_.clear();
                ++stepIndex_;
                continue;
            }
            armTimeout(stringStep->label);
            return;
        }

        if (const auto* barrierStep = std::get_if<ProbeEventBarrierStep>(&step)) {
            stepTimer_.stop();
            currentStep_.clear();
            pendingEvents_.clear();
            trace_.recordMarker(QStringLiteral("event-barrier: %1").arg(barrierStep->label));
            ++stepIndex_;
            continue;
        }

        const auto* shutdownStep = std::get_if<ProbeShutdownStep>(&step);
        if (shutdownStep == nullptr) {
            fail(QStringLiteral("scenario '%1' contains an unsupported step.").arg(scenario_.id));
            return;
        }
        if (stepIndex_ + 1 != scenario_.steps.size()) {
            fail(QStringLiteral("shutdown step must be the final step in scenario '%1'.")
                     .arg(scenario_.id));
            return;
        }

        trace_.recordMarker(QStringLiteral("scripted-shutdown: %1").arg(shutdownStep->label));
        ++stepIndex_;
        beginFinalize(true, QString{});
        return;
    }

    beginFinalize(true, QString{});
}

void PlaybackProbeScenarioRunner::armTimeout(const QString& stepLabel)
{
    if (stepTimer_.isActive() && currentStep_ == stepLabel) {
        return;
    }
    currentStep_ = stepLabel;
    stepTimer_.start();
}

void PlaybackProbeScenarioRunner::fail(const QString& diagnostic)
{
    if (!running_ || finalizing_) {
        return;
    }
    beginFinalize(false, diagnostic);
}

void PlaybackProbeScenarioRunner::beginFinalize(bool success, const QString& diagnostic)
{
    if (finalizing_) {
        return;
    }

    finalizing_ = true;
    stepTimer_.stop();
    currentStep_.clear();
    trace_.recordMarker(success ? QStringLiteral("scenario-complete")
                                : QStringLiteral("scenario-failed: %1").arg(diagnostic));

    QMetaObject::invokeMethod(
        this,
        [this, success, diagnostic]() {
            runtime_.shutdown();
            trace_.recordMarker(QStringLiteral("runtime-shutdown-complete"));
            trace_.dump();
            running_ = false;
            finalizing_ = false;
            emit finished(success, diagnostic);
        },
        Qt::QueuedConnection);
}

bool PlaybackProbeScenarioRunner::takeCommandReply(quint64 requestId)
{
    return takeFirstMatching(
        pendingEvents_,
        [requestId](const player::playback::mpv::MpvEvent& event) {
            return event.type == player::playback::mpv::MpvEventType::CommandReply
                && event.replyUserdata == requestId;
        });
}

bool PlaybackProbeScenarioRunner::takeEvent(const ProbeWaitEventStep& step)
{
    return takeFirstMatching(
        pendingEvents_,
        [&step](const player::playback::mpv::MpvEvent& event) {
            if (event.type != step.type) {
                return false;
            }
            if (step.type != player::playback::mpv::MpvEventType::EndFile
                || !step.endReason.has_value()) {
                return true;
            }
            const auto* endFile = std::get_if<player::playback::mpv::MpvEndFileData>(&event.payload);
            return endFile != nullptr && endFile->reason == *step.endReason;
        });
}

bool PlaybackProbeScenarioRunner::takeFlagProperty(const ProbeWaitFlagPropertyStep& step)
{
    return takeFirstMatching(
        pendingEvents_,
        [&step](const player::playback::mpv::MpvEvent& event) {
            if (event.type != player::playback::mpv::MpvEventType::PropertyChange) {
                return false;
            }
            const auto* change = std::get_if<player::playback::mpv::MpvPropertyChange>(&event.payload);
            if (change == nullptr || change->id != step.propertyId) {
                return false;
            }
            const bool* value = std::get_if<bool>(&change->value);
            return value != nullptr && *value == step.expectedValue;
        });
}

bool PlaybackProbeScenarioRunner::takeStringProperty(const ProbeWaitStringPropertyStep& step)
{
    return takeFirstMatching(
        pendingEvents_,
        [&step](const player::playback::mpv::MpvEvent& event) {
            if (event.type != player::playback::mpv::MpvEventType::PropertyChange) {
                return false;
            }
            const auto* change = std::get_if<player::playback::mpv::MpvPropertyChange>(&event.payload);
            if (change == nullptr || change->id != step.propertyId) {
                return false;
            }
            const QString* value = std::get_if<QString>(&change->value);
            return value != nullptr
                && stringValuesMatch(step.propertyId, *value, step.expectedValue);
        });
}

bool PlaybackProbeScenarioRunner::scenarioExpectsEndReason(
    player::playback::mpv::MpvEndFileReason reason) const
{
    for (const PlaybackProbeStep& step : scenario_.steps) {
        const auto* eventStep = std::get_if<ProbeWaitEventStep>(&step);
        if (eventStep != nullptr
            && eventStep->type == player::playback::mpv::MpvEventType::EndFile
            && eventStep->endReason.has_value() && *eventStep->endReason == reason) {
            return true;
        }
    }
    return false;
}

} // namespace player::tools::playback_probe
