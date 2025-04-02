/*****************************************************************//**
 * @file		crosshairs.h
 * @brief		Class to display crosshairs on the chart.
 * @author		Dennis Vollenweider
 * @date		22.05.2024
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
*********************************************************************/

#pragma once

#include <QtWidgets/QGraphicsItem>
#include <dialogsettings.h>
#include <QtCharts/QChart>

#if (QT_VERSION < QT_VERSION_CHECK(6, 2, 0))
QT_CHARTS_USE_NAMESPACE 
#endif

class Crosshairs
{
public:
	Crosshairs(QChart* chart);
	void updatePosition(QPointF position);

private:
	QGraphicsLineItem* m_xLine, * m_yLine;
	QGraphicsTextItem* m_xText, * m_yText;
	QChart* m_chart;
	QSettings settings;
};
