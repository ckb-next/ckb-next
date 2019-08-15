#ifndef NOWHEELCOMBOBOX_H
#define NOWHEELCOMBOBOX_H

#include <QComboBox>

class NoWheelComboBox : public QComboBox
{
    Q_OBJECT
public:
    NoWheelComboBox(QWidget* parent = nullptr) : QComboBox(parent) {}
    void wheelEvent(QWheelEvent* e);
};

#endif // NOWHEELCOMBOBOX_H
