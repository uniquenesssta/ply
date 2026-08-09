#pragma once

#include "playback/infrastructure/mpv/client/mpv_handle.h"
#include "playback/infrastructure/mpv/initialization/mpv_initializer.h"
#include "playback/infrastructure/mpv/initialization/mpv_option_profile.h"

#include <QByteArray>
#include <QList>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QString>
#include <QSurfaceFormat>

#include <memory>
#include <utility>

namespace player::test::render {

using player::playback::mpv::MpvHandle;
using player::playback::mpv::MpvInitializer;
using player::playback::mpv::MpvOption;
using player::playback::mpv::MpvOptionProfile;

class OffscreenOpenGlContext final
{
public:
    bool initialize(QString* errorMessage)
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }

        context_.setFormat(QSurfaceFormat::defaultFormat());
        if (!context_.create()) {
            assignError(errorMessage, QStringLiteral("Unable to create the OpenGL test context."));
            return false;
        }

        surface_.setFormat(context_.format());
        surface_.create();
        if (!surface_.isValid()) {
            assignError(errorMessage, QStringLiteral("Unable to create the offscreen OpenGL test surface."));
            return false;
        }

        return makeCurrent(errorMessage);
    }

    bool makeCurrent(QString* errorMessage = nullptr)
    {
        if (errorMessage != nullptr) {
            errorMessage->clear();
        }

        if (!context_.makeCurrent(&surface_)) {
            assignError(errorMessage, QStringLiteral("Unable to make the OpenGL test context current."));
            return false;
        }

        return true;
    }

    void doneCurrent() noexcept
    {
        if (QOpenGLContext::currentContext() == &context_) {
            context_.doneCurrent();
        }
    }

    ~OffscreenOpenGlContext()
    {
        doneCurrent();
    }

private:
    static void assignError(QString* errorMessage, const QString& message)
    {
        if (errorMessage != nullptr) {
            *errorMessage = message;
        }
    }

    QOpenGLContext context_;
    QOffscreenSurface surface_;
};

inline std::unique_ptr<MpvHandle> createInitializedCore(QString* errorMessage)
{
    std::unique_ptr<MpvHandle> handle = MpvHandle::create(errorMessage);
    if (!handle) {
        return {};
    }

    if (!MpvInitializer::initializeProduct(*handle, errorMessage)) {
        return {};
    }

    return handle;
}

inline std::unique_ptr<MpvHandle> createInitializedVideoCore(QString* errorMessage)
{
    std::unique_ptr<MpvHandle> handle = MpvHandle::create(errorMessage);
    if (!handle) {
        return {};
    }

    MpvOptionProfile defaults = MpvOptionProfile::productDefaults();
    QList<MpvOption> options = defaults.options();
    options.append(MpvOption{
        QByteArrayLiteral("vo"),
        QByteArrayLiteral("libmpv"),
    });

    MpvOptionProfile profile(std::move(options));
    if (!MpvInitializer::initialize(*handle, profile, errorMessage)) {
        return {};
    }

    return handle;
}

} // namespace player::test::render
