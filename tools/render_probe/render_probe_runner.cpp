#include "render_probe_runner.h"

#include "render_probe_report.h"

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/render/mpv_render_shutdown_coordinator.h"

#include <mpv/client.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QScreen>
#include <QTextStream>
#include <QtGlobal>

#include <utility>

namespace player::tools::render_probe {
namespace {

QString mpvErrorMessage(const char* operation, int result)
{
    return QStringLiteral("%1 failed: %2 (%3).")
        .arg(QString::fromLatin1(operation), QString::fromUtf8(mpv_error_string(result)))
        .arg(result);
}

} // namespace

RenderProbeRunner::RenderProbeRunner(RenderProbeOptions options, QObject* parent)
    : QObject(parent)
    , options_(std::move(options))
    , videoItem_(window_.contentItem())
{
    window_.setTitle(QStringLiteral("R4 Render Performance Probe"));
    window_.setColor(Qt::black);
    window_.resize(options_.windowSize);
    updateVideoItemSize();

    pollTimer_.setInterval(kPollIntervalMs);
    connect(&pollTimer_, &QTimer::timeout, this, &RenderProbeRunner::pollStage);

    phaseTimer_.setSingleShot(true);
    connect(&phaseTimer_, &QTimer::timeout, this, [this] {
        if (stage_ == Stage::MeasuringWindowed) {
            finishMeasurement(QStringLiteral("windowed"));
        } else if (stage_ == Stage::MeasuringFullscreen) {
            finishMeasurement(QStringLiteral("fullscreen"));
        }
    });

    connect(&window_, &QWindow::widthChanged, this, [this](int) {
        updateVideoItemSize();
    });
    connect(&window_, &QWindow::heightChanged, this, [this](int) {
        updateVideoItemSize();
    });

    connect(
        &window_,
        &QQuickWindow::afterRendering,
        &window_,
        [this] {
            renderedFrameCount_.fetch_add(1, std::memory_order_release);
            frameTiming_.noteFrame();
        },
        Qt::DirectConnection);
}

RenderProbeRunner::~RenderProbeRunner()
{
    if (core_ == nullptr) {
        return;
    }

    const auto coordinator = videoItem_.renderShutdownCoordinator();
    if (coordinator != nullptr && coordinator->snapshot().liveRenderContexts > 0) {
        qCritical("render_probe is exiting before render release; the mpv core is intentionally left alive for process teardown.");
        (void)core_.release();
        return;
    }

    core_->close();
}

bool RenderProbeRunner::start(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (stage_ != Stage::Idle) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("render_probe has already been started.");
        }
        return false;
    }

    QString environmentWarning;
    environment_ = captureRenderProbeEnvironment(&environmentWarning);
    if (!environmentWarning.isEmpty()) {
        qWarning().noquote() << environmentWarning;
    }

    QString coreError;
    core_ = player::playback::mpv::MpvHandle::create(&coreError);
    if (core_ == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = coreError;
        }
        return false;
    }

    if (!player::playback::mpv::MpvInitializer::initializeProduct(*core_, &coreError)) {
        if (errorMessage != nullptr) {
            *errorMessage = coreError;
        }
        core_.reset();
        return false;
    }

    videoItem_.setRenderCoreHandle(core_->nativeHandle());
    window_.show();
    window_.update();

    stage_ = Stage::WaitingForWindow;
    stageElapsed_.start();
    pollTimer_.start();
    return true;
}

void RenderProbeRunner::pollStage()
{
    if (stage_ == Stage::WaitingForWindow) {
        if (windowReady()) {
            pollTimer_.stop();
            loadMedia();
            return;
        }

        if (stageElapsed_.elapsed() > kStartupTimeoutMs) {
            fail(QStringLiteral("Timed out waiting for the Qt Quick window and Scene Graph."));
        }
        return;
    }

    if (stage_ == Stage::WaitingForMedia) {
        if (mediaReady()) {
            pollTimer_.stop();
            settleAndStartWindowed();
            return;
        }

        if (stageElapsed_.elapsed() > kStartupTimeoutMs) {
            fail(QStringLiteral("Timed out waiting for decoded video, an active mpv render context, and real Qt render frames."));
        }
        return;
    }

    if (stage_ == Stage::WaitingForFullscreen) {
        if (fullscreenReady()) {
            pollTimer_.stop();
            QTimer::singleShot(kModeSettleMs, this, [this] {
                if (stage_ == Stage::WaitingForFullscreen) {
                    startFullscreenMeasurement();
                }
            });
            return;
        }

        if (stageElapsed_.elapsed() > kStartupTimeoutMs) {
            fail(QStringLiteral("Timed out waiting for fullscreen render readiness."));
        }
        return;
    }

    if (stage_ == Stage::ShuttingDown) {
        const auto coordinator = videoItem_.renderShutdownCoordinator();
        if (coordinator != nullptr && coordinator->snapshot().renderReleased) {
            completeShutdown();
            return;
        }

        if (stageElapsed_.elapsed() > kShutdownTimeoutMs) {
            pollTimer_.stop();
            pendingError_ = pendingError_.isEmpty()
                ? QStringLiteral("Render shutdown barrier timed out; mpv core destroy was intentionally skipped.")
                : pendingError_ + QStringLiteral(" Render shutdown barrier timed out; mpv core destroy was intentionally skipped.");
            if (core_ != nullptr) {
                (void)core_.release();
            }
            finish(2);
        }
    }
}

void RenderProbeRunner::loadMedia()
{
    if (core_ == nullptr || !core_->isOpen()) {
        fail(QStringLiteral("Cannot load media because the mpv core is unavailable."));
        return;
    }

    minimumRenderedFrameCountForMediaReady_ = -1;

    const QByteArray source = options_.mediaSource.toUtf8();
    const char* command[] = {
        "loadfile",
        source.constData(),
        nullptr,
    };
    const int result = mpv_command(core_->nativeHandle(), command);
    if (result < 0) {
        fail(mpvErrorMessage("loadfile", result));
        return;
    }

    stage_ = Stage::WaitingForMedia;
    stageElapsed_.restart();
    pollTimer_.start();
}

void RenderProbeRunner::settleAndStartWindowed()
{
    window_.showNormal();
    window_.resize(options_.windowSize);
    updateVideoItemSize();
    window_.update();

    QTimer::singleShot(kModeSettleMs, this, [this] {
        if (stage_ == Stage::WaitingForMedia) {
            startWindowedMeasurement();
        }
    });
}

void RenderProbeRunner::startWindowedMeasurement()
{
    if (!windowReady()) {
        fail(QStringLiteral("Windowed render path became unavailable before measurement."));
        return;
    }

    stage_ = Stage::MeasuringWindowed;
    phaseBefore_ = captureRenderProbeMpvSnapshot(core_->nativeHandle());
    frameTiming_.start();
    phaseTimer_.start(options_.phaseDurationMs);
}

void RenderProbeRunner::startFullscreenTransition()
{
    window_.showFullScreen();
    window_.update();
    stage_ = Stage::WaitingForFullscreen;
    stageElapsed_.restart();
    pollTimer_.start();
}

void RenderProbeRunner::startFullscreenMeasurement()
{
    if (!fullscreenReady()) {
        fail(QStringLiteral("Fullscreen render path became unavailable before measurement."));
        return;
    }

    stage_ = Stage::MeasuringFullscreen;
    phaseBefore_ = captureRenderProbeMpvSnapshot(core_->nativeHandle());
    frameTiming_.start();
    phaseTimer_.start(options_.phaseDurationMs);
}

void RenderProbeRunner::finishMeasurement(const QString& mode)
{
    const RenderProbeTimingSummary timing = frameTiming_.finish();
    const RenderProbeMpvSnapshot after = captureRenderProbeMpvSnapshot(core_->nativeHandle());
    phases_.append(makePhaseResult(mode, phaseBefore_, after, timing));

    if (stage_ == Stage::MeasuringWindowed) {
        startFullscreenTransition();
        return;
    }

    beginShutdown();
}

void RenderProbeRunner::beginShutdown()
{
    if (stage_ == Stage::Finished || stage_ == Stage::ShuttingDown) {
        return;
    }

    phaseTimer_.stop();
    pollTimer_.stop();
    stage_ = Stage::ShuttingDown;
    videoItem_.beginRenderShutdown();
    window_.update();
    stageElapsed_.restart();
    pollTimer_.start();
}

void RenderProbeRunner::completeShutdown()
{
    pollTimer_.stop();

    if (core_ != nullptr && core_->isOpen()) {
        const char* stopCommand[] = {
            "stop",
            nullptr,
        };
        (void)mpv_command(core_->nativeHandle(), stopCommand);
        core_->close();
        core_.reset();
    }

    window_.hide();
    window_.releaseResources();
    QCoreApplication::processEvents();

    if (!pendingError_.isEmpty()) {
        finish(1);
        return;
    }

    RenderProbeReport report;
    report.label = options_.label;
    report.mediaSource = options_.mediaSource;
    report.capturedAtUtc = QDateTime::currentDateTimeUtc();
    report.environment = environment_;
    report.phases = phases_;

    QString reportError;
    if (!writeRenderProbeReport(report, options_.outputPath, &reportError)) {
        pendingError_ = reportError;
        finish(1);
        return;
    }

    finish(0);
}

void RenderProbeRunner::fail(QString message)
{
    if (pendingError_.isEmpty()) {
        pendingError_ = std::move(message);
    }
    beginShutdown();
}

void RenderProbeRunner::finish(int exitCode)
{
    if (stage_ == Stage::Finished) {
        return;
    }

    pollTimer_.stop();
    phaseTimer_.stop();
    stage_ = Stage::Finished;

    if (exitCode != 0 && !pendingError_.isEmpty()) {
        QTextStream error(stderr);
        error << "[render_probe] FAIL: " << pendingError_ << Qt::endl;
    }

    emit finished(exitCode);
}

void RenderProbeRunner::updateVideoItemSize()
{
    videoItem_.setWidth(static_cast<qreal>(window_.width()));
    videoItem_.setHeight(static_cast<qreal>(window_.height()));
}

bool RenderProbeRunner::windowReady() const noexcept
{
    const auto visibilityPolicy = videoItem_.renderVisibilityPolicy();
    return window_.isVisible()
        && window_.isExposed()
        && window_.isSceneGraphInitialized()
        && visibilityPolicy != nullptr
        && visibilityPolicy->snapshot().updatesAllowed;
}

bool RenderProbeRunner::mediaReady() noexcept
{
    if (core_ == nullptr || !core_->isOpen()) {
        minimumRenderedFrameCountForMediaReady_ = -1;
        return false;
    }

    const RenderProbeMpvSnapshot snapshot = captureRenderProbeMpvSnapshot(core_->nativeHandle());
    const auto coordinator = videoItem_.renderShutdownCoordinator();
    const bool pipelineReady = snapshot.videoWidth.has_value()
        && *snapshot.videoWidth > 0
        && snapshot.videoHeight.has_value()
        && *snapshot.videoHeight > 0
        && coordinator != nullptr
        && coordinator->snapshot().liveRenderContexts > 0;

    if (!pipelineReady) {
        minimumRenderedFrameCountForMediaReady_ = -1;
        return false;
    }

    const int renderedFrameCount = renderedFrameCount_.load(std::memory_order_acquire);
    if (minimumRenderedFrameCountForMediaReady_ < 0) {
        minimumRenderedFrameCountForMediaReady_ =
            renderedFrameCount + kRequiredRenderedFramesAfterPipelineReady;
        return false;
    }

    return renderedFrameCount >= minimumRenderedFrameCountForMediaReady_;
}

bool RenderProbeRunner::fullscreenReady() const noexcept
{
    return windowReady() && window_.visibility() == QWindow::FullScreen;
}

RenderProbePhaseResult RenderProbeRunner::makePhaseResult(
    const QString& mode,
    RenderProbeMpvSnapshot before,
    RenderProbeMpvSnapshot after,
    RenderProbeTimingSummary timing) const
{
    RenderProbePhaseResult result;
    result.mode = mode;
    result.logicalWindowSize = window_.size();

    const qreal dpr = window_.effectiveDevicePixelRatio();
    result.devicePixelRatio = dpr;
    result.physicalRenderSize = QSize(
        qRound(static_cast<qreal>(window_.width()) * dpr),
        qRound(static_cast<qreal>(window_.height()) * dpr));

    if (QScreen* screen = window_.screen(); screen != nullptr) {
        result.screenPixelSize = screen->size();
        result.screenRefreshRate = screen->refreshRate();
    }

    result.timing = std::move(timing);
    result.before = std::move(before);
    result.after = std::move(after);
    return result;
}

} // namespace player::tools::render_probe
