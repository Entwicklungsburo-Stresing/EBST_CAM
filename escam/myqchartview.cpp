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
	QPoint pos = event->pos();
	QPointF mappedPos = chart()->mapToValue(pos);
	QPointF nearestPoint = findNearestPoint(mappedPos.x());
	if (nearestPoint.isNull()) return;

	QString toolTip = QString("X: %1, Y: %2").arg(nearestPoint.x()).arg(nearestPoint.y());
	QToolTip::showText(mapToGlobal(pos), toolTip, this);
}

 QPointF MyQChartView::findNearestPoint(qreal xValue) {
	 QPointF nearestPoint;
	 if (chart()->series().empty()) return nearestPoint;
	 const QLineSeries* series = static_cast<const QLineSeries*>(chart()->series().at(0));
	 qreal minDistance = std::numeric_limits<qreal>::max();

	 for (const QPointF& point : series->points()) {
		 qreal distance = qAbs(point.x() - xValue);
		 if (distance < minDistance) {
			 minDistance = distance;
			 nearestPoint = point;
		 }
	 }
	 return nearestPoint;
 }

