#include "playback/infrastructure/mpv/initialization/mpv_option_profile.h"

#include <utility>

namespace player::playback::mpv {
namespace {

QList<MpvOption> environmentIsolationOptions()
{
    return {
        {QByteArrayLiteral("config"), QByteArrayLiteral("no")},
    };
}

} // namespace

MpvOptionProfile::MpvOptionProfile(QList<MpvOption> options)
    : options_(std::move(options))
{
}

MpvOptionProfile MpvOptionProfile::productDefaults()
{
    return MpvOptionProfile(environmentIsolationOptions());
}

const QList<MpvOption>& MpvOptionProfile::options() const noexcept
{
    return options_;
}

} // namespace player::playback::mpv
