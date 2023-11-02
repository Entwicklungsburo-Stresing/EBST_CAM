#include "myqchartview.h"

MyQChartView::MyQChartView(QWidget* parent) : QChartView(parent)
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

void MyQChartView::mouseReleaseEvent(QMouseEvent* event)
{
	QChartView::mouseReleaseEvent(event);
	emit rubberBandChanged();
}

/**
 * Displays x and y value on a tooltip in the chart when hovering over it
 */
void MyQChartView::mouseMoveEvent(QMouseEvent* event)
{
	QChartView::mouseMoveEvent(event);
	QPoint pos = event->pos();
	QPointF mappedPos = chart()->mapToValue(pos);
	QPointF nearestPoint = findNearestPoint(mappedPos.x());
	if (nearestPoint.x() < 0 || nearestPoint.y() < 0) return;

	QString toolTip = QString("X: %1, Y: %2").arg(nearestPoint.x()).arg(nearestPoint.y());
	QToolTip::showText(mapToGlobal(pos), toolTip, this);
}

/**
 * Returns the nearest y value for given x value.
 *
 * \param xValue
 * \return nearestPoint
 */
QPointF MyQChartView::findNearestPoint(qreal xValue) {
	qreal roundedXValue = std::floor(xValue);
	if (chart()->series().empty() || roundedXValue < 0) return QPointF(-1, -1);
	const QLineSeries* series = static_cast<const QLineSeries*>(chart()->series().at(0));
	if (roundedXValue > series->points().last().x()) return QPointF(-1, -1);
	QPointF nearestPoint;
	qreal minDistance = std::numeric_limits<qreal>::max();


	if (roundedXValue == series->points().first().x())
	{
		nearestPoint = series->points().first();
	}
	else
	{
		for (const QPointF& point : series->points())
		{
			qreal distance = qAbs(point.x() - roundedXValue);
			if (distance < minDistance)
			{
				minDistance = distance;
				nearestPoint = point;
			}
		}
	}
		return nearestPoint;
	}

