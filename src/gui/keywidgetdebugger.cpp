#include "keywidgetdebugger.h"
#include "ui_keywidgetdebugger.h"
#include "keywidgetlayout.h"
#include "keywidget.h"
#include "keymap.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QListWidgetItem>
#include <QSignalBlocker>
#include <limits>

KeyWidgetDebugger::KeyWidgetDebugger(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::KeyWidgetDebugger), w(new KeyWidget(this)), m(KeyMap::Model::NO_MODEL), l(KeyMap::Layout::NO_LAYOUT)
{
    ui->setupUi(this);
    ui->devw->setMaximum(std::numeric_limits<short>::max());
    ui->devh->setMaximum(std::numeric_limits<short>::max());

    KeyWidgetLayout* wl = new KeyWidgetLayout;
    wl->addItem(new QWidgetItem(w));
    wl->addItem(new QWidgetItem(new QWidget(this)));
    ui->verticalLayout->addLayout(wl);
    ui->layoutComboBox->addItems(KeyMap::layoutList);
    for(int i = KeyMap::Model::NO_MODEL + 1; i < KeyMap::Model::_MODEL_MAX; i++)
        ui->modelComboBox->addItem(KeyMap::getModel(static_cast<KeyMap::Model>(i)));

    ui->layoutComboBox->setCurrentIndex(KeyMap::Layout::US);
    ui->modelComboBox->setCurrentIndex(KeyMap::Model::K95P);
    connect(w, &KeyWidget::selectionChanged, this, [this](const QStringList& sl){
        if(!sl.count())
            return;
        QList<QListWidgetItem*> list = ui->keyList->findItems(sl.at(0), Qt::MatchExactly);
        if(!list.count())
            return;
        ui->keyList->setCurrentItem(list.at(0));
    });
}

KeyWidgetDebugger::~KeyWidgetDebugger()
{
    delete ui;
}

void KeyWidgetDebugger::on_lightingCheckBox_toggled(bool checked)
{
    w->rgbMode(checked);
}

void KeyWidgetDebugger::updateMap()
{
    map = KeyMapDebug(m, l);
    {
        const QSignalBlocker b5(ui->devw);
        const QSignalBlocker b6(ui->devh);
        ui->devw->setValue(map.keyWidth);
        ui->devh->setValue(map.keyHeight);
    }
    w->map(map);
    ui->keyList->clear();
    ui->keyList->addItems(map.keys());
    ui->keyList->sortItems();
}

void KeyWidgetDebugger::on_layoutComboBox_currentIndexChanged(int arg1)
{
    l = static_cast<KeyMap::Layout>(arg1);
    updateMap();
}

void KeyWidgetDebugger::on_modelComboBox_currentIndexChanged(int arg1)
{
    m = KeyMap::getModel(ui->modelComboBox->itemText(arg1));
    updateMap();
}

void KeyWidgetDebugger::on_keyList_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    if(!current)
        return;
    Key k = map.key(current->text());
    // Block signals so that valueChanged events aren't called
    const QSignalBlocker b1(ui->x);
    const QSignalBlocker b2(ui->y);
    const QSignalBlocker b3(ui->w);
    const QSignalBlocker b4(ui->h);
    ui->x->setValue(k.x);
    ui->y->setValue(k.y);
    ui->w->setValue(k.width);
    ui->h->setValue(k.height);
    w->setSelection(QStringList(current->text()));
}

#define HANDLE_SPINBOX_VAL(arg, val) \
    if(!ui->keyList->currentItem()) \
        return; \
    map[ui->keyList->currentItem()->text()].arg = val; \
    w->map(map);

void KeyWidgetDebugger::on_x_valueChanged(int arg1)
{
    HANDLE_SPINBOX_VAL(x, arg1);
}

void KeyWidgetDebugger::on_y_valueChanged(int arg1)
{
    HANDLE_SPINBOX_VAL(y, arg1);
}

void KeyWidgetDebugger::on_w_valueChanged(int arg1)
{
    HANDLE_SPINBOX_VAL(width, arg1);
}

void KeyWidgetDebugger::on_h_valueChanged(int arg1)
{
    HANDLE_SPINBOX_VAL(height, arg1);
}

void KeyWidgetDebugger::on_showSelectionSurfaces_toggled(bool checked)
{
    w->setDebug(checked);
}

void KeyWidgetDebugger::on_devw_valueChanged(int arg1)
{
    map.keyWidth = arg1;
    w->map(map);
}

void KeyWidgetDebugger::on_devh_valueChanged(int arg1)
{
    map.keyHeight = arg1;
    w->map(map);
}
