/*****************************************************************//**
 * @file   myqchartview.h
 * @brief  Class MyQChartView for modifying QChartView
 * 
 * @author Florian Hahn
 * @date   01.07.2021
 *********************************************************************/

#pragma once
#include <QtCharts>
#include <dialogsettings.h>
#include "crosshairs.h"

class MyQChartView : public QChartView
{
	Q_OBJECT
public:
	MyQChartView(QWidget *parent = nullptr);
	qreal curr_xmax=1088;
	qreal curr_xmin=0;
	qreal curr_ymax=0xFFFF;
	qreal curr_ymin=0;
	bool pointsVisible = false;
	void setChartData(QLineSeries** series, uint16_t numberOfSets);
	void setChartData(uint16_t* data, uint32_t* length, uint16_t numberOfSets, QList<QString> lineSeriesNamesList);
public slots:
	void setDefaultAxes();
	void on_axes_changed();
	void on_rubberBandChanged();
	void updateLabelMouseCoordinates(QPoint mousePos);
signals:
	void rubberBandChanged();
protected:
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	QList<QPointF> findNearestPoint(qreal xValue);
private:
	Crosshairs *xCrosshair;
	QSettings settings;
	qsizetype countPointsInRect();
};
