#include "modalith_controller.hpp"
#include "plot_items.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

int main(int argc, char* argv[]) {
  QGuiApplication application(argc, argv);
  application.setOrganizationName("Modalith Engineering");
  application.setApplicationName("Modalith Studio");
  application.setApplicationVersion("0.2.0");
  QQuickStyle::setStyle("Material");

  qmlRegisterType<SpotPlotItem>("Modalith", 1, 0, "SpotPlot");
  qmlRegisterType<RayFanPlotItem>("Modalith", 1, 0, "RayFanPlot");
  qmlRegisterType<OpticalLayoutItem>("Modalith", 1, 0, "OpticalLayout");

  ModalithController controller;
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("modalithController", &controller);
  engine.rootContext()->setContextProperty("surfaceModel", controller.surfaceModel());
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                   &application, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("ModalithApp", "Main");
  return application.exec();
}
