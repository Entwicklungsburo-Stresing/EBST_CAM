#include "myqspinbox.h"

/**
 * @brief Constructor of class MyQSpinBox.
 * Connects signal valueChanged with custom slot On_valueChanged.
 * @param parent
 */
MyQSpinBox::MyQSpinBox(QWidget* parent)
        : QSpinBox(parent)
{
    connect(this, qOverload<int>(&MyQSpinBox::valueChanged), this, &MyQSpinBox::On_valueChanged);
}

/**
 * @brief Overrides the slot of QSpinBox::setValue().
 * Calls QSpinBox::setValue() but also add flag valueBeingSet.
 * @param val
 */
void MyQSpinBox::setValue(int val)
{
    valueBeingSet = true;
    QSpinBox::setValue(val);
    valueBeingSet = false;
}

/**
 * @brief Custom slot. Checks the flag valueBeingSet before emitting the signal valueManuallyChanged.
 * @param val
 */
void MyQSpinBox::On_valueChanged(int val)
{
    if(!valueBeingSet)
        emit valueManuallyChanged(val);
}
