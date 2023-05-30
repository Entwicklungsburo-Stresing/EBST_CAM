#include "myqchartview.h"

MyQChartView::MyQChartView(QWidget *parent) : QChartView(parent)
{
	chart()->legend()->hide();
	setRubberBand(QChartView::RectangleRubberBand);
	QSettings settings;
	curr_xmax = settings.value(settingPixelPath, settingPixelDefault).toReal();
	if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble() == 2)
		curr_ymax = 0x3FFF;
	else
		curr_ymax = 0xFFFF;
}

void MyQChartView::mouseReleaseEvent(QMouseEvent *event)
{
	QChartView::mouseReleaseEvent(event);
	emit rubberBandChanged();
}
