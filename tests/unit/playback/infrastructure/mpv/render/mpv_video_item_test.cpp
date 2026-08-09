#include "playback/infrastructure/mpv/render/mpv_video_item.h"
#include "playback/infrastructure/mpv/render/mpv_video_renderer.h"

#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtQml/qqml.h>
#include <QtTest>

#include <memory>

namespace player::playback::infrastructure::mpv::render {

class MpvVideoItemTest final : public QObject
{
    Q_OBJECT

private slots:
    void rendererBoundaryIsCreated();
    void synchronizeCopiesResizeDprAndVisibility();
    void qmlCanInstantiateVideoItem();
};

void MpvVideoItemTest::rendererBoundaryIsCreated()
{
    MpvVideoItem item;
    QQuickFramebufferObject::Renderer* renderer = item.createRenderer();

    QVERIFY(renderer != nullptr);
    auto* videoRenderer = dynamic_cast<MpvVideoRenderer*>(renderer);
    QVERIFY(videoRenderer != nullptr);
    delete videoRenderer;
}

void MpvVideoItemTest::synchronizeCopiesResizeDprAndVisibility()
{
    QQuickWindow window;
    MpvVideoItem item(window.contentItem());
    item.setWidth(640.0);
    item.setHeight(360.0);
    item.setVisible(true);

    MpvVideoRenderer renderer;
    renderer.synchronize(&item);

    MpvVideoPresentationState state = renderer.presentationState();
    QCOMPARE(state.logicalSize, QSizeF(640.0, 360.0));
    QCOMPARE(state.devicePixelRatio, window.effectiveDevicePixelRatio());
    QVERIFY(state.visible);

    item.setWidth(1280.0);
    item.setHeight(720.0);
    item.setVisible(false);
    renderer.synchronize(&item);

    state = renderer.presentationState();
    QCOMPARE(state.logicalSize, QSizeF(1280.0, 720.0));
    QCOMPARE(state.devicePixelRatio, window.effectiveDevicePixelRatio());
    QVERIFY(!state.visible);
}

void MpvVideoItemTest::qmlCanInstantiateVideoItem()
{
    const int registrationId = qmlRegisterType<MpvVideoItem>(
        "Player.Render.Test",
        1,
        0,
        "MpvVideoItem");
    QVERIFY(registrationId >= 0);

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(
        QByteArrayLiteral(
            "import QtQuick\n"
            "import Player.Render.Test 1.0\n"
            "MpvVideoItem { width: 320; height: 180; visible: false }\n"),
        QUrl(QStringLiteral("qrc:/tests/MpvVideoItem.qml")));

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(component.errorString()));

    auto* item = qobject_cast<MpvVideoItem*>(object.get());
    QVERIFY(item != nullptr);
    QCOMPARE(item->width(), qreal{320.0});
    QCOMPARE(item->height(), qreal{180.0});
    QVERIFY(!item->isVisible());
}

} // namespace player::playback::infrastructure::mpv::render

int main(int argc, char* argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication application(argc, argv);
    player::playback::infrastructure::mpv::render::MpvVideoItemTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "mpv_video_item_test.moc"
