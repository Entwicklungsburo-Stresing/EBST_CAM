#include "myqchartview.h"

MyQChartView::MyQChartView(QWidget* parent) : QChartView(parent)
{
	chart()->legend()->hide();
	chart()->legend()->setAlignment(Qt::AlignBottom);
	setRubberBand(QChartView::RectangleRubberBand);
	setMouseTracking(true);
	QSettings settings;
	curr_xmax = settings.value(settingPixelPath, settingPixelDefault).toReal() - 1;
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
	QList<QPointF> nearestPointList = findNearestPoint(mappedPos.x());
	if (nearestPointList.first().x() < 0 || nearestPointList.first().y() < 0) return;
	QString toolTip = QString("X:  %1").arg(nearestPointList.first().x());
	for (int i = 0; i < nearestPointList.size(); i++) {
		toolTip.append(QString(", Y%1: %2").arg(i).arg(nearestPointList.at(i).y()));
	}
	QToolTip::showText(mapToGlobal(pos), toolTip, this, QRect(), 1000);
}

/**
 * Returns the nearest y value for given x value.
 *
 * \param xValue
 * \return nearestPoint
 */
QList<QPointF> MyQChartView::findNearestPoint(qreal xValue) {
	QList<QPointF> pointList;
	pointList.append(QPointF(-1, -1)); //Used to exit tooltip creation if chart has no data

	qreal roundedXValue = std::floor(xValue); //Used to not display next value early

	if (chart()->series().empty() || roundedXValue < 0) return pointList;

	QList<QAbstractSeries*> seriesList = chart()->series();
	const QLineSeries* series = static_cast<const QLineSeries*>(chart()->series().at(0));
	
	if (roundedXValue > series->points().last().x()) return pointList;
	
	pointList.pop_front(); //Deletes exit value created earlier

	//Loops trough the series and appends points to the pointList depending on how many series there are in the chart
	for (int i = 0; i < seriesList.size(); i++)
	{
		QPointF nearestPoint;
		qreal minDistance = std::numeric_limits<qreal>::max();
		const QLineSeries* currentSeries = static_cast<const QLineSeries*>(chart()->series().at(i));
		//Check if x is the first x value in the series
		if (roundedXValue == currentSeries->points().first().x())
		{
			nearestPoint = currentSeries->points().first();
		}
		else
		{
			//Loop trough the points to find nearest y value to given x
			for (const QPointF& point : currentSeries->points())
			{
				qreal distance = qAbs(point.x() - roundedXValue);
				if (distance < minDistance)
				{
					minDistance = distance;
					nearestPoint = point;
				}
			}
		}
		pointList.append(nearestPoint);
	}
		return pointList;
	}

