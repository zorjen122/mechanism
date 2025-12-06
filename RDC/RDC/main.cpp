#include "RdpClient.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QApplication app(argc, argv);

  // 设置 Qt Quick Controls 样式
  QQuickStyle::setStyle("Fusion");

  // 注册 RdpClient 类型到 QML
  qmlRegisterType<RdpClient>("RDC", 1, 0, "RdpClient");

  QQmlApplicationEngine engine;
  engine.load(QUrl(QStringLiteral("qrc:/qt/qml/rdc/main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
