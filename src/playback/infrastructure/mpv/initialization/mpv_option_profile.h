#pragma once

#include <QByteArray>
#include <QList>

namespace player::playback::mpv {

struct MpvOption final
{
    QByteArray name;
    QByteArray value;
};

class MpvOptionProfile final
{
public:
    explicit MpvOptionProfile(QList<MpvOption> options);

    [[nodiscard]] static MpvOptionProfile productDefaults();
    [[nodiscard]] const QList<MpvOption>& options() const noexcept;

private:
    QList<MpvOption> options_;
};

} // namespace player::playback::mpv
