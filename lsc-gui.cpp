#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include "lsc.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QString hi = QString::fromStdString(helloworld());
  QPushButton hello(hi);
  hello.show();
  return app.exec();
}