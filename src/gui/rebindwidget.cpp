#include <QStandardPaths>
#include "rebindwidget.h"
#include "ui_rebindwidget.h"
#include <QDebug>
#include <QMessageBox>
#include "kbmanager.h"
#include "macrostringeditdialog.h"
#include "mainwindow.h"
#include <QScrollBar>

static const int DPI_OFFSET = -KeyAction::DPI_CYCLE_UP + 1;
static const int DPI_CUST_IDX = KeyAction::DPI_CUSTOM + DPI_OFFSET;

static inline void setLocalUiElementsEnabled(Ui::RebindWidget* const ui, const bool e){
    ui->editAsStringBtn->setEnabled(e);
    ui->btnAddEvent->setEnabled(e);
    ui->btnRemoveEvent->setEnabled(e);
    ui->btnClearMacro->setEnabled(e);
}

static inline void setUiElementsEnabled(Ui::RebindWidget* const ui, const bool e){
    ui->applyButton->setEnabled(e);
    ui->resetButton->setEnabled(e);
    ui->unbindButton->setEnabled(e);
    ui->rb_delay_asTyped->setEnabled(e);
    ui->rb_delay_default->setEnabled(e);
    ui->captureTypeBox->setEnabled(e);
    MainWindow::mainWindow->setTabsEnabled(e);
    ui->tabWidget->tabBar()->setEnabled(e);
    ui->cancelButton->setEnabled(e);
    setLocalUiElementsEnabled(ui, e);
}

RebindWidget::RebindWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RebindWidget),
    bind(nullptr), profile(nullptr), macroReader(nullptr), leftMouseClicked(false),
    macroDelegate(new MacroDropdownDelegate)
{
    ui->setupUi(this);
    ui->lightWrapBox->hide();
    ui->modeWrapBox->hide();

    // Populate key lists
    modKeys << "caps" << "lshift" << "rshift" << "lctrl" << "rctrl" << "lwin" << "rwin" << "lalt" << "ralt" << "rmenu" << "fn";
    fnKeys << "esc" << "f1" << "f2" << "f3" << "f4" << "f5" << "f6" << "f7" << "f8" << "f9" << "f10" << "f11" << "f12" << "prtscn" << "scroll" << "pause" << "f13" << "f14" << "f15" << "f16" << "f17" << "f18" << "f19" << "f20";
    numKeys << "numlock" << "num0" << "num1" << "num2" << "num3" << "num4" << "num5" << "num6" << "num7" << "num8" << "num9" << "numslash" << "numstar" << "numminus" << "numplus" << "numdot" << "numenter";
    mediaKeys << "stop" << "prev" << "play" << "next" << "volup" << "voldn" << "mute" << "eject";
    mouseKeys << "mouse1" << "mouse2" << "mouse3";
    mouseExtKeys << "mouse4" << "mouse5" << "mouse6" << "mouse7" << "mouse8";
    wheelKeys << "wheelup" << "wheeldn" << "wheellf" << "wheelrg";
#ifndef Q_OS_MACOS
    fnKeys << "f21" << "f22" << "f23" << "f24";
#endif
    fnKeys << "ins" << "del" << "home" << "end" << "pgup" << "pgdn" << "power" << "lightup" << "lightdn" << "up" << "down" << "left" << "right";

    // Populate boxes
    foreach(const QString& key, modKeys)
        ui->modBox->addItem(KeyMap::friendlyName(key));
    foreach(const QString& key, fnKeys)
        ui->fnBox->addItem(KeyMap::friendlyName(key).replace("\n", " / "));
    foreach(const QString& key, numKeys)
        ui->numBox->addItem(KeyMap::friendlyName(key).remove("NumPad "));
    foreach(const QString& key, mediaKeys)
        ui->mediaBox->addItem(KeyMap::friendlyName(key));
#ifdef Q_OS_MACOS
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
    ui->progTipLabel->setText("Tip: use the open command to launch a file, directory, or app. For instance, to start Safari:\n  open /Applications/Safari.app");
#else
    ui->progTipLabel->setText(tr("Tip: use xdg-open to launch a file or directory. For instance, to open your home folder:\n  xdg-open ") + QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
#endif
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &RebindWidget::tabChanged);
    ui->tableView->setModel(&macroLines);
    ui->tableView->setColumnWidth(0, 24);
    ui->tableView->setItemDelegateForColumn(0, macroDelegate);
    connect(ui->tableView->verticalScrollBar(), &QScrollBar::rangeChanged, this, [this](int min, int max){
        ui->tableView->verticalScrollBar()->setValue(max);
    });

    // Detect when the user closes the editor. Needed for when they press esc after they have added a new row manually
    connect(ui->tableView->itemDelegate(), &QAbstractItemDelegate::closeEditor, this, [this](QWidget* e, QAbstractItemDelegate::EndEditHint h = QAbstractItemDelegate::NoHint){
        setLocalUiElementsEnabled(ui, true);
        ui->btnStartMacro->setEnabled(true);
        if(h != QAbstractItemDelegate::RevertModelCache)
            return;
        macroLines.removeEmptyRowAtEnd();
    });
    connect(&macroLines, &QAbstractTableModel::dataChanged, this, &RebindWidget::regeneratePreview);
    connect(&macroLines, &QAbstractTableModel::modelReset, this, &RebindWidget::regeneratePreview);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

RebindWidget::~RebindWidget(){
    // Clean things up if the daemon died while we were recording a macro
    if(macroReader){
        // re-enable everything
        setUiElementsEnabled(ui, true);
        macroReader->deleteLater();
    }
    delete ui;
    delete macroDelegate;
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
            const Key& pos = map.key(name);
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
        setEnabled(false);
        return;
    } else
        setEnabled(true);

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
    setBox(nullptr);
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
    // Clear new UI elements in MacroTab
    ui->macroPreview->clear();
    ui->macroName->clear();
    macroLines.clear();
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
            if (act.isMacro()) {
                // This string needs to exist for as long as the references below are used
                QString value = act.value();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                // Split the action and discard the "$macro" from the beginning
                QVector<QStringRef> macroData = value.midRef(7).split(QChar(':'));
#else
                QStringView valueView(value);
                QList<QStringView> macroData = valueView.sliced(7).split(QChar(':'));
#endif
                const int dataCount = macroData.count();
                // Old format doesn't have the last field, which is the raw macro data
                // It might not exist at all (count == 3) or it might be set to "x"
                if(dataCount == 3 || dataCount == 4){
                    // Colons in the name field are escaped
                    QString macroName = macroData[2].toString();
                    macroName.replace("&das_IST_31N_col0n;", ":");
                    ui->macroName->setText(macroName);

                    // Pick the appropriate string to parse due to legacy formats
                    QStringView macroString = macroData[0];
                    if(dataCount == 4 && macroData[3] != QStringLiteral("x"))
                        macroString = macroData[3];

                    // Try to parse the string
                    // If there's an error, we'll ignore it and parse as much as possible
                    macroLines.fromString(macroString.toString(), false);

                    // If the daemon macro string contains even a single delay, then the we set the
                    // delay to "as typed" instead of "default"
                    QRegularExpression re("=\\d+");
                    if(re.match(macroData[0]).hasMatch())
                        ui->rb_delay_asTyped->setChecked(true);
                    else
                        ui->rb_delay_default->setChecked(true);
                }
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
    } else if (macroLines.length()) {
        // Check if the macro is matched, and if not, warn the user
        const int pos = macroLines.isMacroMatched();
        if(pos > -1){
            const MacroLine& ml = macroLines.at(pos);
            const QString msg = (ml.keyDown ?
                                     tr("Key %1 (%2) is pressed but never released.\n"
                                   "This will result in the key being pressed by the macro until you manually press the key itself and release it.\n\n"
                                   "Are you sure you want to continue?")
                                   : tr("Key %1 (%2) is released but never pressed.\n"
                                        "This will have no observable effect unless the key is held down manually or by another macro.\n\n"
                                        "Are you sure you want to continue?"));
            if(QMessageBox::warning(this, tr("Macro warning"),
                                 msg.arg(pos + 1).arg(ml.key),
                                 QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok) {
                // Select the row that's causing the warning
                ui->tableView->selectRow(pos);
                return;
            }
        }
        // Generate macro format string
        // Format is $macro:daemon_string:preview_string:user_specified_macro_name:original_recorded_macro_string
        // $macro: is added by KeyAction::macroAction()
        // daemon_string is the one sent to the daemon, and original_recorded_macro_string is the raw user data
        // preview_string is now unused as it can be generated from the remaining data
        // Any colons in user_specified_macro_name are escaped
        QString escapedMacroName = ui->macroName->text().replace(":", "&das_IST_31N_col0n;");
        QString finalDaemonString = macroLines.toString(false);
        QString originalString = macroLines.toString(true);
        QString macro = QString("%1::%2:%3").arg(finalDaemonString, escapedMacroName, originalString);
        bind->setAction(keys, KeyAction::macroAction(macro));
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
    if (box != ui->macroPreview) {
        on_btnClearMacro_clicked();
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
    } else {
        ui->programKpButton->setChecked(true);
        setBox(ui->programKpBox);
    }
}

void RebindWidget::on_programKrBox_textChanged(const QString &arg1){
    if(arg1.isEmpty()){
        ui->programKrButton->setChecked(false);
    } else {
        ui->programKrButton->setChecked(true);
        setBox(ui->programKrBox);
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

void RebindWidget::on_btnStartMacro_clicked() {
    // Get the capture type
    CAPTURE_TYPE type = static_cast<CAPTURE_TYPE>(ui->captureTypeBox->currentIndex());
    // If we're already capturing, stop
    if(macroReader) {
        macroReader->deleteLater();
        macroReader = nullptr;

        // Disable the macro notify node
        if(type == CAPTURE_TYPE::CAPTURE_CURRENT_DEVICE) {
            bind->handleNotificationChannel(false);
        } else if (type == CAPTURE_TYPE::CAPTURE_ALL_DAEMON_DEVICES) {
            foreach(Kb* dev, KbManager::devices()) {
                dev->currentBind()->handleNotificationChannel(false);
            }
        }

        // Remove the last mouse click if the user stopped
        // the capture by clicking on the Stop Recording button
        if(type != CAPTURE_TYPE::CAPTURE_ALL_KEYBOARDS && leftMouseClicked && ui->btnStartMacro->isClickedByMouse()){
            macroLines.removeLastMouseLeftClick();
            QString oldstr = ui->macroPreview->toPlainText();
            oldstr.replace(QRegularExpression("<mouse1>$"), QString());
            ui->macroPreview->setPlainText(oldstr);
        }

        ui->btnStartMacro->setText(tr("Start Recording"));
        ui->lbl_macro->setText(tr("Click Apply or manually edit the events."));
        ui->macroPreview->eatKeyEvents(false);
        ui->macroPreview->releaseKeyboard();
        setUiElementsEnabled(ui, true);

        // If we captured any data, clear all other selections in the widget
        if(macroLines.length())
            setBox(ui->macroPreview);
        return;
    }
    // Else, start a new capture
    ui->macroPreview->setFocus();

    // Start the appropriate capture mode
    QStringList devicePaths;

    // Enable the notify node
    if(type == CAPTURE_TYPE::CAPTURE_CURRENT_DEVICE) {
        bind->handleNotificationChannel(true);
        devicePaths.append(bind->getMacroPath());
    } else if (type == CAPTURE_TYPE::CAPTURE_ALL_DAEMON_DEVICES) {
        foreach(Kb* dev, KbManager::devices()) {
            dev->currentBind()->handleNotificationChannel(true);
            devicePaths.append(dev->getMacroPath());
        }
    }

    leftMouseClicked = false;

    macroReader = new MacroReader(devicePaths);
    connect(macroReader, &MacroReader::macroLineRead, this, &RebindWidget::macroLineRead);

    if(type == CAPTURE_TYPE::CAPTURE_ALL_KEYBOARDS){
        connect(ui->macroPreview, &NoUserInputTextEdit::macroKeyEvent, macroReader, &MacroReader::translateQKeyEvent);
        // Pause macro recording if an error is detected (unknown keypress)
        connect(macroReader, &MacroReader::macroReadError, this, [this](QString k, QString m) {
            ui->macroPreview->eatKeyEvents(false);
            ui->macroPreview->releaseKeyboard();
            QMessageBox::warning(this, tr("Unknown key combination pressed"), tr("An unknown key combination (%1, %2) has been pressed.\n"
                                                                     "Make sure your keyboard layout is set to English - United States while recording macros.")
                                 .arg(k, m));
            ui->macroPreview->grabKeyboard();
            ui->macroPreview->eatKeyEvents(true);
            macroReader->resetTimer();
        });
    }

    // Disable the buttons while recording
    ui->btnStartMacro->setText(tr("Stop Recording"));
    ui->lbl_macro->setText(tr("Type your macro and press Stop Recording when finished."));
    ui->macroPreview->eatKeyEvents(true);
    ui->macroPreview->grabKeyboard();
    setUiElementsEnabled(ui, false);
}

void RebindWidget::on_btnClearMacro_clicked() {
    ui->lbl_macro->setText(tr("Click Start Recording or manually edit the events."));
    macroLines.clear();
    ui->macroName->clear();
    ui->macroPreview->clear();
}

void RebindWidget::on_rb_delay_default_toggled(bool checked){
    macroLines.setDefaultDelay(checked);
}

void RebindWidget::insertIntoMacroPreview(const bool keydown, const bool printable, const QString& line){
    if(keydown){
        ui->macroPreview->moveCursor(QTextCursor::End);
        if(printable)
            ui->macroPreview->insertPlainText(line);
        else if(line == QString("space")) // Special case to handle the spacebar
            ui->macroPreview->insertPlainText(" ");
        else
            ui->macroPreview->insertPlainText("<" + line + ">");
    }
}

void RebindWidget::macroLineRead(QString line, qint64 ustime, bool keydown){
    const bool printable = (line.length() == 1);
    macroLines.append(MacroLine(line, ustime, ustime, keydown));

    // Add keystroke to the key actions textedit
    // "+k", "-k", these can be represented as text
    if(line == "mouse1")
        leftMouseClicked = true;
    else
        ui->macroPreview->setFocus();
    insertIntoMacroPreview(keydown, printable, line);
}

void RebindWidget::tabChanged(int idx){
    if(macroReader)
        return;
    // We monitor the tabs, and when we switch to the macro tab, we select the appropriate capture method
    if(ui->tabWidget->currentWidget() == ui->macroTab){
        if(bind->map().isKeyboard()){
            ui->captureTypeBox->setCurrentIndex(CAPTURE_TYPE::CAPTURE_CURRENT_DEVICE);
        } else {
            // Check if there are any daemon-managed keyboards connected, and if so, capture from them too
            foreach(const Kb* dev, KbManager::devices()){
                if(dev->isKeyboard()){
                    ui->captureTypeBox->setCurrentIndex(CAPTURE_TYPE::CAPTURE_ALL_DAEMON_DEVICES);
                    return;
                }
            }
            // If none were found, capture from everything as a last resort
            ui->captureTypeBox->setCurrentIndex(CAPTURE_TYPE::CAPTURE_ALL_KEYBOARDS);
        }
    }
}

// This is automatically called whenever setCurrentIndex is called programmatically as well
void RebindWidget::on_captureTypeBox_currentIndexChanged(int index){
    captureType = static_cast<CAPTURE_TYPE>(index);
    if(captureType != CAPTURE_ALL_KEYBOARDS)
        return;
    if(CkbSettings::get("UI/Bind/SuppressCaptureAllWarning", false).toBool())
        return;
    QMessageBox msg;
    // MessageBox takes ownership of the checkbox, so we don't need to delete it
    msg.setCheckBox(new QCheckBox("Don't warn me again"));
    msg.setIcon(QMessageBox::Warning);
    msg.addButton(QMessageBox::Ok);
    msg.setText(tr("\"Record from all keyboards\" is only recommended if you do not have a keyboard managed by ckb-next.\n"
                   "It currently only functions with an English - United States keyboard layout.\n"
                   "Make sure your keyboard is switched to it before recording."));
    msg.setWindowTitle(tr("Record from all keyboards"));
    msg.exec();
    CkbSettings::set("UI/Bind/SuppressCaptureAllWarning", msg.checkBox()->isChecked());
}

int RebindWidget::regeneratePreview(){
    ui->macroPreview->clear();
    const int len = macroLines.length();
    for(int i = 0; i < len; i++){
        const MacroLine& ml = macroLines.at(i);
        insertIntoMacroPreview(ml.keyDown, ml.isPrintable(), ml.key);
    }

    return len;
}

void RebindWidget::on_editAsStringBtn_clicked(){
    MacroStringEditDialog* dlg = new MacroStringEditDialog(&macroLines, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
    connect(dlg, &MacroStringEditDialog::accepted, this, [=](){
        const int len = macroLines.length();
        if(len)
            setBox(ui->macroPreview);
    });
}

void RebindWidget::on_btnRemoveEvent_clicked(){
    const QModelIndexList mil = ui->tableView->selectionModel()->selectedRows();
    if(!mil.length())
        return;

    macroLines.removeMultipleColumns(mil);
    regeneratePreview();
}

void RebindWidget::on_btnAddEvent_clicked(){
    setLocalUiElementsEnabled(ui, false);
    ui->btnStartMacro->setEnabled(false);
    const QModelIndex idx = macroLines.addBlankElement();
    ui->tableView->setCurrentIndex(idx);
    ui->tableView->edit(idx);
}
