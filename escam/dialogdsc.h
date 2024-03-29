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
	explicit DialogDSC(QWidget* parent = nullptr);
	~DialogDSC();
	void initDialogDsc();

private:
	Ui::DialogDSC* ui;
private slots:
	void on_pushButton_RS_DSC_1_clicked();
	void on_pushButton_RS_DSC_2_clicked();
	void on_comboBox_DIR_DSC_1_currentIndexChanged(int index);
	void on_comboBox_DIR_DSC_2_currentIndexChanged(int index);
	void on_pushButton_update_cur_clicked();
public slots:
	void updateDSC();
};

#endif // DIALOGDSC_H
