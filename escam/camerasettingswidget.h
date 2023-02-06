#ifndef CAMERASETTINGSWIGET_H
#define CAMERASETTINGSWIGET_H


#include <QWidget>
#include <QSettings>
#include "ui_camerasettingswidget.h"
#include "dialogsettings.h"
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class CameraSettingsWidgetClass; }
QT_END_NAMESPACE

class CameraSettingsWidget : public QWidget
{
	Q_OBJECT
public:
	CameraSettingsWidget(QWidget *parent = nullptr);
	~CameraSettingsWidget();
public slots:
	void on_accepted();
private:
	Ui::CameraSettingsWidgetClass *ui;
	QSettings settings;
};

#endif // CAMERASETTINGSWIGET_H