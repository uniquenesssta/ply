#include <QFile>
#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QSignalSpy>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <QtTest>

#include <memory>
#include <utility>

namespace player::presentation::qml {
namespace {

QString readSource(const QString& relativePath)
{
    QFile file(QStringLiteral(PLAYER_SOURCE_DIR "/") + relativePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QString diagnostics(const QQmlComponent& component)
{
    QStringList result;
    for (const QQmlError& error : component.errors()) {
        result.append(error.toString());
    }
    return result.join(QLatin1Char('\n'));
}

bool resolve(QQmlComponent& component)
{
    if (component.status() != QQmlComponent::Loading) {
        return true;
    }
    QSignalSpy spy(&component, &QQmlComponent::statusChanged);
    return spy.wait(5000);
}

class StatusStub final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusKey READ statusKey CONSTANT)
    Q_PROPERTY(bool visible READ visible CONSTANT)
    Q_PROPERTY(int bufferingPercent READ bufferingPercent CONSTANT)

public:
    StatusStub(QString statusKey, bool visible, int bufferingPercent = -1)
        : statusKey_(std::move(statusKey))
        , visible_(visible)
        , bufferingPercent_(bufferingPercent)
    {
    }

    [[nodiscard]] QString statusKey() const { return statusKey_; }
    [[nodiscard]] bool visible() const noexcept { return visible_; }
    [[nodiscard]] int bufferingPercent() const noexcept { return bufferingPercent_; }

private:
    QString statusKey_;
    bool visible_ = false;
    int bufferingPercent_ = -1;
};

std::unique_ptr<QObject> createOverlay(
    QQmlEngine& engine,
    StatusStub& viewModel,
    bool suppressBuffering = false)
{
    const QString path = QStringLiteral(
        PLAYER_SOURCE_DIR "/src/presentation/qml/screens/player/overlays/status/PlayerStatusOverlay.qml");
    QQmlComponent component(&engine, QUrl::fromLocalFile(path));
    if (!resolve(component) || !component.isReady()) {
        qWarning().noquote() << diagnostics(component);
        return nullptr;
    }

    QVariantMap initialProperties;
    initialProperties.insert(
        QStringLiteral("viewModel"),
        QVariant::fromValue(static_cast<QObject*>(&viewModel)));
    initialProperties.insert(QStringLiteral("suppressBuffering"), suppressBuffering);

    std::unique_ptr<QObject> object(component.createWithInitialProperties(initialProperties));
    if (!object) {
        qWarning().noquote() << diagnostics(component);
        return nullptr;
    }

    QCoreApplication::processEvents();
    return object;
}

} // namespace

class PlayerStatusOverlayTest final : public QObject
{
    Q_OBJECT

private slots:
    void loadsEachStatusRole();
    void emptyActionRoutesOpenIntent();
    void bufferingCanBeSuppressedByTimelineInteraction();
    void overlayBoundaryStaysFocused();
    void playerScreenRoutesStatusWithoutDuplicateErrorState();
};

void PlayerStatusOverlayTest::loadsEachStatusRole()
{
    struct Case final {
        QString key;
        QString objectName;
        int bufferingPercent;
    };

    const QList<Case> cases{
        {QStringLiteral("empty"), QStringLiteral("playerEmptyFeedback"), -1},
        {QStringLiteral("loading"), QStringLiteral("playerLoadingFeedback"), -1},
        {QStringLiteral("buffering"), QStringLiteral("playerBufferingFeedback"), 42},
        {QStringLiteral("ended"), QStringLiteral("playerEndedFeedback"), -1},
        {QStringLiteral("error"), QStringLiteral("playerErrorFeedback"), -1},
    };

    for (const Case& testCase : cases) {
        QQmlEngine engine;
        StatusStub viewModel(testCase.key, true, testCase.bufferingPercent);
        std::unique_ptr<QObject> overlay = createOverlay(engine, viewModel);
        QVERIFY2(overlay != nullptr, qPrintable(testCase.key));
        QVERIFY(overlay->property("statusVisible").toBool());
        QTRY_VERIFY_WITH_TIMEOUT(
            overlay->findChild<QObject*>(testCase.objectName) != nullptr,
            1000);
    }
}

void PlayerStatusOverlayTest::emptyActionRoutesOpenIntent()
{
    QQmlEngine engine;
    StatusStub viewModel(QStringLiteral("empty"), true);
    std::unique_ptr<QObject> overlay = createOverlay(engine, viewModel);
    QVERIFY(overlay != nullptr);

    QTRY_VERIFY_WITH_TIMEOUT(
        overlay->findChild<QObject*>(QStringLiteral("playerEmptyFeedback")) != nullptr,
        1000);
    QObject* emptyFeedback = overlay->findChild<QObject*>(QStringLiteral("playerEmptyFeedback"));
    QVERIFY(emptyFeedback != nullptr);

    QSignalSpy openSpy(overlay.get(), SIGNAL(openMediaRequested()));
    QVERIFY(QMetaObject::invokeMethod(emptyFeedback, "actionRequested"));
    QCOMPARE(openSpy.count(), 1);
}

void PlayerStatusOverlayTest::bufferingCanBeSuppressedByTimelineInteraction()
{
    QQmlEngine engine;
    StatusStub viewModel(QStringLiteral("buffering"), true, 75);
    std::unique_ptr<QObject> overlay = createOverlay(engine, viewModel, true);
    QVERIFY(overlay != nullptr);
    QVERIFY(!overlay->property("statusVisible").toBool());
    QVERIFY(overlay->findChild<QObject*>(QStringLiteral("playerBufferingFeedback")) == nullptr);
}

void PlayerStatusOverlayTest::overlayBoundaryStaysFocused()
{
    const QString source = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/overlays/status/PlayerStatusOverlay.qml"));
    QVERIFY(!source.isEmpty());

    QVERIFY(source.contains(QStringLiteral("EmptyFeedback")));
    QVERIFY(source.contains(QStringLiteral("LoadingFeedback")));
    QVERIFY(source.contains(QStringLiteral("BufferingFeedback")));
    QVERIFY(source.contains(QStringLiteral("EndedFeedback")));
    QVERIFY(source.contains(QStringLiteral("ErrorFeedback")));
    QVERIFY(source.contains(QStringLiteral("signal openMediaRequested()")));
    QVERIFY(source.contains(QStringLiteral("onActionRequested: root.openMediaRequested()")));
    QVERIFY(source.contains(QStringLiteral("suppressBuffering")));
    QVERIFY(source.contains(QStringLiteral("visible: root.statusVisible")));
    QVERIFY(source.contains(QStringLiteral("active: root.statusVisible")));

    QVERIFY(!source.contains(QStringLiteral("FileDialog")));
    QVERIFY(!source.contains(QStringLiteral("Timer {")));
    QVERIFY(!source.contains(QStringLiteral("Toast")));
    QVERIFY(!source.contains(QStringLiteral("Hud"), Qt::CaseInsensitive));
    QVERIFY(!source.contains(QStringLiteral("Dialog")));
    QVERIFY(!source.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!source.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
    QVERIFY(!source.contains(QStringLiteral("mpv_"), Qt::CaseInsensitive));
}

void PlayerStatusOverlayTest::playerScreenRoutesStatusWithoutDuplicateErrorState()
{
    const QString source = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    QVERIFY(!source.isEmpty());

    QVERIFY(source.contains(QStringLiteral("property var statusViewModel: null")));
    QVERIFY(source.contains(QStringLiteral("readonly property bool errorOverlayVisible")));
    QVERIFY(source.contains(QStringLiteral("root.statusViewModel.errorVisible")));
    QVERIFY(source.contains(QStringLiteral("signal openMediaRequested()")));
    QVERIFY(source.contains(QStringLiteral("PlayerStatusOverlay {")));
    QVERIFY(source.contains(QStringLiteral("viewModel: root.statusViewModel")));
    QVERIFY(source.contains(QStringLiteral("suppressBuffering: root.timelineInteractionActive")));
    QVERIFY(source.contains(QStringLiteral("onOpenMediaRequested: root.openMediaRequested()")));
    QVERIFY(!source.contains(QStringLiteral("property bool errorOverlayVisible: false")));
    QVERIFY(!source.contains(QStringLiteral("FileDialog")));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::PlayerStatusOverlayTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "player_status_overlay_test.moc"
