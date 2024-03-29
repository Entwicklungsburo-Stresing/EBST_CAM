#ifndef DIALOGAXES_H
#define DIALOGAXES_H

#include <QDialog>

namespace Ui {
	class DialogAxes;
}

class DialogAxes : public QDialog
{
	Q_OBJECT

public:
	explicit DialogAxes(QWidget* parent = nullptr);
	~DialogAxes();

	void on_rubberband_valueChanged();

	void on_checkBoxMirrorX_stateChanged();
	void on_checkBoxMirrorY_stateChanged();

private slots:
	void on_buttonBox_rejected();
	void on_spinBoxXmin_valueChanged(int arg1);
	void on_spinBoxXmax_valueChanged(int arg1);
	void on_spinBoxYmin_valueChanged(int arg1);
	void on_spinBoxYmax_valueChanged(int arg1);

private:
	Ui::DialogAxes* ui;
	qreal xmax_old = 0;
	qreal xmin_old = 0;
	qreal ymax_old = 0;
	qreal ymin_old = 0;
};

#endif // DIALOGAXES_H
