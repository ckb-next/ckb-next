#include <QStandardPaths>
#include "rebindwidget.h"
#include "ui_rebindwidget.h"
#include <qdebug.h>     // lae.

static const int DPI_OFFSET = -KeyAction::DPI_CYCLE_UP + 1;
static const int DPI_CUST_IDX = KeyAction::DPI_CUSTOM + DPI_OFFSET;

RebindWidget::RebindWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RebindWidget),
    bind(0), profile(0), macReader(0)
{
    ui->setupUi(this);
    ui->lightWrapBox->hide();
    ui->modeWrapBox->hide();
    ui->programKpExtra->hide();
    ui->programKrExtra->hide();

    // Populate key lists
    modKeys << "caps" << "lshift" << "rshift" << "lctrl" << "rctrl" << "lwin" << "rwin" << "lalt" << "ralt" << "rmenu" << "fn";
    fnKeys << "esc" << "f1" << "f2" << "f3" << "f4" << "f5" << "f6" << "f7" << "f8" << "f9" << "f10" << "f11" << "f12" << "prtscn" << "scroll" << "pause" << "f13" << "f14" << "f15" << "f16" << "f17" << "f18" << "f19" << "f20" << "ins" << "del" << "home" << "end" << "pgup" << "pgdn" << "power" << "lightup" << "lightdn" << "up" << "down" << "left" << "right";
    numKeys << "numlock" << "num0" << "num1" << "num2" << "num3" << "num4" << "num5" << "num6" << "num7" << "num8" << "num9" << "numslash" << "numstar" << "numminus" << "numplus" << "numdot" << "numenter";
    mediaKeys << "stop" << "prev" << "play" << "next" << "volup" << "voldn" << "mute" << "eject";
    mouseKeys << "mouse1" << "mouse2" << "mouse3";
    mouseExtKeys << "mouse4" << "mouse5" << "mouse6" << "mouse7" << "mouse8";
    wheelKeys << "wheelup" << "wheeldn";
    // Populate boxes
    foreach(const QString& key, modKeys)
        ui->modBox->addItem(KeyMap::friendlyName(key));
    foreach(const QString& key, fnKeys)
        ui->fnBox->addItem(KeyMap::friendlyName(key).replace("\n", " / "));
    foreach(const QString& key, numKeys)
        ui->numBox->addItem(KeyMap::friendlyName(key).remove("NumPad "));
    foreach(const QString& key, mediaKeys)
        ui->mediaBox->addItem(KeyMap::friendlyName(key));
#ifdef Q_OS_MACX
    // Replace some OSX keys with their actual meanings
    ui->modBox->setItemText(modKeys.indexOf("lwin") + 1, "Left Cmd");
    ui->modBox->setItemText(modKeys.indexOf("rwin") + 1, "Right Cmd");
    ui->modBox->setItemText(modKeys.indexOf("lalt") + 1, "Left Option");
    ui->modBox->setItemText(modKeys.indexOf("ralt") + 1, "Right Option");
    ui->fnBox->setItemText(fnKeys.indexOf("prtscn") + 1, "F13");
    ui->fnBox->setItemText(fnKeys.indexOf("scroll") + 1, "F14");
    ui->fnBox->setItemText(fnKeys.indexOf("pause") + 1, "F15");
    ui->fnBox->setItemText(fnKeys.indexOf("f13") + 1, "F13 (Windows/Linux)");
    ui->fnBox->setItemText(fnKeys.indexOf("f14") + 1, "F14 (Windows/Linux)");
    ui->fnBox->setItemText(fnKeys.indexOf("f15") + 1, "F15 (Windows/Linux)");
    ui->fnBox->setItemText(fnKeys.indexOf("ins") + 1, "Help");
    ui->numBox->setItemText(numKeys.indexOf("numlock") + 1, "Clear");

    // Add tip label
    ui->progTipLabel->setText("<p style=\"line-height:150%\">Tip: use the <font face=\"monospace\">open</font> command to launch a file, directory, or app. For instance, to start Safari:<br /><font face=\"monospace\">&nbsp;&nbsp;open /Applications/Safari.app</font></p>");
#else
    ui->progTipLabel->setText("<p style=\"line-height:150%\">Tip: use <font face=\"monospace\">xdg-open</font> to launch a file or directory. For instance, to open your home folder:<br /><font face=\"monospace\">&nbsp;&nbsp;xdg-open " + QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "</font></p>");
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
    // Use the K95 map as it has all keys
    const KeyMap& map = KeyMap(KeyMap::K95, bind->map().layout());
    foreach(const QString& name, map.byPosition()){
        KeyAction action(bind->defaultAction(name));
        if(action.isNormal() && !modKeys.contains(name) && !fnKeys.contains(name) && !numKeys.contains(name) && !mediaKeys.contains(name) && name != "enter" && name != "tab" && name != "bspace"){
            const Key& pos = map[name];
            QString friendly = pos.friendlyName();
            ui->typingBox->addItem(friendly);
            typingKeys.append(name);
        }
    }
    typingKeys << "enter" << "tab" << "bspace";
    ui->typingBox->addItem("Enter");
    ui->typingBox->addItem("Tab");
    ui->typingBox->addItem("Backspace");
    if(!map.isISO()){
        // Add ISO backslash (aka KEY_102ND) to ANSI options
        typingKeys << "bslash_iso";
        ui->typingBox->addItem("Backslash (ISO layout)");
    }

    // Populate mode list
    ui->modeBox->clear();
    ui->modeBox->addItem(" ");
    ui->modeBox->addItem("(Previous)");
    ui->modeBox->addItem("(Next)");
    int idx = 1;
    foreach(KbMode* mode, newProfile->modes())
        ui->modeBox->addItem(QString("%1: %2").arg(idx++).arg(mode->name()));

    // Enable/disable DPI based on device
    if(bind->isMouse()){
        ui->dpiButton->setEnabled(true);
        ui->dpiBox->setEnabled(true);
        ui->dpiWarning->hide();
        // Fill DPI slots
        const KbPerf* perf = bind->perf();
        for(int i = 0; i < KbPerf::DPI_COUNT; i++){
            bool sniper = (i == 0);
            int boxIdx = i + DPI_OFFSET;
            QPoint dpi = perf->dpi(i);
            QString text = tr(sniper ? "Sniper:\t%1 x %2" : "%3:\t%1 x %2").arg(dpi.x()).arg(dpi.y());
            if(!sniper) text = text.arg(i);
            ui->dpiBox->setItemText(boxIdx, text);
        }
    } else {
        ui->dpiButton->setEnabled(false);
        ui->dpiBox->setEnabled(false);
        ui->dpiWarning->show();
    }
    // Always disable custom DPI boxes until selected
    ui->dpiCustXBox->setEnabled(false);
    ui->dpiCustYBox->setEnabled(false);
    ui->dpiCustLabel->setEnabled(false);
}

void RebindWidget::setSelection(const QStringList& newSelection, bool applyPrevious){
    if(bind){
        // Populate animation list
        // FIXME: There should be a trigger to update this whenever an animation is added/removed, even if the key selection hasn't changed
        ui->animBox->clear();
        ui->animBox->addItem(" ");
        foreach(KbAnim* anim, bind->light()->animList())
            ui->animBox->addItem(anim->name());
    }

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
    KeyAction act(action);
    // Clear everything
    setBox(0);
    ui->dpiCustXBox->setValue(400);
    ui->dpiCustYBox->setValue(400);
    ui->animOnceBox->setChecked(false);
    ui->animKrBox->setChecked(false);
    ui->programKpBox->setText("");
    ui->programKrBox->setText("");
    ui->programKpSIBox->setChecked(false);
    ui->programKrSIBox->setChecked(false);
    ui->programKpModeBox->setCurrentIndex(0);
    ui->programKrModeBox->setCurrentIndex(0);
    ui->programKpModeBox->setEnabled(false);
    ui->programKrModeBox->setEnabled(false);
    // Clear neu UI elements in MacroTab
    ui->pteMacroBox->setPlainText("");
    ui->pteMacroText->setPlainText("");
    ui->pteMacroComment->setPlainText("");
    ui->txtBuffer->setText("");
    // Fill in field and select tab according to action type
    bool mouse = act.isMouse();
    if(mouse){
        // Mouse buttons
        ui->tabWidget->setCurrentIndex(TAB_MOUSE);
        // Set mouse buttons (indexOf returns -1 if not found, index zero is blank)
        ui->mbBox->setCurrentIndex(mouseKeys.indexOf(action) + 1);
        ui->mb2Box->setCurrentIndex(mouseExtKeys.indexOf(action) + 1);
        ui->wheelBox->setCurrentIndex(wheelKeys.indexOf(action) + 1);
        if(act.isDPI()){
            QPoint custom;
            int value = act.dpiInfo(custom);
            if(value <= -DPI_OFFSET || value > KeyAction::DPI_CUSTOM)
                return;
            ui->dpiBox->setCurrentIndex(value + DPI_OFFSET);
            if(value == KeyAction::DPI_CUSTOM){
                ui->dpiCustXBox->setValue(custom.x());
                ui->dpiCustYBox->setValue(custom.y());
            }
        }
    } else if(act.isNormal()){
        // Standard key
        ui->tabWidget->setCurrentIndex(TAB_KB);
        ui->typingBox->setCurrentIndex(typingKeys.indexOf(action) + 1);
        ui->modBox->setCurrentIndex(modKeys.indexOf(action) + 1);
        ui->fnBox->setCurrentIndex(fnKeys.indexOf(action) + 1);
        ui->numBox->setCurrentIndex(numKeys.indexOf(action) + 1);
        ui->mediaBox->setCurrentIndex(mediaKeys.indexOf(action) + 1);
    } else if(act.isProgram()){
        // Program key
        ui->tabWidget->setCurrentIndex(TAB_PROGRAM);
        QString onPress, onRelease;
        int stop = act.programInfo(onPress, onRelease);
        ui->programKpBox->setText(onPress);
        ui->programKrBox->setText(onRelease);
        switch(stop & 0x0F){
        case KeyAction::PROGRAM_PR_MULTI:
            ui->programKpSIBox->setChecked(false);
            ui->programKpModeBox->setCurrentIndex(0);
            ui->programKpModeBox->setEnabled(false);
            break;
        default:
            ui->programKpSIBox->setChecked(true);
            ui->programKpModeBox->setCurrentIndex(stop & 0x0F);
            ui->programKpModeBox->setEnabled(true);
            break;
        }
        switch(stop & 0xF0){
        case KeyAction::PROGRAM_RE_MULTI:
            ui->programKrSIBox->setChecked(false);
            ui->programKrModeBox->setCurrentIndex(0);
            ui->programKrModeBox->setEnabled(false);
            break;
        case KeyAction::PROGRAM_RE_INDEF:
            ui->programKrSIBox->setChecked(true);
            ui->programKrModeBox->setCurrentIndex(0);
            ui->programKrModeBox->setEnabled(true);
            break;
        case KeyAction::PROGRAM_RE_KPSTOP:
            ui->programKrSIBox->setChecked(true);
            ui->programKrModeBox->setCurrentIndex(1);
            ui->programKrModeBox->setEnabled(true);
            break;
        }
    } else if(act.isAnim()){
        // Animation key
        ui->tabWidget->setCurrentIndex(TAB_ANIM);
        bool onlyOnce = false, stopOnRelease = false;
        QUuid id = act.animInfo(onlyOnce, stopOnRelease);
        // Find this animation in the list. If not found, -1 will be returned, resulting in the blank space being selected
        int index = bind->light()->findAnimIdx(id);
        ui->animBox->setCurrentIndex(index + 1);
        ui->animOnceBox->setChecked(!onlyOnce);
        ui->animKrBox->setChecked(stopOnRelease);
    } else if(act.isSpecial()){
        // Other special keys
        ui->tabWidget->setCurrentIndex(TAB_SPECIAL);
        int param;
        QString sAction = act.specialInfo(param);
        // Mode selection. Check wrap-around flag
        ui->modeWrapBox->setChecked(true);
        if(sAction == "mode"){
            ui->modeWrapBox->setChecked(true);
            if(param == KeyAction::MODE_PREV_WRAP)
                param = KeyAction::MODE_PREV;
            else if(param == KeyAction::MODE_NEXT_WRAP)
                param = KeyAction::MODE_NEXT;
            else if(param < 0)
                ui->modeWrapBox->setChecked(false);
            // Set mode box to current selection, or to mode 1 if invalid
            param += 3;
            if(param >= 0 && param < ui->modeBox->count())
                ui->modeBox->setCurrentIndex(param);
            else
                // 0 -> "", 1 -> Prev, 2 -> Next, 3 -> Mode 1
                ui->modeBox->setCurrentIndex(3);
        } else if (sAction == "macro") {
            ui->tabWidget->setCurrentIndex(TAB_MACRO);
            if (act.isValidMacro()) {
                ui->pteMacroBox->setPlainText(act.macroContent());
                ui->pteMacroText->setPlainText(act.macroLine()[1].replace("&das_IST_31N_col0n;", ":"));
                ui->pteMacroComment->setPlainText(act.macroLine()[2].replace("&das_IST_31N_col0n;", ":"));
                // Set the invisible Buffer to the original timing information.
                // For convenience / Migration from older versions:
                // If the timing information is only "x", then ignore it by setting it to an empty QString.
                ui->txtBuffer->setText("");
                if (act.macroTiming() != "x") ui->txtBuffer->setText(act.macroTiming());
                setCorrectRadioButton(act.macroContent());
            } else {
                qDebug("RebindWidget::setSelection found invalid macro definition.");
                act.macroDisplay();
            }
        } else
            ui->modeBox->setCurrentIndex(0);
        // Brightness control. Also check wrap
        if(sAction == "light"){
            ui->lightWrapBox->setChecked(true);
            if(param == KeyAction::LIGHT_DOWN_WRAP)
                param = KeyAction::LIGHT_DOWN;
            else if(param == KeyAction::LIGHT_UP_WRAP)
                param = KeyAction::LIGHT_UP;
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
    }
}

void RebindWidget::applyChanges(const QStringList& keys, bool doUnbind){
    if(ui->typingBox->currentIndex() > 0)
        bind->setAction(keys, typingKeys[ui->typingBox->currentIndex() - 1]);
    else if(ui->modBox->currentIndex() > 0)
        bind->setAction(keys, modKeys[ui->modBox->currentIndex() - 1]);
    else if(ui->fnBox->currentIndex() > 0)
        bind->setAction(keys, fnKeys[ui->fnBox->currentIndex() - 1]);
    else if(ui->numBox->currentIndex() > 0)
        bind->setAction(keys, numKeys[ui->numBox->currentIndex() - 1]);
    else if(ui->mediaBox->currentIndex() > 0)
        bind->setAction(keys, mediaKeys[ui->mediaBox->currentIndex() - 1]);
    else if(ui->mbBox->currentIndex() > 0)
        bind->setAction(keys, mouseKeys[ui->mbBox->currentIndex() - 1]);
    else if(ui->mb2Box->currentIndex() > 0)
        bind->setAction(keys, mouseExtKeys[ui->mb2Box->currentIndex() - 1]);
    else if(ui->wheelBox->currentIndex() > 0)
        bind->setAction(keys, wheelKeys[ui->wheelBox->currentIndex() - 1]);
    else if(ui->dpiBox->currentIndex() > 0)
        bind->setAction(keys, KeyAction::dpiAction(ui->dpiBox->currentIndex() - DPI_OFFSET, ui->dpiCustXBox->value(), ui->dpiCustYBox->value()));
    else if(ui->modeBox->currentIndex() > 0)
        bind->setAction(keys, KeyAction::modeAction(ui->modeBox->currentIndex() - 3 - (ui->modeWrapBox->isChecked() && ui->modeBox->currentIndex() < 3 ? 2 : 0)));
    else if(ui->lightBox->currentIndex() > 0)
        bind->setAction(keys, KeyAction::lightAction(ui->lightBox->currentIndex() - 1 + (ui->lightWrapBox->isChecked() ? 2 : 0)));
    else if(ui->lockBox->currentIndex() > 0)
        bind->setAction(keys, KeyAction::lockAction(ui->lockBox->currentIndex() - 1));
    else if(ui->animBox->currentIndex() > 0){
        KbAnim* anim = bind->light()->animList().at(ui->animBox->currentIndex() - 1);
        bind->setAction(keys, KeyAction::animAction(anim->guid(), !ui->animOnceBox->isChecked(), ui->animKrBox->isChecked()));
    } else if(!ui->programKpBox->text().isEmpty() || !ui->programKrBox->text().isEmpty()){
        int kpStop = 0, krStop = 0;
        if(!ui->programKpBox->text().isEmpty()){
            if(!ui->programKpSIBox->isChecked())
                kpStop = KeyAction::PROGRAM_PR_MULTI;
            else
                kpStop = ui->programKpModeBox->currentIndex();
        }
        if(!ui->programKrBox->text().isEmpty()){
            if(!ui->programKrSIBox->isChecked())
                krStop = KeyAction::PROGRAM_RE_MULTI;
            else if(ui->programKrModeBox->currentIndex() == 0)
                krStop = KeyAction::PROGRAM_RE_INDEF;
            else
                krStop = KeyAction::PROGRAM_RE_KPSTOP;
        }
        bind->setAction(keys, KeyAction::programAction(ui->programKpBox->text(), ui->programKrBox->text(), kpStop | krStop));
    } else if (ui->pteMacroBox->toPlainText().length() > 0) {
        /// G-key macro handling:
        /// Set the macro definiton for all keys selected (indeed, it may be multiple keys).
        /// First, concat the Macro Key Definion and the Macro plain text
        /// after escaping possible colos in the parts for Macro Text and Macro Comment.

        /// But first, there is a special condition to handle:
        /// You have recorded a macro with timing infos.
        /// Afterwards you changed manually the timing infos in the pteMacroBox and press Apply.
        /// In that case we must overwrite the txtBuffer to remember your changes.
        if (ui->rb_delay_asTyped->isChecked()) ui->txtBuffer->setText(ui->pteMacroBox->toPlainText());

        /// \todo There is still a bug in the state machine:
        /// If you record a macro in asTyped-mode, switch to another mode
        /// and change the vontent of the pteMacroBox manually,
        /// then the changes are not saved in the timing buffer.
        /// But anyhow, let's do more relevant things...
        QString mac;
        mac = ui->txtBuffer->text();
        mac = ui->pteMacroComment->toPlainText().replace(":", "&das_IST_31N_col0n;") + ":" + mac;
        mac = ui->pteMacroText->toPlainText().replace(":", "&das_IST_31N_col0n;") + ":" + mac;
        mac = ui->pteMacroBox->toPlainText() + ":" + mac;
        bind->setAction(keys, KeyAction::macroAction(mac));
    } else if(doUnbind)
        bind->noAction(keys);
}

void RebindWidget::on_applyButton_clicked(){
    // Normally, this should be done via signalling.
    // Because there is no serarate thread, we have to call it directly
    // (otherwise we could do Key char conversion step by step,
    // but so it is more easy to change the key definition):
    on_btnStopMacro_clicked();
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
    // Key
    if(box != ui->typingBox) ui->typingBox->setCurrentIndex(0);
    if(box != ui->modBox) ui->modBox->setCurrentIndex(0);
    if(box != ui->fnBox) ui->fnBox->setCurrentIndex(0);
    if(box != ui->numBox) ui->numBox->setCurrentIndex(0);
    if(box != ui->mediaBox) ui->mediaBox->setCurrentIndex(0);
    // Mouse
    if(box != ui->mbBox) ui->mbBox->setCurrentIndex(0);
    if(box != ui->mb2Box) ui->mb2Box->setCurrentIndex(0);
    if(box != ui->wheelBox) ui->wheelBox->setCurrentIndex(0);
    if(box != ui->dpiBox) ui->dpiBox->setCurrentIndex(0);
    // Anim
    if(box != ui->animBox) ui->animBox->setCurrentIndex(0);
    // Special
    if(box != ui->modeBox) ui->modeBox->setCurrentIndex(0);
    if(box != ui->lightBox) ui->lightBox->setCurrentIndex(0);
    if(box != ui->lockBox) ui->lockBox->setCurrentIndex(0);
    // Program
    if(box != ui->programKpBox && box != ui->programKrBox){
        ui->programKpButton->setChecked(false);
        ui->programKrButton->setChecked(false);
    }
    // Clear macro panel
    if (box != ui->pteMacroBox) {
        ui->pteMacroBox->setPlainText("");
        ui->txtBuffer->setText("");
        helpStatus(1);
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

void RebindWidget::on_pteMacroBox_textChanged() {
    if (ui->pteMacroBox->toPlainText().length() > 0) {
        setBox(ui->pteMacroBox);
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

void RebindWidget::on_mbBox_currentIndexChanged(int index){
    if(index == 0)
        ui->mbButton->setChecked(false);
    else {
        ui->mbButton->setChecked(true);
        setBox(ui->mbBox);
    }
}

void RebindWidget::on_mb2Box_currentIndexChanged(int index){
    if(index == 0)
        ui->mb2Button->setChecked(false);
    else {
        ui->mb2Button->setChecked(true);
        setBox(ui->mb2Box);
    }
}

void RebindWidget::on_wheelBox_currentIndexChanged(int index){
    if(index == 0)
        ui->wheelButton->setChecked(false);
    else {
        ui->wheelButton->setChecked(true);
        setBox(ui->wheelBox);
    }
}

void RebindWidget::on_dpiBox_currentIndexChanged(int index){
    if(index != DPI_CUST_IDX){
        // Clear custom DPI
        ui->dpiCustXBox->setEnabled(false);
        ui->dpiCustYBox->setEnabled(false);
        ui->dpiCustLabel->setEnabled(false);
    } else {
        ui->dpiCustXBox->setEnabled(true);
        ui->dpiCustYBox->setEnabled(true);
        ui->dpiCustLabel->setEnabled(true);
    }
    if(index == 0)
        ui->dpiButton->setChecked(false);
    else {
        ui->dpiButton->setChecked(true);
        setBox(ui->dpiBox);
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

void RebindWidget::on_animBox_currentIndexChanged(int index){
    if(index == 0)
        ui->animButton->setChecked(false);
    else {
        ui->animButton->setChecked(true);
        setBox(ui->animBox);
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

void RebindWidget::on_mbButton_clicked(bool checked){
    if(checked && ui->mbBox->currentIndex() == 0)
        ui->mbBox->setCurrentIndex(1);
}

void RebindWidget::on_mb2Button_clicked(bool checked){
    if(checked && ui->mb2Box->currentIndex() == 0)
        ui->mb2Box->setCurrentIndex(1);
}

void RebindWidget::on_wheelButton_clicked(bool checked){
    if(checked && ui->wheelBox->currentIndex() == 0)
        ui->wheelBox->setCurrentIndex(1);
}

void RebindWidget::on_dpiButton_clicked(bool checked){
    if(checked && ui->dpiBox->currentIndex() == 0)
        ui->dpiBox->setCurrentIndex(1);
}

void RebindWidget::on_programKpSIBox_clicked(bool checked){
    ui->programKpModeBox->setCurrentIndex(0);
    ui->programKpModeBox->setEnabled(checked);
}

void RebindWidget::on_programKrSIBox_clicked(bool checked){
    ui->programKrModeBox->setCurrentIndex(0);
    ui->programKrModeBox->setEnabled(checked);
}

void RebindWidget::on_animButton_clicked(bool checked){
    if(checked && ui->animBox->currentIndex() == 0)
        ui->animBox->setCurrentIndex(1);
}

//////////
/// \brief RebindWidget::on_btnStartMacro_clicked starts macro recording.
/// A new notification channel and MacroReader are created to do the job.
///
/// The UI is protected against false clicking
/// (e.g. if you type start and than Apply, the channel is closed in wrong order).
///
/// At this time, all neccessary params like macroNumber, macroPath, cmdFile etc. had been cached.
///
void RebindWidget::on_btnStartMacro_clicked() {
    if (!macReader) {
        bind->handleNotificationChannel(true);
        macReader = new MacroReader(bind->getMacroNumber(), bind->getMacroPath(), ui->pteMacroBox, ui->pteMacroText);
        // because of the second thread we need to disable three of the four bottom buttons.
        // Clicking "Stop" will enable them again.
        ui->applyButton->setEnabled(false);
        ui->resetButton->setEnabled(false);
        ui->unbindButton->setEnabled(false);
        ui->btnStartMacro->setEnabled(false);
        ui->btnStopMacro->setEnabled(true);
        ui->rb_delay_asTyped->setEnabled(false);
        ui->rb_delay_no->setEnabled(false);
        ui->rb_delay_default->setEnabled(false);
        helpStatus(2);
    }
}

//////////
/// \brief RebindWidget::on_btnStopMacro_clicked ends the macro recording.
/// Notify channel ist closed, the ReaderThread is deleted when the notification is really down.
///
/// Afterwards, the characters in the MacroBox are changed from KB-out format to cmd-in format.
/// At last the UI changes to the new state.
///
void RebindWidget::on_btnStopMacro_clicked() {
    if (macReader) {
        bind->handleNotificationChannel(false);
        delete macReader;
        macReader = 0;
        convertMacroBox();
        ui->applyButton->setEnabled(true);
        ui->resetButton->setEnabled(true);
        ui->unbindButton->setEnabled(true);
        ui->btnStartMacro->setEnabled(true);
        ui->btnStopMacro->setEnabled(false);
        ui->rb_delay_asTyped->setEnabled(true);
        ui->rb_delay_no->setEnabled(true);
        ui->rb_delay_default->setEnabled(true);
        helpStatus(3);
    }
}

//////////
/// \brief RebindWidget::on_btnClearMacro_clicked changes the help info an the panel.
/// The job of clearing the input panels is triggerd with signal/slot via the RebindWidget.ui file.
///
/// \todo I do not know what is the better solution with the delay-buttons in case of clicking clear:
/// Reset the button to the default value or do not touch it? Not clear is ignored.
///
void RebindWidget::on_btnClearMacro_clicked() {
    helpStatus(1);
}

//////////
/// \brief RebindWidget::helpStatus shows a help line in the ui.
/// \param status determines what to display.
///
void RebindWidget::helpStatus(int status) {
    switch (status) {
    case 1:
        ui->lbl_macro->setText("Type in a macro name in the comment box and click start.");
        break;
    case 2:
        ui->lbl_macro->setText("Type your macro and click stop when finished.");
        break;
    case 3:
        ui->lbl_macro->setText("Click Apply or change values in Macro Key Actions in advance.");
        break;
    default:
        ui->lbl_macro->setText(QString("Oops: Some magic in RebindWidget::helpStatus (%1)").arg(status));
    }
}

//////////
/// \brief RebindWidget::convertMacroBox converts the macroBox content.
/// The KB sends each keypress as "key [+|-]<keyname><newline>"
/// This is followed by timing information (delays between keystrokes).
///
/// The ckb-daemon needs a shorter format, only " [+|-]<keyname>=<delay>",
/// multiple entries are separated by comma.
///
/// That function does the conversion.
///
void RebindWidget::convertMacroBox() {
    QString in;

    // Remember the original input stream before it is converted.
    // In case of new choice of delay mode we have to restore it.
    if (ui->txtBuffer->text() == "") {
        ui->txtBuffer->setText(ui->pteMacroBox->toPlainText());
        in = ui->pteMacroBox->toPlainText();
    } else in = ui->txtBuffer->text();

    in.replace (QRegExp("\n"), ",");    // first join all in one line
    in.replace (QRegExp("key "), "");   // then delete keyword "key" followed by space
    in.replace (QRegExp(",="), "=");    // at last join each keystroke with its delay parameter

    // How to deal with the delay params?
    // Because the three radio buttons are mututally exclusive,
    // we can run through the if-chain w/o conflicts.
    // If rb_delay_asTyped is checked, do nothing, because that's the standard.

    if (ui->rb_delay_default->isChecked()) {
        in.replace(QRegExp("=\\d+,"), ",");  // Delete the timing infos, use default value
        in.replace(QRegExp("=\\d+$"), "");   // The last entry is without comma
    }
    if (ui->rb_delay_no->isChecked()) {
        in.replace(QRegExp("=\\d+,"), "=0,");  // Set timing infos to zero for no delay
        in.replace(QRegExp("=\\d+$"), "=0");   // Again the last entry w/o comma
        in.replace(QRegExp("([\\+\\-]\\w+),"), "\\1=0,");  // If no delay is given, force it to zero
        in.replace(QRegExp("([\\+\\-]\\w+)$"), "\\1=0");
    }

    // Show the new format by replacing the older one.
    ui->pteMacroBox->setPlainText(in);
}

//////////
/// \brief RebindWidget::on_rb_delay_no_toggled
/// \param checked
/// The following slots are triggerd by changing the mutual exclusive radio buttons
/// when choosing the delay.
/// They are called, if the button ist enabled.
/// This first one should disable all delay.
///
void RebindWidget::on_rb_delay_no_toggled(bool checked)
{
    convertMacroBox();
}

//////////
/// \brief RebindWidget::on_rb_delay_asTyped_toggled
/// \param checked
/// This button ist clicked to use the delay times, as they are recorded.
/// Returs a warning message, if we are not in the recording phase,
/// because then we don't have the delay times any more.
///
void RebindWidget::on_rb_delay_asTyped_toggled(bool checked)
{
    convertMacroBox();
}

//////////
/// \brief RebindWidget::on_rb_delay_default_toggled
/// \param checked
/// This is as easy as the no-delay-button, because this means
/// take the default values.
///
void RebindWidget::on_rb_delay_default_toggled(bool checked)
{
    convertMacroBox();
}

//////////
/// \brief RebindWidget::setCorrectRadioButton
/// \param macdef
/// Set the radiobutton for timing paramters according to
/// the context.
/// If no "=" followed by a number and comma can be found, it is the default button.
/// If "=" can be found and numbers with more than one digit (means: > 9), it is the "asTyped" button
/// Otherwise it is the "no" button.
///
void RebindWidget::setCorrectRadioButton (QString macdef) {
    if (!macdef.contains(QRegExp("=\\d+,"))) {
        ui->rb_delay_default->setChecked(true);
        return;
    }
    if (macdef.contains(QRegExp("=\\d\\d+,"))) {
        ui->rb_delay_asTyped->setChecked(true);
        return;
    }
    ui->rb_delay_no->setChecked(true);
}
