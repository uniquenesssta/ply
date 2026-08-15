#include <QFile>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QSignalSpy>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>
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

class HudStub final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible CONSTANT)
    Q_PROPERTY(QString messageKey READ messageKey CONSTANT)
    Q_PROPERTY(QString valueText READ valueText CONSTANT)

public:
    HudStub(bool visible, QString messageKey, QString valueText)
        : visible_(visible)
        , messageKey_(std::move(messageKey))
        , valueText_(std::move(valueText))
    {
    }

    [[nodiscard]] bool visible() const noexcept { return visible_; }
    [[nodiscard]] QString messageKey() const { return messageKey_; }
    [[nodiscard]] QString valueText() const { return valueText_; }

private:
    bool visible_ = false;
    QString messageKey_;
    QString valueText_;
};

std::unique_ptr<QObject> createOverlay(
    QQmlEngine& engine,
    HudStub& viewModel,
    bool suppressed = false)
{
    const QString path = QStringLiteral(
        PLAYER_SOURCE_DIR "/src/presentation/qml/screens/player/overlays/hud/PlayerHudOverlay.qml");
    QQmlComponent component(&engine, QUrl::fromLocalFile(path));
    if (!resolve(component) || !component.isReady()) {
        qWarning().noquote() << diagnostics(component);
        return nullptr;
    }

    const QVariantMap initialProperties{
        {QStringLiteral("viewModel"),
         QVariant::fromValue(static_cast<QObject*>(&viewModel))},
        {QStringLiteral("suppressed"), suppressed},
    };
    std::unique_ptr<QObject> object(
        component.createWithInitialProperties(initialProperties));
    if (!object) {
        qWarning().noquote() << diagnostics(component);
    }
    return object;
}

} // namespace

class PlayerHudOverlayTest final : public QObject
{
    Q_OBJECT

private slots:
    void loadsVolumeSeekMuteFailureSpeedAndTrackSemantics();
    void hiddenQueueKeepsHudSemanticallyHidden();
    void fatalErrorSuppressionKeepsHudSemanticallyHidden();
    void overlayBoundaryStaysFocused();
    void playerScreenAndBootstrapRouteSingleQueue();
};

void PlayerHudOverlayTest::loadsVolumeSeekMuteFailureSpeedAndTrackSemantics()
{
    struct Case final {
        QString key;
        QString value;
        QString expectedLabel;
        QString expectedPrimary;
    };

    const QList<Case> cases{
        {QStringLiteral("volume"), QStringLiteral("42%"), QStringLiteral("Volume"), QStringLiteral("42%")},
        {QStringLiteral("seek"), QStringLiteral("01:23"), QStringLiteral("Seek"), QStringLiteral("01:23")},
        {QStringLiteral("muted"), QString{}, QStringLiteral("Muted"), QStringLiteral("Muted")},
        {QStringLiteral("seekFailed"), QString{}, QStringLiteral("Seek failed"), QStringLiteral("Seek failed")},
        {QStringLiteral("speed"), QStringLiteral("1.25×"), QStringLiteral("Speed"), QStringLiteral("1.25×")},
        {QStringLiteral("track"), QStringLiteral("English"), QStringLiteral("Track"), QStringLiteral("English")},
    };

    for (const Case& testCase : cases) {
        QQmlEngine engine;
        HudStub viewModel(true, testCase.key, testCase.value);
        std::unique_ptr<QObject> overlay = createOverlay(engine, viewModel);
        QVERIFY2(overlay != nullptr, qPrintable(testCase.key));
        QVERIFY(overlay->property("hudVisible").toBool());
        QCOMPARE(overlay->property("labelText").toString(), testCase.expectedLabel);
        QCOMPARE(overlay->property("primaryText").toString(), testCase.expectedPrimary);
        QVERIFY(!overlay->property("enabled").toBool());
        QVERIFY(overlay->findChild<QObject*>(QStringLiteral("playerHudSurface")) != nullptr);
        QVERIFY(overlay->findChild<QObject*>(QStringLiteral("playerHudPrimaryText")) != nullptr);
    }
}

void PlayerHudOverlayTest::hiddenQueueKeepsHudSemanticallyHidden()
{
    QQmlEngine engine;
    HudStub viewModel(false, QStringLiteral("volume"), QStringLiteral("50%"));
    std::unique_ptr<QObject> overlay = createOverlay(engine, viewModel);
    QVERIFY(overlay != nullptr);
    QVERIFY(!overlay->property("hudVisible").toBool());
}

void PlayerHudOverlayTest::fatalErrorSuppressionKeepsHudSemanticallyHidden()
{
    QQmlEngine engine;
    HudStub viewModel(true, QStringLiteral("volume"), QStringLiteral("50%"));
    std::unique_ptr<QObject> overlay = createOverlay(engine, viewModel, true);
    QVERIFY(overlay != nullptr);
    QVERIFY(overlay->property("suppressed").toBool());
    QVERIFY(!overlay->property("hudVisible").toBool());
    QVERIFY(!overlay->property("visible").toBool());
}

void PlayerHudOverlayTest::overlayBoundaryStaysFocused()
{
    const QString source = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/overlays/hud/PlayerHudOverlay.qml"));
    QVERIFY(!source.isEmpty());

    QVERIFY(source.contains(QStringLiteral("property bool suppressed: false")));
    QVERIFY(source.contains(QStringLiteral("&& !root.suppressed")));
    QVERIFY(source.contains(QStringLiteral("visible: !root.suppressed")));
    QVERIFY(source.contains(QStringLiteral("Hud {")));
    QVERIFY(source.contains(QStringLiteral("z: ZOrderTokens.hud")));
    QVERIFY(source.contains(QStringLiteral("enabled: false")));
    QVERIFY(source.contains(QStringLiteral("MotionTokens.hudShowDuration")));
    QVERIFY(source.contains(QStringLiteral("MotionTokens.hudHideDuration")));
    QVERIFY(source.contains(QStringLiteral("MotionTokens.enterEasingType")));
    QVERIFY(source.contains(QStringLiteral("MotionTokens.exitEasingType")));
    QVERIFY(source.contains(QStringLiteral("MotionTokens.enterBezier")));
    QVERIFY(source.contains(QStringLiteral("MotionTokens.exitBezier")));
    QVERIFY(source.contains(QStringLiteral("OpacityTokens.visible")));
    QVERIFY(source.contains(QStringLiteral("OpacityTokens.hidden")));
    QVERIFY(source.contains(QStringLiteral("case \"seekFailed\"")));
    QVERIFY(source.contains(QStringLiteral("case \"speed\"")));
    QVERIFY(source.contains(QStringLiteral("case \"track\"")));
    QVERIFY(!source.contains(QStringLiteral("MotionTokens.resolvedDuration")));
    QVERIFY(!source.contains(QStringLiteral("MotionTokens.commonEasing")));

    QVERIFY(!source.contains(QStringLiteral("Timer {")));
    QVERIFY(!source.contains(QStringLiteral("Toast")));
    QVERIFY(!source.contains(QStringLiteral("Dialog")));
    QVERIFY(!source.contains(QStringLiteral("PlaybackSession")));
    QVERIFY(!source.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive));
    QVERIFY(!source.contains(QStringLiteral("mpv_"), Qt::CaseInsensitive));
}

void PlayerHudOverlayTest::playerScreenAndBootstrapRouteSingleQueue()
{
    const QString screen = readSource(QStringLiteral(
        "src/presentation/qml/screens/player/PlayerScreen.qml"));
    const QString window = readSource(QStringLiteral(
        "src/presentation/qml/shell/MainWindow.qml"));
    const QString bootstrap = readSource(QStringLiteral(
        "src/app/bootstrap/application_bootstrap.cpp"));
    const QString composition = readSource(QStringLiteral(
        "src/app/composition/playback_composition.cpp"));

    QVERIFY(!screen.isEmpty());
    QVERIFY(!window.isEmpty());
    QVERIFY(!bootstrap.isEmpty());
    QVERIFY(!composition.isEmpty());

    QVERIFY(screen.contains(QStringLiteral("property var hudMessageQueue: null")));
    QVERIFY(screen.contains(QStringLiteral("\n    PlayerHudOverlay {\n")));
    QVERIFY(!screen.contains(QStringLiteral("\n        PlayerHudOverlay {\n")));
    QVERIFY(screen.contains(QStringLiteral("viewModel: root.hudMessageQueue")));
    QVERIFY(screen.contains(QStringLiteral("suppressed: root.errorOverlayVisible")));

    QVERIFY(window.contains(QStringLiteral("property var hudMessageQueue: null")));
    QVERIFY(window.contains(QStringLiteral("hudMessageQueue: window.hudMessageQueue")));
    QVERIFY(bootstrap.contains(QStringLiteral("\"hudMessageQueue\"")));
    QVERIFY(bootstrap.contains(QStringLiteral("playbackComposition.hudMessageQueue()")));

    QVERIFY(composition.contains(QStringLiteral("hudMessageQueue_->showSeek")));
    QVERIFY(composition.contains(QStringLiteral("hudMessageQueue_->showSeekFailure")));
    QVERIFY(composition.contains(QStringLiteral("hudMessageQueue_->showVolume")));
    QVERIFY(!composition.contains(QStringLiteral("hudMessageQueue_->showSpeed")));
    QVERIFY(!composition.contains(QStringLiteral("hudMessageQueue_->showTrackChange")));
    QVERIFY(composition.contains(QStringLiteral(
        "submitSeek(absoluteSeconds, SeekMode::Absolute)")));
    QVERIFY(composition.contains(QStringLiteral(
        "submitSeek(deltaSeconds, SeekMode::Relative)")));
    QVERIFY(composition.contains(QStringLiteral("if (!submitVolume(percent))")));
    QVERIFY(composition.contains(QStringLiteral("if (!submitMuted(muted))")));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::PlayerHudOverlayTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "player_hud_overlay_test.moc"
