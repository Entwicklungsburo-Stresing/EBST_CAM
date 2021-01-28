#include "lsc.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  QString hi = QString::fromStdString(helloworld());
  w.setText(hi);

  return app.exec();
}
