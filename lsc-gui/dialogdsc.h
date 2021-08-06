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
private slots:
	void on_pushButton_RS_DSC_1_clicked();
	void on_pushButton_RS_DSC_2_clicked();
	void on_pushButton_RS_DSC_3_clicked();
	void on_comboBox_DIR_DSC_1_currentIndexChanged();
	void on_comboBox_DIR_DSC_2_currentIndexChanged();
	void on_comboBox_DIR_DSC_3_currentIndexChanged();
	void on_pushButton_update_act_clicked();
public slots:
	void updateDSC();
};

#endif // DIALOGDSC_H
