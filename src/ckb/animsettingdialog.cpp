#include <cstdio>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>
#include "animsettingdialog.h"
#include "ui_animsettingdialog.h"
#include "colorbutton.h"
#include "gradientbutton.h"

AnimSettingDialog::AnimSettingDialog(QWidget* parent, KbAnim* anim) :
    QDialog(parent),
    ui(new Ui::AnimSettingDialog), stopCheck(0), kpStopCheck(0),
    _anim(anim), lastDuration(1.0)
{
    ui->setupUi(this);
    setWindowTitle(anim->scriptName() + " Animation");
    ui->animName->setText(anim->name());
    const AnimScript* script = anim->script();

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
                || param.name == "delay" || param.name == "kpdelay"
                || param.name == "repeat" || param.name == "kprepeat"
                || param.name == "stop" || param.name == "kpstop" || param.name == "kprelease")
            continue;
        QVariant value = anim->parameters.value(param.name);
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
            break;
        case AnimScript::Param::RGB:
            widget = new ColorButton(this);
            ((ColorButton*)widget)->color(QColor("#" + value.toString()));
            colSpan = 3;
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
            break;
        }
        case AnimScript::Param::GRADIENT:
            widget = new GradientButton(this);
            ((GradientButton*)widget)->fromString(value.toString());
            colSpan = 3;
            break;
        case AnimScript::Param::AGRADIENT:
            widget = new GradientButton(this, true);
            ((GradientButton*)widget)->fromString(value.toString());
            colSpan = 3;
            break;
        case AnimScript::Param::STRING:
            widget = new QLineEdit(this);
            ((QLineEdit*)widget)->setText(value.toString());
            colSpan = 3;
            break;
        case AnimScript::Param::LABEL:
            widget = new QLabel(this);
            ((QLabel*)widget)->setText(param.prefix);
            colSpan = 4;
            break;
        default:
            break;
        }
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
            // Display postfix label on the right
            ui->settingsGrid->addWidget(new QLabel(postfix, this), row, 3 + colSpan, 1, 4 - colSpan);
            if(colSpan < 3 && !rSpacePlaced){
                // Additionally add spacers to compress short elements to the left
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
        lastDuration = anim->parameters.value("duration").toDouble();
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
    check->setChecked(anim->parameters.value("trigger").toBool());
    ui->settingsGrid->addWidget(check, row, 3, 1, 4);
    settingWidgets["trigger"] = check;
    row++;
    check = new QCheckBox("Start with key press", this);
    check->setChecked(anim->parameters.value("kptrigger").toBool());
    ui->settingsGrid->addWidget(check, row, 3, 1, 4);
    settingWidgets["kptrigger"] = check;
    row++;
    // Add horizontal spacer to compress content to left
    ui->settingsGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 6);
    // Add vertical spacer to compress content to top
    ui->settingsGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), row, 0);

    // Add timing fields
    settingWidgets["delay"] = ui->delayBox;
    ui->delayBox->setValue(anim->parameters.value("delay").toDouble());
    settingWidgets["kpdelay"] = ui->kpDelayBox;
    ui->kpDelayBox->setValue(anim->parameters.value("kpdelay").toDouble());
    ui->kpReleaseBox->setChecked(anim->parameters.value("kprelease").toBool());
    settingWidgets["kprelease"] = ui->kpReleaseBox;
    if(anim->parameters.contains("repeat")){
        hasRepeat = true;
        settingWidgets["repeat"] = ui->repeatBox;
        settingWidgets["kprepeat"] = ui->kpRepeatBox;
        ui->repeatBox->setValue(anim->parameters.value("repeat").toDouble());
        ui->kpRepeatBox->setValue(anim->parameters.value("kprepeat").toDouble());
        // If repeat enabled, add repeat counts as integer values
        QSpinBox* spinner;
        ui->timeGrid->addWidget(new QLabel("Repeat:", this), 4, 1);
        spinner = new QSpinBox(this);
        spinner->setMinimum(0);
        spinner->setMaximum(1000000);
        spinner->setValue(anim->parameters.value("stop").toInt());
        settingWidgets["stop"] = spinner;
        ui->timeGrid->addWidget(spinner, 4, 3);
        ui->timeGrid->addWidget(new QLabel("times", this), 4, 4);

        ui->timeGrid->addWidget(new QLabel("Repeat:", this), 10, 1);
        spinner = new QSpinBox(this);
        spinner->setMinimum(0);
        spinner->setMaximum(1000000);
        spinner->setValue(anim->parameters.value("kpstop").toInt());
        settingWidgets["kpstop"] = spinner;
        ui->timeGrid->addWidget(spinner, 10, 3);
        ui->timeGrid->addWidget(new QLabel("times", this), 10, 4);

        stopCheck = new QCheckBox("Forever", this);
        stopCheck->setChecked(anim->parameters.value("stop").toInt() < 0);
        ui->timeGrid->addWidget(stopCheck, 4, 5);
        kpStopCheck = new QCheckBox("Forever", this);
        connect(stopCheck, SIGNAL(clicked()), this, SLOT(updateStops()));
        kpStopCheck->setChecked(anim->parameters.value("kpstop").toInt() < 0);
        ui->timeGrid->addWidget(kpStopCheck, 10, 5);
        connect(kpStopCheck, SIGNAL(clicked()), this, SLOT(updateStops()));
    } else {
        hasRepeat = false;
        // Hide repeat fields if not supported
        ui->repeatBox->setHidden(true);
        ui->repeatLabel->setHidden(true);
        ui->repeatLabel_2->setHidden(true);
        ui->kpRepeatBox->setHidden(true);
        ui->kpRepeatLabel->setHidden(true);
        ui->kpRepeatLabel_2->setHidden(true);
        // Add stop times as double values
        QDoubleSpinBox* spinner;
        spinner = new QDoubleSpinBox(this);
        spinner->setDecimals(1);
        spinner->setMinimum(0.1);
        spinner->setMaximum(24. * 60. * 60.);
        double stop = anim->parameters.value("stop").toDouble();
        if(stop <= 0.)
            spinner->setValue(lastDuration);
        else
            spinner->setValue(stop);
        settingWidgets["stop"] = spinner;
        ui->timeGrid->addWidget(spinner, 4, 3);
        ui->timeGrid->addWidget(new QLabel("seconds", this), 4, 4);

        spinner = new QDoubleSpinBox(this);
        spinner->setDecimals(1);
        spinner->setMinimum(0.1);
        spinner->setMaximum(24. * 60. * 60.);
        double kpstop = anim->parameters.value("kpstop").toDouble();
        if(kpstop <= 0.)
            spinner->setValue(lastDuration);
        else
            spinner->setValue(kpstop);
        settingWidgets["kpstop"] = spinner;
        ui->timeGrid->addWidget(spinner, 10, 3);
        ui->timeGrid->addWidget(new QLabel("seconds", this), 10, 4);

        stopCheck = new QCheckBox("Stop after:", this);
        stopCheck->setChecked(stop > 0.);
        ui->timeGrid->addWidget(stopCheck, 4, 1);
        connect(stopCheck, SIGNAL(clicked()), this, SLOT(updateStops()));
        kpStopCheck = new QCheckBox("Stop after:", this);
        kpStopCheck->setChecked(kpstop > 0.);
        ui->timeGrid->addWidget(kpStopCheck, 10, 1);
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
}

void AnimSettingDialog::applySettings(){
    // Apply all settings entered on the UI
    const AnimScript* script = _anim->script();
    QMapIterator<QString, QWidget*> i(settingWidgets);
    while(i.hasNext()){
        i.next();
        const QString& name = i.key();
        // stop and kpstop have defeat switches
        if((name == "stop" && stopCheck->isChecked() == hasRepeat)
                || (name == "kpstop" && kpStopCheck->isChecked() == hasRepeat)){
            _anim->parameters[name] = -1;
            continue;
        }
        // Read value based on type
        switch(script->param(name).type){
        case AnimScript::Param::BOOL:{
            QCheckBox* widget = (QCheckBox*)settingWidgets[name];
            _anim->parameters[name] = (int)widget->isChecked();
            break;
        }
        case AnimScript::Param::LONG:{
            QSpinBox* widget = (QSpinBox*)settingWidgets[name];
            _anim->parameters[name] = widget->value();
            break;
        }
        case AnimScript::Param::DOUBLE:{
            QDoubleSpinBox* widget = (QDoubleSpinBox*)settingWidgets[name];
            _anim->parameters[name] = widget->value();
            break;
        }
        case AnimScript::Param::RGB:{
            ColorButton* widget = (ColorButton*)settingWidgets[name];
            QColor color = widget->color();
            char hex[7];
            snprintf(hex, sizeof(hex), "%02x%02x%02x", color.red(), color.green(), color.blue());
            _anim->parameters[name] = QString(hex);
            break;
        }
        case AnimScript::Param::ARGB:{
            ColorButton* widget = (ColorButton*)settingWidgets[name];
            QColor color = widget->color();
            char hex[9];
            snprintf(hex, sizeof(hex), "%02x%02x%02x%02x", color.alpha(), color.red(), color.green(), color.blue());
            _anim->parameters[name] = QString(hex);
            break;
        }
        case AnimScript::Param::GRADIENT:
        case AnimScript::Param::AGRADIENT:{
            GradientButton* widget = (GradientButton*)settingWidgets[name];
            _anim->parameters[name] = widget->toString();
            break;
        }
        case AnimScript::Param::STRING:{
            QLineEdit* widget = (QLineEdit*)settingWidgets[name];
            _anim->parameters[name] = widget->text();
        }
        default:
            break;
        }
    }
    _anim->reInit();
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
