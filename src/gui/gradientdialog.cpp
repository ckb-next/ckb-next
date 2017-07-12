#include <cmath>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include "ckbsettings.h"
#include "gradientdialog.h"
#include "ui_gradientdialog.h"

static const QString prefsPath = "Stored Gradients";

GradientDialog::GradientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GradientDialog)
{
    ui->setupUi(this);
    connect(ui->widget, SIGNAL(currentChanged(QColor,bool,int)), this, SLOT(currentChanged(QColor,bool,int)));
    connect(ui->stopColor, SIGNAL(colorChanged(QColor)), this, SLOT(colorChanged(QColor)));

    // Add built-in presets
    Preset blackAndWhite("B & W", true);
    blackAndWhite.gradient.append(QGradientStop(0., QColor(0, 0, 0)));
    blackAndWhite.gradient.append(QGradientStop(1., QColor(255, 255, 255)));
    Preset fade("Fade", true);
    fade.gradient.append(QGradientStop(0., QColor(255, 255, 255)));
    fade.gradient.append(QGradientStop(1., QColor(255, 255, 255, 0)));
    Preset rainbow("Rainbow", true);
    rainbow.gradient.append(QGradientStop(0.00, QColor(255, 0, 0)));
    rainbow.gradient.append(QGradientStop(0.17, QColor(255, 255, 0)));
    rainbow.gradient.append(QGradientStop(0.33, QColor(0, 255, 0)));
    rainbow.gradient.append(QGradientStop(0.50, QColor(0, 255, 255)));
    rainbow.gradient.append(QGradientStop(0.67, QColor(0, 0, 255)));
    rainbow.gradient.append(QGradientStop(0.83, QColor(255, 0, 255)));
    rainbow.gradient.append(QGradientStop(1.00, QColor(255, 0, 0)));
    addPreset(blackAndWhite);
    addPreset(fade);
    addPreset(rainbow);

    // Load stored presets
    CkbSettings settings(prefsPath);
    foreach(const QString& name, settings.childGroups()){
        QString pName = name.toLower();
        if(presets.contains(pName))
            continue;
        SGroup group(settings, name);
        // Create keys in a map to ensure correct order and no duplicates
        QMap<int, QColor> colors;
        foreach(const QString& position, settings.childKeys()){
            int pos = position.toInt();
            if(pos < 0 || pos > 100)
                continue;
            colors[pos] = settings.value(position).value<QColor>();
        }
        if(colors.count() < 2)
            continue;
        Preset preset(name);
        QMapIterator<int, QColor> i(colors);
        while(i.hasNext()){
            i.next();
            preset.gradient.append(QGradientStop(i.key() / 100., i.value()));
        }
        presets[pName] = preset;
    }

    updatePresets();
}

QGradientStops GradientDialog::getGradient(const QGradientStops& prevGradient){
    ui->presetDelete->setEnabled(false);
    ui->presetSave->setEnabled(true);
    ui->widget->setStops(prevGradient);
    exec();
    if(result() != QDialog::Accepted)
        return prevGradient;
    return ui->widget->stops();
}

GradientDialog::~GradientDialog(){
    // Save presets
    CkbSettings settings(prefsPath, true);
    QMapIterator<QString, Preset> i(presets);
    while(i.hasNext()){
        Preset preset = i.next().value();
        if(!preset.builtIn){
            SGroup group(settings, preset.name);
            foreach(const QGradientStop& stop, preset.gradient)
                settings.setValue(QString::number((int)(stop.first * 100.)), stop.second);
        }
    }
    delete ui;
}

QIcon GradientDialog::makeIcon(const Preset& preset){
    // Paint gradient into a square
    int w = 60, h = 60;
    QImage image(w, h, QImage::Format_RGB888);
    QPainter painter(&image);
    // Draw background
    for(int x = 0; x < w; x += 30){
        for(int y = 0; y < h; y += 30){
            painter.fillRect(x, y, 15, 15, QColor(255, 255, 255));
            painter.fillRect(x + 15, y, 15, 15, QColor(192, 192, 192));
            painter.fillRect(x, y + 15, 15, 15, QColor(192, 192, 192));
            painter.fillRect(x + 15, y + 15, 15, 15, QColor(255, 255, 255));
        }
    }
    // Draw gradient
    painter.setPen(QColor(0, 0, 0));
    QLinearGradient gradient(1., 1., w - 1., h - 1.);
    gradient.setStops(preset.gradient);
    painter.setBrush(gradient);
    painter.drawRect(0, 0, w - 1, h - 1);
    return QPixmap::fromImage(image);
}

void GradientDialog::updatePresets(){
    ui->presetList->clear();
    QMapIterator<QString, Preset> i(presets);
    while(i.hasNext()){
        i.next();
        const Preset& preset = i.value();
        QListWidgetItem* item = new QListWidgetItem(makeIcon(preset), preset.name, ui->presetList);
        if(currentPreset == i.key())
            item->setSelected(true);
        ui->presetList->addItem(item);
    }
}

void GradientDialog::currentChanged(QColor color, bool spontaneous, int position){
    int pCount = ui->widget->stopCount();
    if(pCount == 1)
        ui->stopBox->setTitle("1 point");
    else
        ui->stopBox->setTitle(QString("%1 points").arg(pCount));
    if(!color.isValid()){
        ui->stopBox->setEnabled(false);
        ui->stopPos->setValue(0);
        ui->stopColor->color(QColor(0, 0, 0));
        ui->stopOpacity->setValue(0);
    } else {
        ui->stopBox->setEnabled(true);
        ui->stopPos->setValue(position);
        ui->stopColor->color(color);
        ui->stopOpacity->setValue(round(color.alphaF() * 100.f));
    }
    // Un-set current preset
    if(spontaneous)
        setPreset("");
    setFocus();
}

void GradientDialog::setPreset(const QString &newPreset){
    if(currentPreset == newPreset)
        return;
    if(newPreset == ""){
        ui->presetList->clearSelection();
        ui->presetList->setCurrentItem(0);
        Preset current = presets.value(currentPreset);
        QString curName = ui->presetName->text().trimmed();
        if(curName == "" || (curName == current.name && current.builtIn))
            ui->presetName->setText("Custom");
        ui->presetDelete->setEnabled(false);
        ui->presetSave->setEnabled(true);
    } else {
        Preset current = presets.value(newPreset);
        ui->presetName->setText(current.name);
        ui->presetDelete->setEnabled(!current.builtIn);
        ui->presetSave->setEnabled(false);
        ui->widget->setStops(current.gradient);
    }
    currentPreset = newPreset;
    setFocus();
}

void GradientDialog::on_presetList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous){
    setPreset(current ? current->text().toLower() : "");
}

void GradientDialog::on_presetName_textEdited(const QString &arg1){
    if(currentPreset != "" && arg1.trimmed() != presets[currentPreset].name)
        setPreset("");
}

void GradientDialog::on_presetSave_clicked(){
    if(currentPreset != "")
        return;
    QString name = ui->presetName->text().trimmed();
    if(name == "")
        ui->presetName->setText(name = "Custom");
    QString pName = name.toLower();
    // Make sure not to overwrite a built-in preset
    Preset previous = presets[pName];
    if(previous.builtIn){
        QMessageBox::warning(this, "Error", "Can't overwrite a built-in preset. Please choose a different name.");
        setFocus();
        return;
    } else if(previous.name != ""){
        // Warn when overwriting an existing preset
        if(QMessageBox::warning(this, "Warning", QString("Preset \"%1\" already exists. Replace?").arg(name), QMessageBox::StandardButtons(QMessageBox::Save | QMessageBox::Cancel)) != QMessageBox::Save){
            setFocus();
            return;
        }
    }
    presets[pName] = Preset(name, ui->widget->stops());
    currentPreset = pName;
    ui->presetDelete->setEnabled(true);
    ui->presetSave->setEnabled(false);
    updatePresets();
    setFocus();
}

void GradientDialog::on_presetDelete_clicked(){
    Preset current = presets.value(currentPreset);
    if(current.name == "" || current.builtIn)
        return;
    if(QMessageBox::warning(this, "Warning", QString("Delete preset \"%1\"?").arg(current.name), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No)) != QMessageBox::Yes){
        setFocus();
        return;
    }
    presets.remove(currentPreset);
    currentPreset = "";
    ui->presetDelete->setEnabled(false);
    ui->presetSave->setEnabled(true);
    updatePresets();
    setFocus();
}

void GradientDialog::on_stopPos_valueChanged(int arg1){
    int res = ui->widget->moveCurrent(arg1);
    if(res != arg1)
        ui->stopPos->setValue(res);
    setPreset("");
}

void GradientDialog::colorChanged(QColor color){
    color.setAlphaF(ui->stopOpacity->value() / 100.f);
    ui->widget->setCurrentColor(color);
    setPreset("");
}

void GradientDialog::on_stopOpacity_valueChanged(int arg1){
    colorChanged(ui->stopColor->color());
    setPreset("");
}
