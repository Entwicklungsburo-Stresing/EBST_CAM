#include "myqchartview.h"

MyQChartView::MyQChartView(QWidget *parent) : QChartView(parent)
{
	chart()->legend()->hide();
	setRubberBand(QChartView::RectangleRubberBand);
	setMouseTracking(true);
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

/**
 * Displays correct x and y value.
 * Conflicts with mouseReleaseEvent and disables rubberBanding.
 */
 void MyQChartView::mouseMoveEvent(QMouseEvent* event)
{
	QChartView::mouseMoveEvent(event);
	QPointF point = chart()->mapToValue(event->pos());
	//QString coordinates = QString("X: %1, Y: %2").arg(point.x()).arg(point.y());
	emit mouseMoved(point);
}

