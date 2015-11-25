#include <cstdio>
#include <QDial>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include "animsettingdialog.h"
#include "ui_animsettingdialog.h"
#include "colorbutton.h"
#include "gradientbutton.h"

// QDial shows angles upside down (180° = top, 0° = bottom), so flip it
static inline int angleFlip(int angle){
    if(angle <= 180)
        return 180 - angle;
    else
       return 360 - (angle - 180);
}

AnimSettingDialog::AnimSettingDialog(QWidget* parent, KbAnim* anim) :
    QDialog(parent),
    ui(new Ui::AnimSettingDialog), stopCheck(0), kpStopCheck(0),
    _anim(anim), lastDuration(1.0)
{
    ui->setupUi(this);
    setWindowTitle(anim->scriptName() + " Animation");
    ui->animName->setText(anim->name());
    const AnimScript* script = anim->script();

    connect(&updateMapper, SIGNAL(mapped(QString)), this, SLOT(updateParam(QString)));
    connect(&angleDialMapper, SIGNAL(mapped(QString)), this, SLOT(angleDialChanged(QString)));
    connect(&angleSpinnerMapper, SIGNAL(mapped(QString)), this, SLOT(angleSpinnerChanged(QString)));

    // Build settings UI
    int row = 1;
    ui->settingsGrid->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), row++, 6);
    ui->settingsGrid->addWidget(new QLabel("<b>Animation</b>", this), row++, 0, 1, 7);
    ui->settingsGrid->addWidget(hLine(), row++, 0, 1, 7);
    bool rSpacePlaced = false;
    QListIterator<AnimScript::Param> i(script->paramIterator());
    while(i.hasNext()){
        const AnimScript::Param& param = i.next();
        // Skip timing/playback params
        if(param.name == "duration"
                || param.name == "trigger" || param.name == "kptrigger"
                || param.name == "kpmode"
                || param.name == "delay" || param.name == "kpdelay"
                || param.name == "repeat" || param.name == "kprepeat"
                || param.name == "stop" || param.name == "kpstop"
                || param.name == "kpmodestop" || param.name == "kprelease")
            continue;
        QVariant value = anim->parameter(param.name);
        // Display prefix label on the left (except for booleans and labels)
        if(param.type != AnimScript::Param::BOOL && param.type != AnimScript::Param::LABEL)
            ui->settingsGrid->addWidget(new QLabel(param.prefix, this), row, 1);
        // Configure and display main widget
        QWidget* widget = 0;
        int colSpan = 1;
        QString postfix = param.postfix;
        switch(param.type){
        case AnimScript::Param::BOOL:
            widget = new QCheckBox(param.prefix, this);
            ((QCheckBox*)widget)->setChecked(value.toBool());
            colSpan = 4;
            connect(widget, SIGNAL(stateChanged(int)), &updateMapper, SLOT(map()));
            break;
        case AnimScript::Param::LONG:
            widget = new QSpinBox(this);
            ((QSpinBox*)widget)->setMinimum(param.minimum.toInt());
            ((QSpinBox*)widget)->setMaximum(param.maximum.toInt());
            ((QSpinBox*)widget)->setValue(value.toInt());
            if(postfix.length() <= 3){
                ((QSpinBox*)widget)->setSuffix(postfix);
                postfix = "";
            }
            connect(widget, SIGNAL(valueChanged(int)), &updateMapper, SLOT(map()));
            break;
        case AnimScript::Param::DOUBLE:
            widget = new QDoubleSpinBox(this);
            ((QDoubleSpinBox*)widget)->setDecimals(1);
            ((QDoubleSpinBox*)widget)->setMinimum(param.minimum.toDouble());
            ((QDoubleSpinBox*)widget)->setMaximum(param.maximum.toDouble());
            ((QDoubleSpinBox*)widget)->setValue(value.toDouble());
            if(postfix.length() <= 3){
                ((QDoubleSpinBox*)widget)->setSuffix(postfix);
                postfix = "";
            }
            connect(widget, SIGNAL(valueChanged(double)), &updateMapper, SLOT(map()));
            break;
        case AnimScript::Param::RGB:
            widget = new ColorButton(this);
            ((ColorButton*)widget)->color(QColor("#" + value.toString()));
            colSpan = 3;
            connect(widget, SIGNAL(colorChanged(QColor)), &updateMapper, SLOT(map()));
            break;
        case AnimScript::Param::ARGB:{
            widget = new ColorButton(this, true);
            QString val = value.toString();
            QColor color;
            if(val.length() == 8){
                color = "#" + val.right(6);
                color.setAlpha(val.left(2).toInt(0, 16));
            } else
                color = "#" + val;
            ((ColorButton*)widget)->color(color);
            colSpan = 3;
            connect(widget, SIGNAL(colorChanged(QColor)), &updateMapper, SLOT(map()));
            break;
        }
        case AnimScript::Param::GRADIENT:
            widget = new GradientButton(this);
            ((GradientButton*)widget)->fromString(value.toString());
            colSpan = 3;
            connect(widget, SIGNAL(gradientChanged()), &updateMapper, SLOT(map()));
            break;
        case AnimScript::Param::AGRADIENT:
            widget = new GradientButton(this, true);
            ((GradientButton*)widget)->fromString(value.toString());
            colSpan = 3;
            connect(widget, SIGNAL(gradientChanged()), &updateMapper, SLOT(map()));
            break;
        case AnimScript::Param::ANGLE:
            widget = new QDial(this);
            ((QDial*)widget)->setFixedSize(60, 60);
            ((QDial*)widget)->setMinimum(0);
            ((QDial*)widget)->setMaximum(360);
            // this is NOT a typo...     ^
            // Even though 360 shouldn't be valid, the wheel/arrow keys will lose 1 degree at the bottom unless 360 is set as max
            ((QDial*)widget)->setSingleStep(5);
            ((QDial*)widget)->setPageStep(15);
            ((QDial*)widget)->setNotchTarget(6);
            ((QDial*)widget)->setNotchesVisible(true);
            ((QDial*)widget)->setWrapping(true);
            ((QDial*)widget)->setInvertedAppearance(true);
            ((QDial*)widget)->setValue(angleFlip(value.toInt()));
            angleDialMapper.setMapping(widget, param.name);
            connect(widget, SIGNAL(valueChanged(int)), &angleDialMapper, SLOT(map()));
            break;
        case AnimScript::Param::STRING:
            widget = new QLineEdit(this);
            ((QLineEdit*)widget)->setText(value.toString());
            colSpan = 3;
            connect(widget, SIGNAL(textEdited(const QString&)), &updateMapper, SLOT(map()));
            break;
        case AnimScript::Param::LABEL:
            widget = new QLabel(this);
            ((QLabel*)widget)->setText(param.prefix);
            colSpan = 4;
            break;
        default:
            break;
        }
        if(widget)
            updateMapper.setMapping(widget, param.name);
        if(param.type == AnimScript::Param::BOOL || param.type == AnimScript::Param::LABEL){
            // Boolean values are placed on the left with no prefix or postfix
            settingWidgets[param.name] = widget;
            ui->settingsGrid->addWidget(widget, row, 3, 1, colSpan);
        } else {
            // Display the widget
            if(widget){
                settingWidgets[param.name] = widget;
                ui->settingsGrid->addWidget(widget, row, 3, 1, colSpan);
            }
            // Angles additionally have a spin box
            if(param.type == AnimScript::Param::ANGLE){
                QSpinBox* spinner = new QSpinBox(this);
                spinner->setMinimum(0);
                spinner->setMaximum(359);
                spinner->setWrapping(true);
                spinner->setSuffix("°");
                spinner->setValue(value.toInt());
                angleSpinners[param.name] = spinner;
                angleSpinnerMapper.setMapping(spinner, param.name);
                connect(spinner, SIGNAL(valueChanged(int)), &angleSpinnerMapper, SLOT(map()));
                ui->settingsGrid->addWidget(spinner, row, 4);
                colSpan = 2;
            }
            // Display postfix label on the right
            ui->settingsGrid->addWidget(new QLabel(postfix, this), row, 3 + colSpan, 1, 4 - colSpan);
            if(colSpan < 3 && !rSpacePlaced){
                // Add a spacer to compress short elements to the left
                ui->settingsGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum), row, 4 + colSpan);
                rSpacePlaced = true;
            }
        }
        row++;
    }
    // Add playback info at bottom
    ui->settingsGrid->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), row++, 6);
    ui->settingsGrid->addWidget(new QLabel("<b>Playback</b>", this), row++, 0, 1, 7);
    ui->settingsGrid->addWidget(hLine(), row++, 0, 1, 7);
    if(script->hasParam("duration")){
        // Show duration spinner (if allowed)
        lastDuration = anim->parameter("duration").toDouble();
        ui->settingsGrid->addWidget(new QLabel("Duration:", this), row, 1);
        QDoubleSpinBox* spinner = new QDoubleSpinBox(this);
        spinner->setDecimals(1);
        spinner->setMinimum(0.1);
        spinner->setValue(lastDuration);
        connect(spinner, SIGNAL(valueChanged(double)), this, SLOT(newDuration(double)));
        settingWidgets["duration"] = spinner;
        ui->settingsGrid->addWidget(spinner, row, 3, 1, 1);
        ui->settingsGrid->addWidget(new QLabel("seconds", this), row, 4, 1, 2);
        row++;
    }
    // Show boxes for start with mode/with keypress
    QCheckBox* check = new QCheckBox("Start with mode", this);
    check->setChecked(anim->parameter("trigger").toBool());
    ui->settingsGrid->addWidget(check, row, 3, 1, 4);
    settingWidgets["trigger"] = check;
    connect(check, SIGNAL(stateChanged(int)), &updateMapper, SLOT(map()));
    updateMapper.setMapping(check, "trigger");
    row++;
    check = new QCheckBox("Start with key press", this);
    check->setChecked(anim->parameter("kptrigger").toBool());
    ui->settingsGrid->addWidget(check, row, 3, 1, 2);
    settingWidgets["kptrigger"] = check;
    connect(check, SIGNAL(stateChanged(int)), &updateMapper, SLOT(map()));
    updateMapper.setMapping(check, "kptrigger");
    // Add an option allowing the user to select keypress mode
    QComboBox* combo = new QComboBox(this);
    int selected = anim->parameter("kpmode").toInt();
    if(script->hasKeypress()){
        // If the script supports keypresses, show the option to handle them that way (default)
        combo->addItem("on pressed key");
        combo->addItem("on whole keyboard");
        combo->addItem("on keyboard (once)");
    } else {
        selected--;
        // Otherwise, just show the choice of whether to start it every time or just once
        combo->addItem("every time");
        combo->addItem("only once");
    }
    if(selected < 0 || selected > combo->count())
        selected = 0;
    combo->setCurrentIndex(selected);
    ui->settingsGrid->addWidget(combo, row, 5, 1, 2);
    settingWidgets["kpmode"] = combo;
    connect(combo, SIGNAL(activated(int)), &updateMapper, SLOT(map()));
    updateMapper.setMapping(combo, "kpmode");
    row++;

    // Add horizontal spacer to compress content to left
    ui->settingsGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 6);
    // Add vertical spacer to compress content to top
    ui->settingsGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), row, 0);

    // Add timing fields
    settingWidgets["delay"] = ui->delayBox;
    ui->delayBox->setValue(anim->parameter("delay").toDouble());
    settingWidgets["kpdelay"] = ui->kpDelayBox;
    ui->kpDelayBox->setValue(anim->parameter("kpdelay").toDouble());
    settingWidgets["kpmodestop"] = ui->kpModeStopBox;
    ui->kpModeStopBox->setChecked(anim->parameter("kpmodestop").toBool());
    connect(ui->kpModeStopBox, SIGNAL(clicked(bool)), &updateMapper, SLOT(map()));
    updateMapper.setMapping(ui->kpModeStopBox, "kpmodestop");
    settingWidgets["kprelease"] = ui->kpReleaseBox;
    ui->kpReleaseBox->setChecked(anim->parameter("kprelease").toBool());
    if(anim->hasParameter("repeat")){
        hasRepeat = true;
        settingWidgets["repeat"] = ui->repeatBox;
        ui->repeatBox->setValue(anim->parameter("repeat").toDouble());
        settingWidgets["kprepeat"] = ui->kpRepeatBox;
        ui->kpRepeatBox->setValue(anim->parameter("kprepeat").toDouble());
        // If repeat is enabled, add repeat counts as integer values
        // Mode repeat
        ui->timeGrid->addWidget(new QLabel("Repeat:", this), 4, 1);
        QSpinBox* spinner = new QSpinBox(this);
        spinner->setMinimum(0);
        spinner->setMaximum(1000000);
        spinner->setValue(anim->parameter("stop").toInt());
        settingWidgets["stop"] = spinner;
        connect(spinner, SIGNAL(valueChanged(int)), &updateMapper, SLOT(map()));
        updateMapper.setMapping(spinner, "stop");
        ui->timeGrid->addWidget(spinner, 4, 3);
        ui->timeGrid->addWidget(new QLabel("times", this), 4, 4);
        // KP repeat
        ui->timeGrid->addWidget(new QLabel("Repeat:", this), 12, 1);
        spinner = new QSpinBox(this);
        spinner->setMinimum(0);
        spinner->setMaximum(1000000);
        spinner->setValue(anim->parameter("kpstop").toInt());
        settingWidgets["kpstop"] = spinner;
        connect(spinner, SIGNAL(valueChanged(int)), &updateMapper, SLOT(map()));
        updateMapper.setMapping(spinner, "kpstop");
        ui->timeGrid->addWidget(spinner, 12, 3);
        ui->timeGrid->addWidget(new QLabel("times", this), 12, 4);
        // Infinite repeat toggles
        stopCheck = new QCheckBox("Forever", this);
        stopCheck->setChecked(anim->parameter("stop").toInt() < 0);
        ui->timeGrid->addWidget(stopCheck, 4, 5);
        kpStopCheck = new QCheckBox("Forever", this);
        connect(stopCheck, SIGNAL(clicked()), this, SLOT(updateStops()));
        kpStopCheck->setChecked(anim->parameter("kpstop").toInt() < 0);
        ui->timeGrid->addWidget(kpStopCheck, 12, 5);
        connect(kpStopCheck, SIGNAL(clicked()), this, SLOT(updateStops()));
    } else {
        hasRepeat = false;
        // If repeat is not enabled, hide repeat-related fields
        ui->repeatBox->setHidden(true);
        ui->repeatLabel->setHidden(true);
        ui->repeatLabel_2->setHidden(true);
        ui->kpRepeatBox->setHidden(true);
        ui->kpRepeatLabel->setHidden(true);
        ui->kpRepeatLabel_2->setHidden(true);
        // Add stop times as double values
        // Stop time
        QDoubleSpinBox* spinner = new QDoubleSpinBox(this);
        spinner->setDecimals(1);
        spinner->setMinimum(0.1);
        spinner->setMaximum(24. * 60. * 60.);
        double stop = anim->parameter("stop").toDouble();
        if(stop <= 0.)
            spinner->setValue(lastDuration);
        else
            spinner->setValue(stop);
        settingWidgets["stop"] = spinner;
        connect(spinner, SIGNAL(valueChanged(double)), &updateMapper, SLOT(map()));
        updateMapper.setMapping(spinner, "stop");
        ui->timeGrid->addWidget(spinner, 4, 3);
        ui->timeGrid->addWidget(new QLabel("seconds", this), 4, 4);
        // KP stop time
        spinner = new QDoubleSpinBox(this);
        spinner->setDecimals(1);
        spinner->setMinimum(0.1);
        spinner->setMaximum(24. * 60. * 60.);
        double kpstop = anim->parameter("kpstop").toDouble();
        if(kpstop <= 0.)
            spinner->setValue(lastDuration);
        else
            spinner->setValue(kpstop);
        settingWidgets["kpstop"] = spinner;
        connect(spinner, SIGNAL(valueChanged(double)), &updateMapper, SLOT(map()));
        updateMapper.setMapping(spinner, "kpstop");
        ui->timeGrid->addWidget(spinner, 12, 3);
        ui->timeGrid->addWidget(new QLabel("seconds", this), 12, 4);
        // Infinite run toggles
        stopCheck = new QCheckBox("Stop after:", this);
        stopCheck->setChecked(stop > 0.);
        ui->timeGrid->addWidget(stopCheck, 4, 1);
        connect(stopCheck, SIGNAL(clicked()), this, SLOT(updateStops()));
        kpStopCheck = new QCheckBox("Stop after:", this);
        kpStopCheck->setChecked(kpstop > 0.);
        ui->timeGrid->addWidget(kpStopCheck, 12, 1);
        connect(kpStopCheck, SIGNAL(clicked()), this, SLOT(updateStops()));
    }
    updateStops();

    // Set script info UI
    ui->sNameLabel->setText(script->name());
    ui->sVerLabel->setText(script->version());
    ui->sAuthorLabel->setText(script->author());
    ui->sYearLabel->setText(script->year());
    ui->sLicenseLabel->setText(script->license());
    ui->sDescLabel->setText(script->description());

    // Lock dialog size
    setFixedSize(minimumSize());
}

void AnimSettingDialog::newDuration(double duration){
    // Duration changed. Automatically fix Repeat and KP Repeat to the same values, if they matched before
    QDoubleSpinBox* rep = (QDoubleSpinBox*)settingWidgets.value("repeat");
    QDoubleSpinBox* kpRep = (QDoubleSpinBox*)settingWidgets.value("kprepeat");
    if(rep && rep->value() == lastDuration)
        rep->setValue(duration);
    if(kpRep && kpRep->value() == lastDuration)
        kpRep->setValue(duration);
    lastDuration = duration;
    updateParam("duration");
    updateParam("repeat");
    updateParam("kprepeat");
}

void AnimSettingDialog::updateStops(){
    if(hasRepeat){
        // When repeat is enabled, the check box is "repeat forever", so disable stop count if checked
        settingWidgets.value("stop")->setDisabled(stopCheck->isChecked());
        settingWidgets.value("kpstop")->setDisabled(kpStopCheck->isChecked());
    } else {
        // When repeat is disabled, the check box is "stop after", so disable stop count UNLESS checked
        settingWidgets.value("stop")->setEnabled(stopCheck->isChecked());
        settingWidgets.value("kpstop")->setEnabled(kpStopCheck->isChecked());
    }
    updateParam("stop");
    updateParam("kpstop");
}

void AnimSettingDialog::angleDialChanged(QString name){
    // Dial changed; update spinner value
    angleSpinners[name]->setValue(angleFlip(((QDial*)settingWidgets[name])->value()));
    updateParam(name);
}

void AnimSettingDialog::angleSpinnerChanged(QString name){
    // Spinner changed; update dial value
    ((QDial*)settingWidgets[name])->setValue(angleFlip(angleSpinners[name]->value()));
    updateParam(name);
}

void AnimSettingDialog::updateParam(QString name){
    if(!settingWidgets.contains(name))
        return;
    // stop and kpstop have defeat switches
    if((name == "stop" && stopCheck->isChecked() == hasRepeat)
            || (name == "kpstop" && kpStopCheck->isChecked() == hasRepeat)){
        _anim->parameter(name, -1);
        return;
    } else if(name == "kpmode"){
        // kpmode uses a drop-down, and selection is inverted
        QComboBox* widget = (QComboBox*)settingWidgets[name];
        int selected = widget->currentIndex();
        if(!_anim->script()->hasKeypress())
            selected++;
        _anim->parameter(name, selected);
        return;
    }
    // Read value based on type
    switch(_anim->script()->param(name).type){
    case AnimScript::Param::BOOL:{
        QCheckBox* widget = (QCheckBox*)settingWidgets[name];
        _anim->parameter(name, (int)widget->isChecked());
        break;
    }
    case AnimScript::Param::LONG:{
        QSpinBox* widget = (QSpinBox*)settingWidgets[name];
        _anim->parameter(name, widget->value());
        break;
    }
    case AnimScript::Param::DOUBLE:{
        QDoubleSpinBox* widget = (QDoubleSpinBox*)settingWidgets[name];
        _anim->parameter(name, widget->value());
        break;
    }
    case AnimScript::Param::RGB:{
        ColorButton* widget = (ColorButton*)settingWidgets[name];
        QColor color = widget->color();
        char hex[7];
        snprintf(hex, sizeof(hex), "%02x%02x%02x", color.red(), color.green(), color.blue());
        _anim->parameter(name, QString(hex));
        break;
    }
    case AnimScript::Param::ARGB:{
        ColorButton* widget = (ColorButton*)settingWidgets[name];
        QColor color = widget->color();
        char hex[9];
        snprintf(hex, sizeof(hex), "%02x%02x%02x%02x", color.alpha(), color.red(), color.green(), color.blue());
        _anim->parameter(name, QString(hex));
        break;
    }
    case AnimScript::Param::GRADIENT:
    case AnimScript::Param::AGRADIENT:{
        GradientButton* widget = (GradientButton*)settingWidgets[name];
        _anim->parameter(name, widget->toString());
        break;
    }
    case AnimScript::Param::ANGLE:{
        QDial* widget = (QDial*)settingWidgets[name];
        _anim->parameter(name, angleFlip(widget->value()));
        break;
    }
    case AnimScript::Param::STRING:{
        QLineEdit* widget = (QLineEdit*)settingWidgets[name];
        _anim->parameter(name, widget->text());
        break;
    }
    default:
        break;
    }
}

QString AnimSettingDialog::name() const {
    return ui->animName->text();
}

AnimSettingDialog::~AnimSettingDialog(){
    delete ui;
}

QFrame* AnimSettingDialog::hLine(){
    QFrame* frame = new QFrame(this);
    frame->setFrameShape(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    return frame;
}

void AnimSettingDialog::on_delayBox_valueChanged(double arg1){
    updateParam("delay");
}

void AnimSettingDialog::on_repeatBox_valueChanged(double arg1){
    updateParam("repeat");
}

void AnimSettingDialog::on_kpDelayBox_valueChanged(double arg1){
    updateParam("kpdelay");
}

void AnimSettingDialog::on_kpRepeatBox_valueChanged(double arg1){
    updateParam("kprepeat");
}

void AnimSettingDialog::on_kpReleaseBox_stateChanged(int arg1){
    updateParam("kprelease");
}
