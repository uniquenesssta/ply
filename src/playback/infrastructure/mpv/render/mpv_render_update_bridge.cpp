#include "mpv_render_update_bridge.h"

#include "mpv_render_context.h"
#include "mpv_render_shutdown_coordinator.h"
#include "mpv_render_visibility_policy.h"

#include <QDebug>
#include <QString>

#include <exception>
#include <utility>

namespace player::playback::infrastructure::mpv::render {
namespace {

void assignError(QString* errorMessage, QString message)
{
    if (errorMessage != nullptr) {
        *errorMessage = std::move(message);
    }
}

} // namespace

bool MpvRenderUpdateDeliveryGate::allows(quint64 activationEpoch) const noexcept
{
    return activationEpoch != 0
        && activeEpoch_.load(std::memory_order_acquire) == activationEpoch;
}

MpvRenderUpdateBridge::MpvRenderUpdateBridge(QObject* parent)
    : MpvRenderUpdateBridge({}, {}, parent)
{
}

MpvRenderUpdateBridge::MpvRenderUpdateBridge(
    std::shared_ptr<const MpvRenderVisibilityPolicy> visibilityPolicy,
    QObject* parent)
    : MpvRenderUpdateBridge(std::move(visibilityPolicy), {}, parent)
{
}

MpvRenderUpdateBridge::MpvRenderUpdateBridge(
    std::shared_ptr<const MpvRenderVisibilityPolicy> visibilityPolicy,
    std::shared_ptr<const MpvRenderShutdownCoordinator> shutdownCoordinator,
    QObject* parent)
    : QObject(parent)
    , visibilityPolicy_(std::move(visibilityPolicy))
    , shutdownCoordinator_(std::move(shutdownCoordinator))
    , deliveryGate_(std::make_shared<MpvRenderUpdateDeliveryGate>())
{
}

MpvRenderUpdateBridge::~MpvRenderUpdateBridge()
{
    QString errorMessage;
    if (!deactivate(&errorMessage)) {
        qCritical().noquote()
            << QStringLiteral("MpvRenderUpdateBridge could not safely unregister its libmpv callback: %1")
                   .arg(errorMessage);
        std::terminate();
    }
}

bool MpvRenderUpdateBridge::activate(
    MpvRenderContext& renderContext,
    QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    {
        std::scoped_lock lock(stateMutex_);
        if (renderContext_ != nullptr) {
            if (active_ && renderContext_ == &renderContext) {
                return true;
            }

            assignError(
                errorMessage,
                QStringLiteral("MpvRenderUpdateBridge is already associated with a render context."));
            return false;
        }

        ++activationEpoch_;
        if (activationEpoch_ == 0) {
            ++activationEpoch_;
        }
        renderContext_ = &renderContext;
        active_ = true;
        deliveryGate_->activeEpoch_.store(activationEpoch_, std::memory_order_release);
    }

    QString callbackError;
    if (!renderContext.setUpdateCallback(
            &MpvRenderUpdateBridge::onRenderUpdate,
            this,
            &callbackError)) {
        {
            std::scoped_lock lock(stateMutex_);
            ++activationEpoch_;
            active_ = false;
            renderContext_ = nullptr;
            callbackInstalled_ = false;
            deliveryGate_->activeEpoch_.store(0, std::memory_order_release);
        }

        assignError(
            errorMessage,
            QStringLiteral("Cannot install the mpv render update callback: %1").arg(callbackError));
        return false;
    }

    {
        std::scoped_lock lock(stateMutex_);
        callbackInstalled_ = true;
    }

    return true;
}

bool MpvRenderUpdateBridge::deactivate(QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    MpvRenderContext* renderContext = nullptr;
    bool callbackInstalled = false;
    {
        std::scoped_lock lock(stateMutex_);
        if (active_ || renderContext_ != nullptr) {
            ++activationEpoch_;
        }
        active_ = false;
        deliveryGate_->activeEpoch_.store(0, std::memory_order_release);
        renderContext = renderContext_;
        callbackInstalled = callbackInstalled_;
    }

    if (renderContext == nullptr) {
        waitForCallbacksToDrain();
        return true;
    }

    if (callbackInstalled) {
        QString callbackError;
        if (!renderContext->setUpdateCallback(nullptr, nullptr, &callbackError)) {
            assignError(
                errorMessage,
                QStringLiteral("Cannot unregister the mpv render update callback: %1")
                    .arg(callbackError));
            return false;
        }
    }

    {
        std::scoped_lock lock(stateMutex_);
        if (renderContext_ == renderContext) {
            callbackInstalled_ = false;
            renderContext_ = nullptr;
        }
    }

    waitForCallbacksToDrain();
    return true;
}

bool MpvRenderUpdateBridge::isActive() const noexcept
{
    std::scoped_lock lock(stateMutex_);
    return active_;
}

std::shared_ptr<const MpvRenderUpdateDeliveryGate>
MpvRenderUpdateBridge::deliveryGate() const noexcept
{
    return deliveryGate_;
}

void MpvRenderUpdateBridge::onRenderUpdate(void* context) noexcept
{
    if (context == nullptr) {
        return;
    }

    static_cast<MpvRenderUpdateBridge*>(context)->dispatchRenderUpdate();
}

void MpvRenderUpdateBridge::dispatchRenderUpdate() noexcept
{
    callbacksInFlight_.fetch_add(1, std::memory_order_acq_rel);

    quint64 activationEpoch = 0;
    {
        std::scoped_lock lock(stateMutex_);
        if (active_) {
            activationEpoch = activationEpoch_;
        }
    }

    if (activationEpoch != 0
        && deliveryGate_->allows(activationEpoch)
        && shutdownAllowsUpdateDelivery()
        && visibilityAllowsUpdateDelivery()) {
        emit updateRequested(activationEpoch);
    }

    if (callbacksInFlight_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        std::scoped_lock lock(callbackMutex_);
        callbackCondition_.notify_all();
    }
}

void MpvRenderUpdateBridge::waitForCallbacksToDrain() noexcept
{
    std::unique_lock lock(callbackMutex_);
    callbackCondition_.wait(lock, [this] {
        return callbacksInFlight_.load(std::memory_order_acquire) == 0;
    });
}

bool MpvRenderUpdateBridge::visibilityAllowsUpdateDelivery() const noexcept
{
    return visibilityPolicy_ == nullptr || visibilityPolicy_->snapshot().updatesAllowed;
}

bool MpvRenderUpdateBridge::shutdownAllowsUpdateDelivery() const noexcept
{
    return shutdownCoordinator_ == nullptr || !shutdownCoordinator_->isShutdownRequested();
}

} // namespace player::playback::infrastructure::mpv::render
