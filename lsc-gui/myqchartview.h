#ifndef MYQCHARTVIEW_H
#define MYQCHARTVIEW_H

#include <QtCharts>
#include <dialogsettings.h>

class MyQChartView : public QtCharts::QChartView
{
	Q_OBJECT
public:
	MyQChartView(QWidget *parent = nullptr);
	qreal curr_xmax=1088;
	qreal curr_xmin=0;
	qreal curr_ymax=0xFFFF;
	qreal curr_ymin=0;
protected:
	void mouseReleaseEvent(QMouseEvent *event);
signals:
	void rubberBandChanged();
};

#endif // MYQCHARTVIEW_H