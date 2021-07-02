#ifndef DIALOGTDC_H
#define DIALOGTDC_H

#include <QDialog>

namespace Ui {
class DialogTDC;
}

class DialogTDC : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTDC(QWidget *parent = nullptr);
    ~DialogTDC();
public slots:
	void updateTDC();

private:
    Ui::DialogTDC *ui;
};

#endif // DIALOGTDC_H
