#ifndef CROSSHAIRS_H
#define CROSSHAIRS_H

#include <QtWidgets/QGraphicsItem>
class QChart;

class Crosshairs
{
public:
    Crosshairs(QChart* chart);
    void updatePosition(QPointF position);

private:
    QGraphicsLineItem* m_xLine, * m_yLine;
    QGraphicsTextItem* m_xText, * m_yText;
    QChart* m_chart;
};

#endif // CROSSHAIRS_H