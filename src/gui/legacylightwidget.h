#ifndef LEGACYLIGHTWIDGET_H
#define LEGACYLIGHTWIDGET_H

#include <QWidget>

namespace Ui {
class LegacyLightWidget;
}

class LegacyLightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LegacyLightWidget(QWidget *parent = nullptr);
    ~LegacyLightWidget();

private:
    Ui::LegacyLightWidget *ui;
};

#endif // LEGACYLIGHTWIDGET_H
