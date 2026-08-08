#pragma once

#include <QString>

namespace player::app {

class RuntimePaths final
{
public:
    enum class Mode {
        Installed,
        Portable,
    };

    static RuntimePaths current();
    static RuntimePaths fromExecutableFilePath(const QString& executableFilePath);
    static RuntimePaths resolve(Mode mode, const QString& executableDirectory);
    static Mode detectMode(const QString& executableDirectory);

    [[nodiscard]] Mode mode() const noexcept;
    [[nodiscard]] const QString& executableDirectory() const noexcept;
    [[nodiscard]] const QString& configDirectory() const noexcept;
    [[nodiscard]] const QString& dataDirectory() const noexcept;
    [[nodiscard]] const QString& logDirectory() const noexcept;
    [[nodiscard]] const QString& screenshotDirectory() const noexcept;

private:
    RuntimePaths(
        Mode mode,
        QString executableDirectory,
        QString configDirectory,
        QString dataDirectory,
        QString logDirectory,
        QString screenshotDirectory);

    Mode m_mode;
    QString m_executableDirectory;
    QString m_configDirectory;
    QString m_dataDirectory;
    QString m_logDirectory;
    QString m_screenshotDirectory;
};

} // namespace player::app
