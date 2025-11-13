#pragma once

#include <QWidget>
#include "ui_ioctrlwidget.h"

class IoctrlWidget : public QWidget
{
	Q_OBJECT

public:
	IoctrlWidget(QWidget *parent = nullptr);
	~IoctrlWidget();

private:
	Ui::IoctrlWidgetClass ui;
};

