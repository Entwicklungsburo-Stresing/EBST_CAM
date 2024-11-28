/*****************************************************************//**
 * @file   myqspinbox.h
 * @brief  Class MyQSpinBox for modyfing QSpinBox.
 * 
 * @author Florian Hahn
 * @date   29.01.2021
 *********************************************************************/

#pragma once

#include <QSpinBox>

class MyQSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    MyQSpinBox(QWidget* parent=0);
protected:
    bool valueBeingSet=false;
public slots:
    void setValue(int val);
private slots:
    void On_valueChanged(int val);
signals:
    void valueManuallyChanged(int val);
};

