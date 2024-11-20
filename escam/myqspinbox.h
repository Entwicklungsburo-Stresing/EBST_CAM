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

