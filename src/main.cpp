#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "audiostrobe.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    // Register AudioStrobe class to use with QML
    qmlRegisterType<AudioStrobe>("StrobeTuner",1,0,"AudioStrobe");

    // TODO For now AudioStrobe can throw in constructor, that should be addressed somehow

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
