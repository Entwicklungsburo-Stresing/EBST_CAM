#include "myqchartview.h"

MyQChartView::MyQChartView(QWidget *parent) : QtCharts::QChartView(parent)
{
	chart()->legend()->hide();
	setRubberBand(QChartView::RectangleRubberBand);
	QSettings settings;
	curr_xmax = settings.value(settingPixelPath, settingPixelDefault).toReal();
	if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toUInt() == 2)
		curr_ymax = 0x3FFF;
	else
		curr_ymax = 0xFFFF;
}

void MyQChartView::mouseReleaseEvent(QMouseEvent *event)
{
	QChartView::mouseReleaseEvent(event);
	emit rubberBandChanged();
}
