#include "rebindwidget.h"
#include "ui_rebindwidget.h"

RebindWidget::RebindWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RebindWidget),
    bind(0), profile(0)
{
    ui->setupUi(this);
    ui->lightWrapBox->hide();
    ui->modeWrapBox->hide();
    ui->programKpExtra->hide();
    ui->programKrExtra->hide();

    // Populate key lists
    modKeys << "caps" << "lshift" << "rshift" << "lctrl" << "rctrl" << "lwin" << "rwin" << "lalt" << "ralt" << "rmenu";
    fnKeys << "esc" << "f1" << "f2" << "f3" << "f4" << "f5" << "f6" << "f7" << "f8" << "f9" << "f10" << "f11" << "f12" << "prtscn" << "scroll" << "pause" << "ins" << "del" << "home" << "end" << "pgup" << "pgdn" << "up" << "down" << "left" << "right";
    numKeys << "numlock" << "num0" << "num1" << "num2" << "num3" << "num4" << "num5" << "num6" << "num7" << "num8" << "num9" << "numslash" << "numstar" << "numminus" << "numplus" << "numdot" << "numenter";
    mediaKeys << "stop" << "prev" << "play" << "next" << "volup" << "voldn" << "mute";
#ifdef Q_OS_MACX
    // Replace some OSX keys with their actual meanings
    ui->modBox->setItemText(modKeys.indexOf("lwin") + 1, "Left Cmd");
    ui->modBox->setItemText(modKeys.indexOf("rwin") + 1, "Right Cmd");
    ui->modBox->setItemText(modKeys.indexOf("lalt") + 1, "Left Option");
    ui->modBox->setItemText(modKeys.indexOf("ralt") + 1, "Right Option");
    ui->fnBox->setItemText(fnKeys.indexOf("prtscn") + 1, "F13");
    ui->fnBox->setItemText(fnKeys.indexOf("scroll") + 1, "F14");
    ui->fnBox->setItemText(fnKeys.indexOf("pause") + 1, "F15");
    ui->fnBox->setItemText(fnKeys.indexOf("ins") + 1, "Help");
    ui->numBox->setItemText(numKeys.indexOf("numlock") + 1, "Clear");
#endif
}

RebindWidget::~RebindWidget(){
    delete ui;
}

void RebindWidget::setBind(KbBind* newBind, KbProfile* newProfile){
    bind = newBind;
    profile = newProfile;
    setSelection(QStringList());

    // Populate typing keys by position
    ui->typingBox->clear();
    ui->typingBox->addItem(" ");
    typingKeys.clear();
    const KeyMap& map = bind->map();
    uint count = map.count();
    for(uint i = 0; i < count; i++){
        const KeyPos* pos = map.key(i);
        QString name = pos->name, friendly = pos->friendlyName();
        if(KbBind::isNormal(KbBind::defaultAction(name)) && !modKeys.contains(name) && !fnKeys.contains(name) && !numKeys.contains(name) && !mediaKeys.contains(name) && name != "enter" && name != "tab" && name != "bspace"){
            ui->typingBox->addItem(friendly);
            typingKeys.append(name);
        }
    }
    typingKeys << "enter" << "tab" << "bspace";
    ui->typingBox->addItem("Enter");
    ui->typingBox->addItem("Tab");
    ui->typingBox->addItem("Backspace");

    // Populate mode list
    ui->modeBox->clear();
    ui->modeBox->addItem(" ");
    ui->modeBox->addItem("(Previous)");
    ui->modeBox->addItem("(Next)");
    int idx = 1;
    foreach(KbMode* mode, newProfile->modes)
        ui->modeBox->addItem(QString("%1: %2").arg(idx++).arg(mode->name()));
}

void RebindWidget::setSelection(const QStringList& newSelection, bool applyPrevious){
    // Apply changes to previous selection (if any)
    if(!selection.isEmpty() && applyPrevious){
        QStringList previous = selection;
        selection = QStringList();
        applyChanges(previous, false);
    }

    selection = newSelection;
    if(newSelection.isEmpty()){
        hide();
        return;
    } else
        show();

    bool hasAction = false;
    QString action;
    if(selection.count() == 1){
        // Select action from the UI
        action = bind->action(selection.first());
        if(!action.isEmpty())
            hasAction = true;
    } else {
        // Select an action from the UI if all keys are the same.
        // Select nothing if any are unbound or different
        foreach(const QString& key, selection){
            QString keyAction = bind->action(key);
            if(keyAction.isEmpty() || (hasAction && keyAction != action)){
                hasAction = false;
                break;
            } else if(action.isEmpty()){
                action = keyAction;
                hasAction = true;
            }
        }
    }

    if(!hasAction)
        action = "";
    // Standard key - tab 0
    if(KbBind::isNormal(action)){
        ui->tabWidget->setCurrentIndex(0);
        // Set normal keys (indexOf returns -1 if not found, index zero is blank)
        ui->typingBox->setCurrentIndex(typingKeys.indexOf(action) + 1);
        ui->modBox->setCurrentIndex(modKeys.indexOf(action) + 1);
        ui->fnBox->setCurrentIndex(fnKeys.indexOf(action) + 1);
        ui->numBox->setCurrentIndex(numKeys.indexOf(action) + 1);
        ui->mediaBox->setCurrentIndex(mediaKeys.indexOf(action) + 1);
    } else {
        ui->typingBox->setCurrentIndex(0);
        ui->modBox->setCurrentIndex(0);
        ui->fnBox->setCurrentIndex(0);
        ui->numBox->setCurrentIndex(0);
        ui->mediaBox->setCurrentIndex(0);
    }
    // Program key - tab 2
    ui->programKpBox->setText("");
    ui->programKrBox->setText("");
    if(KbBind::isProgram(action)){
        ui->tabWidget->setCurrentIndex(2);
        QString onPress, onRelease;
        int stop = KbBind::programInfo(action, onPress, onRelease);
        ui->programKpBox->setText(onPress);
        ui->programKrBox->setText(onRelease);
        switch(stop & 0xF){
        case KbBind::PROGRAM_PR_INDEF:
            ui->programKpIndefButton->setChecked(true);
            ui->programKpSKrButton->setChecked(false);
            ui->programKpSKpButton->setChecked(false);
            break;
        case KbBind::PROGRAM_PR_KRSTOP:
            ui->programKpIndefButton->setChecked(false);
            ui->programKpSKrButton->setChecked(true);
            ui->programKpSKpButton->setChecked(false);
            break;
        case KbBind::PROGRAM_PR_KPSTOP:
            ui->programKpIndefButton->setChecked(false);
            ui->programKpSKrButton->setChecked(false);
            ui->programKpSKpButton->setChecked(true);
            break;
        }
        switch(stop & 0xF0){
        case KbBind::PROGRAM_RE_INDEF:
            ui->programKrIndefButton->setChecked(true);
            ui->programKrSKpButton->setChecked(false);
            break;
        case KbBind::PROGRAM_RE_KPSTOP:
            ui->programKrIndefButton->setChecked(false);
            ui->programKrSKpButton->setChecked(true);
            break;
        }
    }
    // Other special keys - tab 1
    if(KbBind::isSpecial(action) && !KbBind::isProgram(action)){
        ui->tabWidget->setCurrentIndex(1);
        int param;
        QString sAction = KbBind::specialInfo(action, param);
        // Mode selection. Check wrap-around flag
        ui->modeWrapBox->setChecked(true);
        if(sAction == "mode"){
            ui->modeWrapBox->setChecked(true);
            if(param == KbBind::MODE_PREV_WRAP)
                param = KbBind::MODE_PREV;
            else if(param == KbBind::MODE_NEXT_WRAP)
                param = KbBind::MODE_NEXT;
            else if(param < 0)
                ui->modeWrapBox->setChecked(false);
            // Set mode box to current selection, or to mode 1 if invalid
            param += 3;
            if(param >= 0 && param < ui->modeBox->count())
                ui->modeBox->setCurrentIndex(param);
            else
                // 0 -> "", 1 -> Prev, 2 -> Next, 3 -> Mode 1
                ui->modeBox->setCurrentIndex(3);
        } else
            ui->modeBox->setCurrentIndex(0);
        // Brightness control. Also check wrap
        if(sAction == "light"){
            ui->lightWrapBox->setChecked(true);
            if(param == KbBind::LIGHT_DOWN_WRAP)
                param = KbBind::LIGHT_DOWN;
            else if(param == KbBind::LIGHT_UP_WRAP)
                param = KbBind::LIGHT_UP;
            else
                ui->lightWrapBox->setChecked(false);
            if(param < 0 || param > 1)
                param = -1;
            ui->lightBox->setCurrentIndex(param + 1);
        } else
            ui->lightBox->setCurrentIndex(0);
        // Win lock
        if(sAction == "lock"){
            if(param < 0 || param > 2)
                param = -1;
            ui->lockBox->setCurrentIndex(param + 1);
        } else
            ui->lockBox->setCurrentIndex(0);
    } else {
        ui->modeBox->setCurrentIndex(0);
        ui->lightBox->setCurrentIndex(0);
        ui->lockBox->setCurrentIndex(0);
    }
}

void RebindWidget::applyChanges(const QStringList& keys, bool doUnbind){
    if(ui->typingBox->currentIndex() > 0)
        bind->keyAction(keys, typingKeys[ui->typingBox->currentIndex() - 1]);
    else if(ui->modBox->currentIndex() > 0)
        bind->keyAction(keys, modKeys[ui->modBox->currentIndex() - 1]);
    else if(ui->fnBox->currentIndex() > 0)
        bind->keyAction(keys, fnKeys[ui->fnBox->currentIndex() - 1]);
    else if(ui->numBox->currentIndex() > 0)
        bind->keyAction(keys, numKeys[ui->numBox->currentIndex() - 1]);
    else if(ui->mediaBox->currentIndex() > 0)
        bind->keyAction(keys, mediaKeys[ui->mediaBox->currentIndex() - 1]);
    else if(ui->modeBox->currentIndex() > 0)
        bind->modeAction(keys, ui->modeBox->currentIndex() - 3 - (ui->modeWrapBox->isChecked() && ui->modeBox->currentIndex() < 3 ? 2 : 0));
    else if(ui->lightBox->currentIndex() > 0)
        bind->lightAction(keys, ui->lightBox->currentIndex() - 1 + (ui->lightWrapBox->isChecked() ? 2 : 0));
    else if(ui->lockBox->currentIndex() > 0)
        bind->lockAction(keys, ui->lockBox->currentIndex() - 1);
    else if(!ui->programKpBox->text().isEmpty() || !ui->programKrBox->text().isEmpty()){
        int kpStop = 0, krStop = 0;
        if(!ui->programKpBox->text().isEmpty()){
            if(ui->programKpIndefButton->isChecked())
                kpStop = KbBind::PROGRAM_PR_INDEF;
            else if(ui->programKpSKpButton->isChecked())
                kpStop = KbBind::PROGRAM_PR_KPSTOP;
            else if(ui->programKpSKrButton->isChecked())
                kpStop = KbBind::PROGRAM_PR_KRSTOP;
        }
        if(!ui->programKrBox->text().isEmpty()){
            if(ui->programKrIndefButton->isChecked())
                krStop = KbBind::PROGRAM_RE_INDEF;
            else if(ui->programKrSKpButton->isChecked())
                krStop = KbBind::PROGRAM_RE_KPSTOP;
        }
        bind->programAction(keys, ui->programKpBox->text(), ui->programKrBox->text(), kpStop | krStop);
    } else if(doUnbind)
        bind->noAction(keys);
}

void RebindWidget::on_applyButton_clicked(){
    applyChanges(selection, true);
}

void RebindWidget::on_cancelButton_clicked(){
    // Re-load selection
    setSelection(selection);
}

void RebindWidget::on_resetButton_clicked(){
    bind->resetAction(selection);
}

void RebindWidget::on_unbindButton_clicked(){
    bind->noAction(selection);
}

void RebindWidget::setBox(QWidget* box){
    // Un-select every item except for the current one.
    // on_*_currentIndexChanged will take care of deselecting the checkbox.
    if(box != ui->typingBox) ui->typingBox->setCurrentIndex(0);
    if(box != ui->modBox) ui->modBox->setCurrentIndex(0);
    if(box != ui->fnBox) ui->fnBox->setCurrentIndex(0);
    if(box != ui->numBox) ui->numBox->setCurrentIndex(0);
    if(box != ui->mediaBox) ui->mediaBox->setCurrentIndex(0);
    if(box != ui->modeBox) ui->modeBox->setCurrentIndex(0);
    if(box != ui->lightBox) ui->lightBox->setCurrentIndex(0);
    if(box != ui->lockBox) ui->lockBox->setCurrentIndex(0);
    if(box != ui->programKpBox && box != ui->programKrBox){
        ui->programKpButton->setChecked(false);
        ui->programKrButton->setChecked(false);
    }
}

void RebindWidget::on_typingBox_currentIndexChanged(int index){
    if(index == 0)
        ui->typingButton->setChecked(false);
    else {
        ui->typingButton->setChecked(true);
        setBox(ui->typingBox);
    }
}

void RebindWidget::on_modBox_currentIndexChanged(int index){
    if(index == 0)
        ui->modButton->setChecked(false);
    else {
        ui->modButton->setChecked(true);
        setBox(ui->modBox);
    }
}

void RebindWidget::on_fnBox_currentIndexChanged(int index){
    if(index == 0)
        ui->fnButton->setChecked(false);
    else {
        ui->fnButton->setChecked(true);
        setBox(ui->fnBox);
    }
}

void RebindWidget::on_numBox_currentIndexChanged(int index){
    if(index == 0)
        ui->numButton->setChecked(false);
    else {
        ui->numButton->setChecked(true);
        setBox(ui->numBox);
    }
}

void RebindWidget::on_mediaBox_currentIndexChanged(int index){
    if(index == 0)
        ui->mediaButton->setChecked(false);
    else {
        ui->mediaButton->setChecked(true);
        setBox(ui->mediaBox);
    }
}

void RebindWidget::on_modeBox_currentIndexChanged(int index){
    if(index == 1 || index == 2)
        ui->modeWrapBox->show();
    else
        ui->modeWrapBox->hide();
    if(index == 0)
        ui->modeButton->setChecked(false);
    else {
        ui->modeButton->setChecked(true);
        setBox(ui->modeBox);
    }
}

void RebindWidget::on_lightBox_currentIndexChanged(int index){
    if(index == 0){
        ui->lightButton->setChecked(false);
        ui->lightWrapBox->hide();
    } else {
        ui->lightWrapBox->show();
        ui->lightButton->setChecked(true);
        setBox(ui->lightBox);
    }
}

void RebindWidget::on_lockBox_currentIndexChanged(int index){
    if(index == 0)
        ui->lockButton->setChecked(false);
    else {
        ui->lockButton->setChecked(true);
        setBox(ui->lockBox);
    }
}

void RebindWidget::on_programKpBox_textChanged(const QString &arg1){
    if(arg1.isEmpty()){
        ui->programKpButton->setChecked(false);
        ui->programKpExtra->hide();
    } else {
        ui->programKpButton->setChecked(true);
        setBox(ui->programKpBox);
        ui->programKpExtra->show();
    }
}

void RebindWidget::on_programKrBox_textChanged(const QString &arg1){
    if(arg1.isEmpty()){
        ui->programKrButton->setChecked(false);
        ui->programKrExtra->hide();
    } else {
        ui->programKrButton->setChecked(true);
        setBox(ui->programKrBox);
        ui->programKrExtra->show();
    }
}

void RebindWidget::on_typingButton_clicked(bool checked){
    if(checked && ui->typingBox->currentIndex() == 0)
        ui->typingBox->setCurrentIndex(1);
}

void RebindWidget::on_modButton_clicked(bool checked){
    if(checked && ui->modBox->currentIndex() == 0)
        ui->modBox->setCurrentIndex(1);
}

void RebindWidget::on_fnButton_clicked(bool checked){
    if(checked && ui->fnBox->currentIndex() == 0)
        ui->fnBox->setCurrentIndex(1);
}

void RebindWidget::on_numButton_clicked(bool checked){
    if(checked && ui->numBox->currentIndex() == 0)
        ui->numBox->setCurrentIndex(1);
}

void RebindWidget::on_mediaButton_clicked(bool checked){
    if(checked && ui->mediaBox->currentIndex() == 0)
        ui->mediaBox->setCurrentIndex(1);
}

void RebindWidget::on_modeButton_clicked(bool checked){
    if(checked && ui->modeBox->currentIndex() == 0)
        ui->modeBox->setCurrentIndex(1);
}

void RebindWidget::on_lightButton_clicked(bool checked){
    if(checked && ui->lightBox->currentIndex() == 0)
        ui->lightBox->setCurrentIndex(1);
}

void RebindWidget::on_lockButton_clicked(bool checked){
    if(checked && ui->lockBox->currentIndex() == 0)
        ui->lockBox->setCurrentIndex(1);
}

void RebindWidget::on_programKpButton_clicked(bool checked){
    if(!checked){
        ui->programKpBox->setText("");
    } else if(ui->programKpBox->text().isEmpty()){
        ui->programKpBox->setFocus();
        setBox(ui->programKpBox);
    }
}

void RebindWidget::on_programKrButton_clicked(bool checked){
    if(!checked){
        ui->programKrBox->setText("");
    } else if(ui->programKrBox->text().isEmpty()){
        ui->programKrBox->setFocus();
        setBox(ui->programKrBox);
    }
}
