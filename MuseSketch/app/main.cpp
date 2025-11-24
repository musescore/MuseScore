#include "ScoreEngine.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickImageProvider>

class ScoreImageProvider : public QQuickImageProvider {
public:
  ScoreImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override {
    ScoreEngine engine;
    // In a real app, we'd keep the engine persistent and load a specific score
    // based on 'id'
    return engine.renderPage(0);
  }
};

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  engine.addImageProvider("score", new ScoreImageProvider);

  const QUrl url(u"qrc:/MuseSketch/Main.qml"_qs);
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.load(url);

  return app.exec();
}
