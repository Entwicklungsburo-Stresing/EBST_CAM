#include "lsc.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  Lsc lsc;
  lsc.initMeasurement();
  lsc.startMeasurement();
  uint16_t data[1088];
  lsc.returnFrame(0,0,0,0,data,1088);
  w.setChartData(data,1088);
  return app.exec();
}
