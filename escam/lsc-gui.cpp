#include "lsc-gui.h"
#include <QApplication>

MainWindow* mainWindow;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("Entwicklungsbuero Stresing");
	QCoreApplication::setOrganizationDomain("stresing.de");
	QCoreApplication::setApplicationName("LSC");
	mainWindow = new MainWindow;
	mainWindow->show();
	return app.exec();
}
