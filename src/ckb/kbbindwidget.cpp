#include <QMessageBox>
#include "rebindwidget.h"
#include "kbbindwidget.h"
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
    if(bind)
        disconnect(bind, SIGNAL(updated()), this, SLOT(updateBind()));
    connect(newBind, SIGNAL(updated()), this, SLOT(updateBind()));
    bind = newBind;
    profile = newProfile;
    updateBind();
}

void KbBindWidget::updateBind(){
    const KeyMap& map = bind->map();
    ui->keyWidget->map(map);
    // Build the action list based on the keymap
    QHash<QString, QString> actions;
    uint count = map.count();
    for(uint i = 0; i < count; i++){
        QString key = map.key(i)->name;
        actions[key] = bind->action(key);
    }
    ui->keyWidget->bindMap(actions);
    if(!currentSelection.isEmpty())
        ui->keyWidget->setSelection(currentSelection);
    ui->rbWidget->setBind(bind, profile);
    ui->rbWidget->setSelection(currentSelection);
}

void KbBindWidget::newSelection(QStringList selection){
    currentSelection = selection;
    ui->rbWidget->setSelection(selection);
    int count = selection.count();
    if(count == 0){
        // No keys selected
        ui->selectLabel->setText("Click to select keys");
        return;
    }
    if(count == 1){
        // Single key selected: show key name and binding
        QString key = selection[0];
        const KeyPos* pos = bind->map().key(key);
        if(!pos)
            ui->selectLabel->setText("(Unknown)");
        else
            ui->selectLabel->setText(pos->friendlyName(false).split("\n")[0] + " → " + bind->friendlyName(key).split("\n")[0]);
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
    foreach(const QString& key, selection)
        bind->resetAction(key);
    updateBind();
}

void KbBindWidget::on_unbindButton_clicked(){
    QStringList selection = currentSelection;
    const KeyMap& map = bind->map();
    if(selection.isEmpty())
        // Reset all keys if none selected
        selection = map.allKeys();
    uint count = selection.count();
    QString text;
    if(count == map.count())
        text = "<center>Unbind all keys?<br />(WARNING: You won't be able to use the keyboard)</center>";
    else if(count == 1)
        text = "<center>Unbind this key?</center>";
    else
        text = tr("<center>Unbind %1 keys?</center>").arg(count);
    if(QMessageBox(QMessageBox::NoIcon, "Confirm action", text, QMessageBox::Yes | QMessageBox::No, this).exec() != QMessageBox::Yes)
        return;
    foreach(const QString& key, selection)
        bind->noAction(key);
    updateBind();
}
