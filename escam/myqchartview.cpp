#include "myqchartview.h"

MyQChartView::MyQChartView(QWidget *parent) : QtCharts::QChartView(parent)
{
	chart()->legend()->hide();
	setRubberBand(QChartView::RectangleRubberBand);
	QSettings settings;
	curr_xmax = settings.value(settingPixelPath, settingPixelDefault).toReal();
}

void MyQChartView::mouseReleaseEvent(QMouseEvent *event)
{
	QChartView::mouseReleaseEvent(event);
	emit rubberBandChanged();
}
