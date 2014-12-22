#include <cstdio>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include "animsettingdialog.h"
#include "ui_animsettingdialog.h"
#include "colorbutton.h"

AnimSettingDialog::AnimSettingDialog(QWidget* parent, KbAnim* anim) :
    QDialog(parent),
    ui(new Ui::AnimSettingDialog),
    _anim(anim), repCheck(0), kpRepCheck(0), lastDuration(1.0)
{
    ui->setupUi(this);
    setWindowTitle(anim->name());
    const AnimScript* script = anim->script();

    // Build settings UI
    ui->settingsGrid->addWidget(new QLabel("<b>Animation</b>", this), 0, 0, 1, 7);
    ui->settingsGrid->addWidget(hLine(), 1, 0, 1, 7);
    int row = 2;
    QListIterator<AnimScript::Param> i(script->paramIterator());
    while(i.hasNext()){
        const AnimScript::Param& param = i.next();
        if(param.name == "duration" || param.name == "trigger" || param.name == "kptrigger" || param.name == "repeat" || param.name == "kprepeat")
            continue;
        QVariant value = anim->parameters.value(param.name);
        // Display prefix label on the left (except for booleans)
        if(param.type != AnimScript::Param::BOOL)
            ui->settingsGrid->addWidget(new QLabel(param.prefix, this), row, 1);
        if(row == 2)
            ui->settingsGrid->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed), row, 2);
        // Configure and display main widget
        QWidget* widget = 0;
        int colSpan = 1;
        switch(param.type){
        case AnimScript::Param::BOOL:
            widget = new QCheckBox(param.prefix, this);
            ((QCheckBox*)widget)->setChecked(value.toBool());
            colSpan = 2;
            break;
        case AnimScript::Param::LONG:
            widget = new QSpinBox(this);
            ((QSpinBox*)widget)->setMinimum(param.minimum.toInt());
            ((QSpinBox*)widget)->setMaximum(param.maximum.toInt());
            ((QSpinBox*)widget)->setValue(value.toInt());
            break;
        case AnimScript::Param::DOUBLE:
            widget = new QDoubleSpinBox(this);
            ((QDoubleSpinBox*)widget)->setDecimals(1);
            ((QDoubleSpinBox*)widget)->setMinimum(param.minimum.toDouble());
            ((QDoubleSpinBox*)widget)->setMaximum(param.maximum.toDouble());
            ((QDoubleSpinBox*)widget)->setValue(value.toDouble());
            break;
        case AnimScript::Param::RGB:
            widget = new ColorButton(this);
            ((ColorButton*)widget)->color(QColor("#" + value.toString()));
            colSpan = 2;
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
            colSpan = 2;
            break;
        }
        default:
            break;
        }
        if(widget){
            settingWidgets[param.name] = widget;
            ui->settingsGrid->addWidget(widget, row, 3, 1, colSpan);
        }
        // Display postfix label on the right
        ui->settingsGrid->addWidget(new QLabel(param.postfix, this), row, 3 + colSpan, 1, 3 - colSpan);
        // On the first row, add a horizontal spacer to collapse the layout on to the left side
        if(row == 2)
            ui->settingsGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), row, 6);
        row++;
    }
    // Add timing info at bottom
    ui->settingsGrid->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), row++, 6);
    ui->settingsGrid->addWidget(new QLabel("<b>Timing</b>", this), row++, 0, 1, 7);
    ui->settingsGrid->addWidget(hLine(), row++, 0, 1, 7);
    if(script->hasParam("duration")){
        // Duration is required for all other timing parameters
        lastDuration = anim->parameters.value("duration").toDouble();
        // Show duration spinner
        ui->settingsGrid->addWidget(new QLabel("Duration", this), row, 1);
        QDoubleSpinBox* spinner = new QDoubleSpinBox(this);
        spinner->setDecimals(1);
        spinner->setMinimum(0.1);
        spinner->setValue(lastDuration);
        connect(spinner, SIGNAL(valueChanged(double)), this, SLOT(newDuration(double)));
        settingWidgets["duration"] = spinner;
        ui->settingsGrid->addWidget(spinner, row, 3, 1, 1);
        ui->settingsGrid->addWidget(new QLabel("seconds", this), row, 4, 1, 2);
        row++;
        if(script->hasParam("trigger")){
            // Start with mode allowed?
            if(script->hasParam("kptrigger") || script->hasParam("repeat")){
                ui->settingsGrid->addItem(new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed), row++, 6);
                ui->settingsGrid->addWidget(hLine(), row++, 0, 1, 7);
                ui->settingsGrid->addItem(new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed), row++, 6);
            }
            if(script->hasParam("kptrigger")){
                // Start with mode AND start with keypress allowed? Show check box
                QCheckBox* check = new QCheckBox("Start with mode", this);
                check->setChecked(anim->parameters.value("trigger").toBool());
                ui->settingsGrid->addWidget(check, row, 1, 1, 6);
                settingWidgets["trigger"] = check;
                row++;
            }
            if(script->hasParam("repeat")){
                // Repeat allowed? Add check box + spinner
                double repValue = anim->parameters.value("repeat").toDouble();
                repCheck = new QCheckBox("Repeat every:", this);
                repCheck->setChecked(repValue >= 0.);
                ui->settingsGrid->addWidget(repCheck, row, 1, 1, 2);
                QDoubleSpinBox* spinner = new QDoubleSpinBox(this);
                spinner->setDecimals(1);
                spinner->setMinimum(0.1);
                if(repValue >= 0.)
                    spinner->setValue(repValue);
                else
                    spinner->setValue(lastDuration);
                settingWidgets["repeat"] = spinner;
                ui->settingsGrid->addWidget(spinner, row, 3, 1, 1);
                ui->settingsGrid->addWidget(new QLabel("seconds", this), row, 4, 1, 2);
                row++;
            }
        }
        if(script->hasParam("kptrigger")){
            // Start with keypress allowed?
            if(script->hasParam("trigger") || script->hasParam("kprepeat")){
                ui->settingsGrid->addItem(new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed), row++, 6);
                ui->settingsGrid->addWidget(hLine(), row++, 0, 1, 7);
                ui->settingsGrid->addItem(new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed), row++, 6);
            }
            if(script->hasParam("trigger")){
                // Start with mode AND start with keypress allowed? Show check box
                QCheckBox* check = new QCheckBox("Start with key press", this);
                check->setChecked(anim->parameters.value("kptrigger").toBool());
                ui->settingsGrid->addWidget(check, row, 1, 1, 6);
                settingWidgets["kptrigger"] = check;
                row++;
            }
            if(script->hasParam("kprepeat")){
                // Repeat allowed? Add check box + spinner
                double repValue = anim->parameters.value("kprepeat").toDouble();
                kpRepCheck = new QCheckBox("Repeat every:", this);
                kpRepCheck->setChecked(repValue >= 0.);
                ui->settingsGrid->addWidget(kpRepCheck, row, 1, 1, 2);
                QDoubleSpinBox* spinner = new QDoubleSpinBox(this);
                spinner->setDecimals(1);
                spinner->setMinimum(0.1);
                if(repValue >= 0.)
                    spinner->setValue(repValue);
                else
                    spinner->setValue(lastDuration);
                settingWidgets["kprepeat"] = spinner;
                ui->settingsGrid->addWidget(spinner, row, 3, 1, 1);
                ui->settingsGrid->addWidget(new QLabel("seconds", this), row, 4, 1, 2);
                row++;
            }
        }
    }
    // Add vertical spacer to compress content to top
    ui->settingsGrid->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), row, 0);

    // Set script info UI
    ui->sNameLabel->setText(script->name());
    ui->sVerLabel->setText(script->version());
    ui->sAuthorLabel->setText(script->author());
    ui->sYearLabel->setText(script->year());
    ui->sLicenseLabel->setText(script->license());
    ui->sDescLabel->setText(script->description());
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

void AnimSettingDialog::applySettings(){
    // Apply all settings entered on the UI
    const AnimScript* script = _anim->script();
    QMapIterator<QString, QWidget*> i(settingWidgets);
    while(i.hasNext()){
        i.next();
        const QString& name = i.key();
        // Repeat and KP repeat can be defeated by switches
        if(name == "repeat" && !repCheck->isChecked()){
            _anim->parameters["repeat"] = -1.;
            continue;
        } else if(name == "kprepeat" && !kpRepCheck->isChecked()){
            _anim->parameters["kprepeat"] = -1.;
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
        }
        case AnimScript::Param::ARGB:{
            ColorButton* widget = (ColorButton*)settingWidgets[name];
            QColor color = widget->color();
            char hex[9];
            snprintf(hex, sizeof(hex), "%02x%02x%02x%02x", color.alpha(), color.red(), color.green(), color.blue());
            _anim->parameters[name] = QString(hex);
        }
        default:
            break;
        }
    }
    _anim->reInit();
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
