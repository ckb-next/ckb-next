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
    foreach(const QString& key, map.keys())
        actions[key] = bind->action(key);
    ui->keyWidget->bindMap(actions);
    ui->rbWidget->setSelection(currentSelection);
    updateSelDisplay();
}

void KbBindWidget::newLayout(){
    ui->keyWidget->map(bind->map());
    ui->rbWidget->setBind(bind, profile);
    updateSelDisplay();
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
        ui->selectLabel->setText("Click to select");
        return;
    }
    if(count == 1){
        // Single key selected: show key name and binding
        QString key = currentSelection[0];
        const Key& pos = bind->map()[key];
        if(!pos)
            ui->selectLabel->setText("(Unknown)");
        else
            ui->selectLabel->setText(pos.friendlyName(false).split("\n")[0] + " → " + bind->friendlyActionName(key).split("\n")[0]);
        return;
    }
    ui->selectLabel->setText(QString("%1 %2 selected").arg(count).arg(bind->isMouse() ? "buttons" : "keys"));
}

void KbBindWidget::on_resetButton_clicked(){
    QStringList selection = currentSelection;
    const KeyMap& map = bind->map();
    if(selection.isEmpty())
        // Reset all keys if none selected
        selection = map.keys();
    uint count = selection.count();
    QString type = bind->isMouse() ? "button" : "key";
    QString text;
    if(count == map.count())
        text = "<center>Reset all %1s to default?</center>";
    else if(count == 1)
        text = "<center>Reset this %1 to default?</center>";
    else
        text = tr("<center>Reset %1 %2s to default?</center>").arg(count);
    if(QMessageBox(QMessageBox::NoIcon, "Confirm action", text.arg(type), QMessageBox::Yes | QMessageBox::No, this).exec() != QMessageBox::Yes)
        return;
    bind->resetAction(selection);
    updateBind();
}

void KbBindWidget::on_copyButton_clicked(){
    QStringList selection = currentSelection;
    const KeyMap& map = bind->map();
    int count = selection.count();
    QString type = bind->isMouse() ? "button" : "key";
    QString text = tr("%1 %2").arg(count).arg(type) + (count == 1 ? "" : "s");
    if(count == 0){
        // Copy all keys if none selected
        selection = map.keys();
        text = tr("all %1s").arg(type);
    }
    text = "Copy binding for " + text + " to:";
    // Display popup
    ModeSelectDialog dialog(this, profile->currentMode(), profile->modes(), text);
    if(dialog.exec() != QDialog::Accepted)
        return;
    // Copy selected keys to selected modes
    QList<KbMode*> selectedModes = dialog.selection();
    foreach(KbMode* mode, selectedModes){
        KbBind* modeBind = mode->bind();
        foreach(const QString& key, selection){
            modeBind->setAction(key, bind->action(key));
        }
    }
}
