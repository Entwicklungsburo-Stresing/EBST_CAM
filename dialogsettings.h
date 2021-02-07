#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QSettings>
#include "lsc.h"

#define NOS_DEFAULT 1000
#define NOB_DEFAULT 2
#define CAMCNT_DEFAULT 1
#define TOR_DEFAULT xck_tor
#define THEME_DEFAULT lighttheme
#define SETTING_NOS "measurement/nos"
#define SETTING_NOB "measurement/nob"
#define SETTING_CAMCNT "camerasetup/camcnt"
#define SETTING_TOR "camerasetup/tor"
#define SETTING_THEME "appearance/theme"

enum theme
{
    lighttheme = 0,
    darktheme = 1,
};

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QSettings* settings, QWidget *parent = nullptr);
    ~DialogSettings();
signals:
    void settings_saved();
private:
    Ui::DialogSettings *ui;
    QSettings* _settings;
private slots:
    void on_accepted();
};

#endif // DIALOGSETTINGS_H
