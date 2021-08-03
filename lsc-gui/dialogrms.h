#ifndef DIALOGRMS_H
#define DIALOGRMS_H

#include <QDialog>

namespace Ui {
class DialogRMS;
}

class DialogRMS : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRMS(QWidget *parent = nullptr);
    ~DialogRMS();

private:
    Ui::DialogRMS *ui;
};

#endif // DIALOGRMS_H
