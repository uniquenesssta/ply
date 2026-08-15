#include <QColor>
#include <QDirIterator>
#include <QFile>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>
#include <QSignalSpy>
#include <QStringList>
#include <QUrl>
#include <QtTest>

#include <cmath>
#include <memory>

namespace player::presentation::qml {
namespace {
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

bool closeEnough(double left, double right)
{
    return std::abs(left - right) <= 0.000001;
}

QStringList presentationBoundaryViolations()
{
    QStringList violations;
    const QString root =
        QStringLiteral(PLAYER_SOURCE_DIR "/src/presentation/qml/feedback");
    QDirIterator iterator(
        root,
        {QStringLiteral("*.qml")},
        QDir::Files,
        QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        const QString path = iterator.next();
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            violations.append(QStringLiteral("cannot read %1").arg(path));
            continue;
        }

        const QString content = QString::fromUtf8(file.readAll());
        if (content.contains(QStringLiteral("PlaybackSession"), Qt::CaseSensitive)
            || content.contains(QStringLiteral("libmpv"), Qt::CaseInsensitive)
            || content.contains(QStringLiteral("mpv_"), Qt::CaseInsensitive)) {
            violations.append(
                QStringLiteral("%1 contains playback/backend vocabulary").arg(path));
        }
        if (content.contains(QStringLiteral("Timer {"), Qt::CaseSensitive)) {
            violations.append(
                QStringLiteral("%1 owns timeout behavior").arg(path));
        }
    }

    return violations;
}
} // namespace

class FeedbackControlTest final : public QObject
{
    Q_OBJECT

private slots:
    void rolesLoadAndStayDistinct();
    void toastUsesToastSurface();
    void moduleRemainsPresentationOnly();
};

void FeedbackControlTest::rolesLoadAndStayDistinct()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Feedback
Item {
    Toast { objectName: "toast"; title: "Saved"; message: "Done" }
    ErrorFeedback { objectName: "error"; title: "Error"; detail: "Failed"; actionText: "Retry" }
    LoadingFeedback { objectName: "loading"; title: "Loading"; detail: "Please wait" }
    BufferingFeedback { objectName: "buffering"; title: "Buffering"; detail: "Waiting for data" }
    EndedFeedback { objectName: "ended"; title: "Finished"; detail: "Playback ended" }
    EmptyFeedback { objectName: "empty"; title: "Empty"; detail: "Open media"; actionText: "Open" }
}
)QML"), QUrl(QStringLiteral("qrc:/FeedbackRoles.qml")));

    QVERIFY2(resolve(component), qPrintable(diagnostics(component)));
    QVERIFY2(component.isReady(), qPrintable(diagnostics(component)));
    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(diagnostics(component)));

    QObject* toast = object->findChild<QObject*>(QStringLiteral("toast"));
    QObject* error = object->findChild<QObject*>(QStringLiteral("error"));
    QObject* loading = object->findChild<QObject*>(QStringLiteral("loading"));
    QObject* buffering = object->findChild<QObject*>(QStringLiteral("buffering"));
    QObject* ended = object->findChild<QObject*>(QStringLiteral("ended"));
    QObject* empty = object->findChild<QObject*>(QStringLiteral("empty"));
    QVERIFY(toast && error && loading && buffering && ended && empty);

    QCOMPARE(toast->property("feedbackRole").toString(), QStringLiteral("toast"));
    QCOMPARE(error->property("feedbackRole").toString(), QStringLiteral("error"));
    QCOMPARE(loading->property("feedbackRole").toString(), QStringLiteral("loading"));
    QCOMPARE(buffering->property("feedbackRole").toString(), QStringLiteral("buffering"));
    QCOMPARE(ended->property("feedbackRole").toString(), QStringLiteral("ended"));
    QCOMPARE(empty->property("feedbackRole").toString(), QStringLiteral("empty"));

    QVERIFY(toast->property("tone").isValid());
    QVERIFY(error->property("actionText").isValid());
    QVERIFY(!loading->property("actionText").isValid());
    QVERIFY(!buffering->property("actionText").isValid());
    QVERIFY(!ended->property("actionText").isValid());
    QVERIFY(empty->property("actionText").isValid());
    QVERIFY(error->findChild<QObject*>(QStringLiteral("feedbackAction")) != nullptr);
    QVERIFY(loading->findChild<QObject*>(QStringLiteral("feedbackAction")) == nullptr);
    QVERIFY(buffering->findChild<QObject*>(QStringLiteral("feedbackAction")) == nullptr);
    QVERIFY(ended->findChild<QObject*>(QStringLiteral("feedbackAction")) == nullptr);
    QVERIFY(empty->findChild<QObject*>(QStringLiteral("feedbackAction")) != nullptr);
    QVERIFY(closeEnough(error->property("z").toDouble(), 35.0));
    QVERIFY(closeEnough(loading->property("z").toDouble(), 35.0));
    QVERIFY(closeEnough(buffering->property("z").toDouble(), 35.0));
    QVERIFY(closeEnough(ended->property("z").toDouble(), 35.0));
    QVERIFY(closeEnough(empty->property("z").toDouble(), 35.0));
}

void FeedbackControlTest::toastUsesToastSurface()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(QByteArrayLiteral(R"QML(
import QtQuick
import Player.Presentation.Feedback
Toast { title: "Done"; message: "Operation completed" }
)QML"), QUrl(QStringLiteral("qrc:/ToastContract.qml")));

    QVERIFY2(resolve(component), qPrintable(diagnostics(component)));
    QVERIFY2(component.isReady(), qPrintable(diagnostics(component)));
    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(diagnostics(component)));

    QCOMPARE(object->property("cornerRadius").toInt(), 22);
    QVERIFY(closeEnough(object->property("fillAlpha").toDouble(), 0.52));
    QCOMPARE(object->property("backdropBlurRadius").toInt(), 18);
    QCOMPARE(object->property("shadowRadius").toInt(), 16);
    QVERIFY(closeEnough(object->property("shadowYOffset").toDouble(), 5.0));
    QVERIFY(closeEnough(object->property("shadowAlpha").toDouble(), 0.08));
    QCOMPARE(object->property("contentPadding").toInt(), 18);
    QVERIFY(closeEnough(object->property("z").toDouble(), 80.0));
    QVERIFY(object->property("implicitWidth").toDouble() > 0.0);
    QVERIFY(object->property("implicitHeight").toDouble() > 0.0);
}

void FeedbackControlTest::moduleRemainsPresentationOnly()
{
    const QStringList violations = presentationBoundaryViolations();
    QVERIFY2(
        violations.isEmpty(),
        qPrintable(violations.join(QLatin1Char('\n'))));
}

} // namespace player::presentation::qml

int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    player::presentation::qml::FeedbackControlTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "feedback_control_test.moc"
