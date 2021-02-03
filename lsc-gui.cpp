#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("Entwicklungsbuero Stresing");
  QCoreApplication::setOrganizationDomain("stresing.de");
  QCoreApplication::setApplicationName("LSC");
  MainWindow w;
  w.show();
  return app.exec();
}
