#include "legacylightwidget.h"
#include "ui_legacylightwidget.h"

LegacyLightWidget::LegacyLightWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LegacyLightWidget)
{
    ui->setupUi(this);
}

LegacyLightWidget::~LegacyLightWidget()
{
    delete ui;
}
