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
#include <QFile>
#include <QTextStream>
#include <algorithm>

bool comp(Key& a, Key& b) {
    return strcmp(a.name, b.name) < 0;
}

KeyWidgetDebugger::KeyWidgetDebugger(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::KeyWidgetDebugger), w(new KeyWidget(this)), m(KeyMap::Model::NO_MODEL), l(KeyMap::Layout::NO_LAYOUT)
{
    // FIXME MAKE THIS A BUTTON
    for(int j = KeyMap::Layout::NO_LAYOUT + 1; j < KeyMap::Layout::_LAYOUT_MAX; j++) {
        KeyMap::Layout lay = static_cast<KeyMap::Layout>(j);
        for(int i = KeyMap::Model::NO_MODEL + 1; i < KeyMap::Model::_MODEL_MAX; i++) {
            KeyMap::Model keyMapModel = static_cast<KeyMap::Model>(i);
            QFile f(QString("layout-%1-%2").arg(KeyMap::getModel(keyMapModel), KeyMap::getLayout(lay)));
            f.open(QIODevice::WriteOnly);
            QTextStream ts(&f);
            KeyMapDebug keyMapDbg(keyMapModel, lay);
            QList<Key> ll = keyMapDbg.positions();
            std::sort(ll.begin(), ll.end(), comp);
            for(const Key& k : ll) {
                const QRect bRect = k.boundingRect();
                ts << k.name << " " << bRect.x() << " " << bRect.y() << " " << bRect.width() << " " << bRect.height() << "\n";
            }
            f.close();
        }
    }


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

    if(k.is_auto_rect()) {
        ui->x->setValue(k.rectX());
        ui->y->setValue(k.rectY());
        ui->w->setValue(k.rectWidth());
        ui->h->setValue(k.rectHeight());
        ui->x->setEnabled(true);
        ui->y->setEnabled(true);
        ui->w->setEnabled(true);
        ui->h->setEnabled(true);
    } else {
        ui->x->setEnabled(false);
        ui->y->setEnabled(false);
        ui->w->setEnabled(false);
        ui->h->setEnabled(false);
    }
    w->setSelection(QStringList(current->text()));
}

#define HANDLE_SPINBOX_VAL(arg, val) \
    if(!ui->keyList->currentItem()) \
        return; \
    map[ui->keyList->currentItem()->text()].setRect##arg(val); \
    w->map(map);

void KeyWidgetDebugger::on_x_valueChanged(int arg1)
{
    HANDLE_SPINBOX_VAL(X, arg1);
}

void KeyWidgetDebugger::on_y_valueChanged(int arg1)
{
    HANDLE_SPINBOX_VAL(Y, arg1);
}

void KeyWidgetDebugger::on_w_valueChanged(int arg1)
{
    HANDLE_SPINBOX_VAL(Width, arg1);
}

void KeyWidgetDebugger::on_h_valueChanged(int arg1)
{
    HANDLE_SPINBOX_VAL(Height, arg1);
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
