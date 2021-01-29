#include "lsc.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  uint16_t data[1088];
  for(uint16_t i=0; i<1088; i++)
      data[i]=i;
  w.setChartData(data,1088);
  return app.exec();
}
