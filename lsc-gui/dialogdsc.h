#ifndef DIALOGDSC_H
#define DIALOGDSC_H

#include <QDialog>

namespace Ui {
class DialogDSC;
}

class DialogDSC : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDSC(QWidget *parent = nullptr);
    ~DialogDSC();

private:
    Ui::DialogDSC *ui;
};

#endif // DIALOGDSC_H
