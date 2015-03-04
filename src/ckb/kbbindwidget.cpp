#include <QMessageBox>
#include "kbbindwidget.h"
#include "modeselectdialog.h"
#include "rebindwidget.h"
#include "ui_kbbindwidget.h"

KbBindWidget::KbBindWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KbBindWidget), bind(0), profile(0)
{
    ui->setupUi(this);
    ui->keyWidget->rgbMode(false);
    connect(ui->keyWidget, SIGNAL(selectionChanged(QStringList)), this, SLOT(newSelection(QStringList)));
}

KbBindWidget::~KbBindWidget(){
    delete ui;
}

void KbBindWidget::setBind(KbBind* newBind, KbProfile* newProfile){
    ui->keyWidget->clearSelection();
    ui->rbWidget->setBind(newBind, newProfile);
    newSelection(QStringList());
    if(bind == newBind)
        return;
    if(bind){
        disconnect(bind, SIGNAL(layoutChanged()), this, SLOT(newLayout()));
        disconnect(bind, SIGNAL(updated()), this, SLOT(updateBind()));
    }
    connect(newBind, SIGNAL(layoutChanged()), this, SLOT(newLayout()));
    connect(newBind, SIGNAL(updated()), this, SLOT(updateBind()));
    bind = newBind;
    profile = newProfile;
    newLayout();
    ui->rbWidget->setBind(bind, profile);
    updateBind();
}

void KbBindWidget::updateBind(){
    const KeyMap& map = bind->map();
    // Build the action list based on the keymap
    QHash<QString, QString> actions;
    uint count = map.count();
    for(uint i = 0; i < count; i++){
        QString key = map.key(i)->name;
        actions[key] = bind->action(key);
    }
    ui->keyWidget->bindMap(actions);
    ui->rbWidget->setSelection(currentSelection);
    updateSelDisplay();
}

void KbBindWidget::newLayout(){
    ui->keyWidget->map(bind->map());
    if(!currentSelection.isEmpty())
        ui->keyWidget->setSelection(currentSelection);
}

void KbBindWidget::newSelection(QStringList selection){
    currentSelection = selection;
    ui->rbWidget->setSelection(selection, true);
    updateSelDisplay();
}

void KbBindWidget::updateSelDisplay(){
    int count = currentSelection.count();
    if(count == 0){
        // No keys selected
        ui->selectLabel->setText("Click to select keys");
        return;
    }
    if(count == 1){
        // Single key selected: show key name and binding
        QString key = currentSelection[0];
        const KeyPos* pos = bind->map().key(key);
        if(!pos)
            ui->selectLabel->setText("(Unknown)");
        else
            ui->selectLabel->setText(pos->friendlyName(false).split("\n")[0] + " → " + bind->friendlyActionName(key).split("\n")[0]);
        return;
    }
    ui->selectLabel->setText(QString("%1 keys selected").arg(count));
}

void KbBindWidget::on_resetButton_clicked(){
    QStringList selection = currentSelection;
    const KeyMap& map = bind->map();
    if(selection.isEmpty())
        // Reset all keys if none selected
        selection = map.allKeys();
    uint count = selection.count();
    QString text;
    if(count == map.count())
        text = "<center>Reset all keys to default?</center>";
    else if(count == 1)
        text = "<center>Reset this key to default?</center>";
    else
        text = tr("<center>Reset %1 keys to default?</center>").arg(count);
    if(QMessageBox(QMessageBox::NoIcon, "Confirm action", text, QMessageBox::Yes | QMessageBox::No, this).exec() != QMessageBox::Yes)
        return;
    bind->resetAction(selection);
    updateBind();
}

void KbBindWidget::on_copyButton_clicked(){
    QStringList selection = currentSelection;
    const KeyMap& map = bind->map();
    int count = selection.count();
    QString text = tr("%1 key").arg(count) + (count == 1 ? "" : "s");
    if(count == 0){
        // Copy all keys if none selected
        selection = map.allKeys();
        text = "all keys";
    }
    text = "Copy binding for " + text + " to:";
    // Display popup
    ModeSelectDialog dialog(this, profile->currentMode, profile->modes, text);
    if(dialog.exec() != QDialog::Accepted)
        return;
    // Copy selected keys to selected modes
    QList<KbMode*> selectedModes = dialog.selection();
    foreach(KbMode* mode, selectedModes){
        KbBind* modeBind = mode->bind();
        foreach(const QString& key, selection){
            modeBind->keyAction(key, bind->action(key));
        }
    }
}
