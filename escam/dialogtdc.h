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
	
//	Lsc lsc;
public slots:
	void updateTDC(uint32_t tdc1, uint32_t tdc2 );

private:
    Ui::DialogTDC *ui;
};

#endif // DIALOGTDC_H
