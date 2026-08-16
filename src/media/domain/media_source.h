#pragma once

#include <QString>
#include <QtGlobal>

namespace player::media::domain {

enum class MediaSourceKind : quint8
{
    LocalFile = 0,
    RemoteUrl,
};

class MediaSource final
{
public:
    [[nodiscard]] static MediaSource localFile(QString canonicalPath);
    [[nodiscard]] static MediaSource remoteUrl(QString normalizedUrl);

    [[nodiscard]] MediaSourceKind kind() const noexcept;
    [[nodiscard]] const QString& location() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;

private:
    MediaSource(MediaSourceKind kind, QString location);

    MediaSourceKind kind_ = MediaSourceKind::LocalFile;
    QString location_;
};

} // namespace player::media::domain
