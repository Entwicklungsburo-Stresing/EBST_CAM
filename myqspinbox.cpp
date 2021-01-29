#include "myqspinbox.h"

MyQSpinBox::MyQSpinBox(QWidget* parent)
        : QSpinBox(parent)
{
    connect(this,SIGNAL(valueChanged(int)),this,SLOT(On_valueChanged(int)));
}

void MyQSpinBox::setValue(int val)
{
    valueBeingSet = true;
    QSpinBox::setValue(val);
    valueBeingSet = false;
}

void MyQSpinBox::On_valueChanged(int val)
{
    if(!valueBeingSet)
        emit valueManuallyChanged(val);
}
