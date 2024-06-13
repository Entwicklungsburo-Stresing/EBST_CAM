#include "myqchartview.h"
#include "lsc-gui.h"

MyQChartView::MyQChartView(QWidget* parent) : QChartView(parent)
{
	connect(this, &MyQChartView::rubberBandChanged, this, &MyQChartView::on_rubberBandChanged);
	xCrosshair = new Crosshairs(chart());
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
	return;
}

/**
 * Function is called, when mouse is moved over the chart.
 * Takes the y value of the nearest point to the mouse position and displays it in the labelMouseCoordinates.
 * Creates a crosshair on mouse position.
 */
void MyQChartView::mouseMoveEvent(QMouseEvent* event)
{
	QChartView::mouseMoveEvent(event);
	QPoint pos = event->pos();
	updateLabelMouseCoordinates(pos);
	xCrosshair->updatePosition(pos);
	return;
}

void MyQChartView::updateLabelMouseCoordinates(QPoint mousePos)
{
	QPointF mappedPos = chart()->mapToValue(mousePos);
	QList<QPointF> nearestPointList = findNearestPoint(mappedPos.x());
	if (nearestPointList.first().x() < 0 || nearestPointList.first().y() < 0) return;
	QString toolTip = QString("X:  %1").arg(nearestPointList.first().x());
	for (int i = 0; i < nearestPointList.size(); i++) {
		toolTip.append(QString(", Y%1: %2").arg(i).arg(nearestPointList.at(i).y()));
	}
	mainWindow->ui->labelMouseCoordinates->setText(toolTip);
	return;
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

	int roundedXValue = qRound(xValue); //Used to not display next value early

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

void MyQChartView::setDefaultAxes()
{
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	qreal xmax = 0;
	qreal ymax = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			qreal pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
			if (pixel - 1 > xmax)
				xmax = pixel - 1;
			if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble() == camera_system_3030 && ymax != 0xFFFF)
				ymax = 0x3FFF;
			else
				ymax = 0xFFFF;
			settings.endGroup();
		}
	}
	curr_ymax = ymax;
	curr_xmax = xmax;
	curr_xmin = 0;
	curr_ymin = 0;
	// retrieve axis pointer
	QList<QAbstractAxis*> axes = this->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis0->setMax(curr_xmax);
	axis0->setMin(curr_xmin);
	axis1->setMax(curr_ymax);
	axis1->setMin(curr_ymin);
	return;
}

void MyQChartView::on_rubberBandChanged()
{
	// retrieve axis pointer
	QList<QAbstractAxis*> axes = this->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	// save current axis configuration
	curr_xmax = axis0->max();
	curr_xmin = axis0->min();
	curr_ymax = axis1->max();
	curr_ymin = axis1->min();
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	qreal ymax = 0;
	uint max_pixel = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint cur_pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
			if (max_pixel < cur_pixel)
				max_pixel = cur_pixel;
			qreal cur_ymax;
			if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble() == 2)
				cur_ymax = 0x3FFF;
			else
				cur_ymax = 0xFFFF;
			if (ymax < cur_ymax)
				ymax = cur_ymax;
			settings.endGroup();
		}
	}
	// apply boundaries on axes
	if (axis0->max() > max_pixel)
	{
		curr_xmax = max_pixel - 1;
		axis0->setMax(curr_xmax);
	}
	if (axis0->min() < 0)
	{
		curr_xmin = 0;
		axis0->setMin(0);
	}
	if (axis1->max() > ymax)
	{
		curr_ymax = ymax;
		axis1->setMax(ymax);
	}
	if (axis1->min() < 0)
	{
		curr_ymin = 0;
		axis1->setMin(0);
	}
	on_axes_changed();
	return;
}

/**
 * @brief Sets the data of chartView. This function takes the data in the Qt format QLineSeries.
 * @param series Data as an array of QLineSeries.
 * @param numberOfSets Number of data sets which are given in the array series.
 */
void MyQChartView::setChartData(QLineSeries** series, uint16_t numberOfSets)
{
	QChart* chart = this->chart();
	chart->removeAllSeries();
	for (uint16_t set = 0; set < numberOfSets; set++)
	{
		if (settings.value(settingAxesMirrorXPath).toBool())
		{
			QVector<QPointF> points = series[set]->pointsVector();
			for (int i = 0; i < points.size() / 2; i++)
			{
				points[i].setX(points.size() - i - 1);
				points[points.size() - i - 1].setX(i);
				series[set]->replace(i, points[points.size() - i - 1]);
				series[set]->replace(points.size() - i - 1, points[i]);
			}
		}
		chart->addSeries(series[set]);
	}
	chart->createDefaultAxes();
	QList<QAbstractAxis*> axes = this->chart()->axes();
	if (axes.isEmpty()) return;
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis0->setMax(curr_xmax);
	axis0->setMin(curr_xmin);
	axis1->setMax(curr_ymax);
	axis1->setMin(curr_ymin);
	return;
}

/**
 * @brief This overloaded function takes data with a C pointer and a length, converts it into QLineSeries and passes it to setChartData.
 * @param data Pointer to data.
 * @param length Length of data.
 * @param numberOfSets Number of data sets which are stored in data pointer.
 */
void MyQChartView::setChartData(uint16_t* data, uint32_t* length, uint16_t numberOfSets, QList<QString> lineSeriesNamesList)
{
	// Allocate memory for the pointer array to the QlineSeries.
	QLineSeries** series = static_cast<QLineSeries**>(calloc(numberOfSets, sizeof(QLineSeries*)));
	// Iterate through all data sets.
	uint16_t* cur_data_ptr = data;
	for (uint16_t set = 0; set < numberOfSets; set++)
	{
		// Set the current data set to a new empty QLineSeries.
		series[set] = new QLineSeries(this);
		series[set]->setPointsVisible(pointsVisible);
		series[set]->setName(lineSeriesNamesList[set]);
		// Iterate through all data points for the current data set.
		for (uint16_t i = 0; i < length[set]; i++)
		{
			// Append the current data point to the current data set.
			series[set]->append(i, *(cur_data_ptr));
			cur_data_ptr++;
		}
	}
	setChartData(series, numberOfSets);
	free(series);
	return;
}

void MyQChartView::on_axes_changed()
{
	qsizetype pointsInRect = countPointsInRect();
	qsizetype pointsVisibleThreshold = 50;
	if (pointsInRect < pointsVisibleThreshold && !pointsVisible)
	{
		pointsVisible = true;
		mainWindow->loadCameraData();
	}
	else if (pointsInRect >= pointsVisibleThreshold && pointsVisible)
	{
		pointsVisible = false;
		mainWindow->loadCameraData();
	}
	return;
}

// https://stackoverflow.com/questions/52777058/how-to-get-points-that-are-displayed-while-zoomed-in
qsizetype MyQChartView::countPointsInRect()
{
	QRectF inScene = this->chart()->plotArea();
	QPolygonF inChart = this->chart()->mapFromScene(inScene);
	QRectF inChartRect = inChart.boundingRect();
	QPointF inItem1 = this->chart()->mapToValue(inChartRect.topLeft(), this->chart()->series()[0]);
	QPointF inItem2 = this->chart()->mapToValue(inChartRect.bottomRight(), this->chart()->series()[0]);
	QRectF rect = QRectF(inItem1, inItem2).normalized();
	QVector<QPointF> result;
	QLineSeries* series = static_cast<QLineSeries*>(this->chart()->series()[0]);
	QList<QPointF> points = series->points();
	std::copy_if(points.begin(), points.end(), std::back_inserter(result),
		[rect](auto& p) { return rect.contains(p); });
	return result.size();
}

