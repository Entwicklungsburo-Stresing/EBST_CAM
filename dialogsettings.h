#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QSettings>

#define NOS_DEFAULT 1000
#define NOB_DEFAULT 2
#define SETTING_NOS "measurement/nos"
#define SETTING_NOB "measurement/nob"
#define SETTING_CAMCNT "camerasetup/camcnt"

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QSettings* settings, QWidget *parent = nullptr);
    ~DialogSettings();

private:
    Ui::DialogSettings *ui;
    QSettings* settings;
private slots:
    void on_accepted();
};

#endif // DIALOGSETTINGS_H
