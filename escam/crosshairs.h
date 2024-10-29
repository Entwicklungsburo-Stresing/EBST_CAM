#ifndef CROSSHAIRS_H
#define CROSSHAIRS_H

#include <QtWidgets/QGraphicsItem>
#include <dialogsettings.h>
#include <QtCharts/QChart>

QT_CHARTS_USE_NAMESPACE 

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

#endif // CROSSHAIRS_H