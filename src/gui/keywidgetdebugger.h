#ifndef KEYWIDGETDEBUGGER_H
#define KEYWIDGETDEBUGGER_H

#include <QListWidgetItem>
#include <QWidget>
#include "keywidget.h"

namespace Ui {
class KeyWidgetDebugger;
}

class KeyWidgetDebugger : public QWidget
{
    Q_OBJECT

public:
    explicit KeyWidgetDebugger(QWidget* parent = nullptr);
    ~KeyWidgetDebugger();

private slots:
    void on_lightingCheckBox_toggled(bool checked);
    void on_layoutComboBox_currentIndexChanged(int arg1);
    void on_modelComboBox_currentIndexChanged(int arg1);
    void on_keyList_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void on_x_valueChanged(int arg1);
    void on_y_valueChanged(int arg1);
    void on_w_valueChanged(int arg1);
    void on_h_valueChanged(int arg1);
    void on_showSelectionSurfaces_toggled(bool checked);
    void on_devw_valueChanged(int arg1);
    void on_devh_valueChanged(int arg1);

private:
    Ui::KeyWidgetDebugger* ui;
    KeyWidget* w;
    KeyMap::Model m;
    KeyMap::Layout l;
    KeyMapDebug map;
    void updateMap();
};

#endif // KEYWIDGETDEBUGGER_H
