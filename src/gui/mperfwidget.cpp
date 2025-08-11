#include "mperfwidget.h"
#include "ui_mperfwidget.h"
#include "modeselectdialog.h"
#include <cmath>
#include "compat/qoverloadlegacy.h"

const static QString xyLinkPath = "UI/DPI/UnlinkXY";

MPerfWidget::MPerfWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::MPerfWidget), perf(nullptr), profile(nullptr), _xyLink(!CkbSettings::get(xyLinkPath).toBool()), colorLink(false), isSetting(false), isDarkCore(false) {
// Re-declare checkStateChanged() signal if Qt Version is below 6.7.0.
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    void (QCheckBox::*checkStateChanged)(Qt::CheckState) = &QCheckBox::checkStateChanged;
#else // QT_VERSION < 6.7.0
    void (QCheckBox::*checkStateChanged)(int) = &QCheckBox::stateChanged;
#endif

    ui->setupUi(this);
    ui->xyBox->setChecked(!_xyLink);
    // Set up DPI stages
    stages[0].indicator = ui->iButton0; stages[0].xSlider = ui->xSlider0; stages[0].ySlider = ui->ySlider0; stages[0].xBox = ui->xBox0; stages[0].yBox = ui->yBox0; stages[0].enableCheck = nullptr;
    stages[1].indicator = ui->iButton1; stages[1].xSlider = ui->xSlider1; stages[1].ySlider = ui->ySlider1; stages[1].xBox = ui->xBox1; stages[1].yBox = ui->yBox1; stages[1].enableCheck = ui->eBox1;
    stages[2].indicator = ui->iButton2; stages[2].xSlider = ui->xSlider2; stages[2].ySlider = ui->ySlider2; stages[2].xBox = ui->xBox2; stages[2].yBox = ui->yBox2; stages[2].enableCheck = ui->eBox2;
    stages[3].indicator = ui->iButton3; stages[3].xSlider = ui->xSlider3; stages[3].ySlider = ui->ySlider3; stages[3].xBox = ui->xBox3; stages[3].yBox = ui->yBox3; stages[3].enableCheck = ui->eBox3;
    stages[4].indicator = ui->iButton4; stages[4].xSlider = ui->xSlider4; stages[4].ySlider = ui->ySlider4; stages[4].xBox = ui->xBox4; stages[4].yBox = ui->yBox4; stages[4].enableCheck = ui->eBox4;
    stages[5].indicator = ui->iButton5; stages[5].xSlider = ui->xSlider5; stages[5].ySlider = ui->ySlider5; stages[5].xBox = ui->xBox5; stages[5].yBox = ui->yBox5; stages[5].enableCheck = ui->eBox5;
    stages[0].indicatorLabel = ui->iLabel0;
    stages[1].indicatorLabel = ui->iLabel1;
    stages[2].indicatorLabel = ui->iLabel2;
    stages[3].indicatorLabel = ui->iLabel3;
    stages[4].indicatorLabel = ui->iLabel4;
    stages[5].indicatorLabel = ui->iLabel5;

    ui->iButtonO->setLabel(false);
    ui->iButtonO->bigIcons(true);
    ui->iButtonO->allowAlpha(true);

    for(int i = 0; i < DPI_COUNT; i++){
        stages[i].indicator->setLabel(false);
        stages[i].indicator->bigIcons(true);
        stages[i].indicator->allowAlpha(true);
        // Disable tracking so that the slider only sends the value when the user has stopped moving it
        stages[i].xSlider->setTracking(false);
        stages[i].ySlider->setTracking(false);
        // Map signals
        connect(stages[i].indicator, &ColorButton::clicked, [=] (){
            emit colorClicked(i);
        });
        connect(stages[i].indicator, &ColorButton::colorChanged, [=] () {
            emit colorChanged(i);
        });
        // valueChanged is used to update the settings
        // sliderMoved is used to update the spinbox in realtime
        connect(stages[i].xSlider, &QSlider::valueChanged, [=] () {
            emit sliderXValueChanged(i);
        });
        connect(stages[i].ySlider, &QSlider::valueChanged, [=] () {
            emit sliderYValueChanged(i);
        });
        connect(stages[i].xSlider, &QSlider::sliderMoved, [=] () {
            emit sliderXMoved(i);
        });
        connect(stages[i].ySlider, &QSlider::sliderMoved, [=] () {
            emit sliderYMoved(i);
        });
        connect(stages[i].xBox, OVERLOAD_PTR(int, QSpinBox, valueChanged), [=] () {
            emit boxXChanged(i);
        });
        connect(stages[i].yBox, OVERLOAD_PTR(int, QSpinBox, valueChanged), [=] () {
            emit boxYChanged(i);
        });
        if(stages[i].enableCheck)
            // Sniper has no enable
            connect(stages[i].enableCheck, checkStateChanged, [=] () {
                emit enableChanged(i);
            });
        // Hide indicator arrows
        stages[i].indicatorLabel->setVisible(false);
    }
    ui->iLabelO->setVisible(false);
}

MPerfWidget::~MPerfWidget(){
    delete ui;
}

void MPerfWidget::setPerf(KbPerf *newPerf, KbProfile *newProfile){
    perf = newPerf;
    profile = newProfile;
    ui->spinBox->setValue(std::round(perf->iOpacity() * 100.f));
    for(int i = 0; i < DPI_COUNT; i++){
        stages[i].indicator->color(perf->dpiColor(i));
        bool oldLink = _xyLink;
        // Don't force X/Y to the same value if they were set differently in the past
        _xyLink = false;
        stages[i].xBox->setValue(perf->dpi(i).x());
        stages[i].yBox->setValue(perf->dpi(i).y());
        _xyLink = oldLink;
        if(stages[i].enableCheck)
            stages[i].enableCheck->setChecked(perf->dpiEnabled(i));
    }
    ui->iButtonO->color(perf->dpiColor(KbPerf::OTHER));
    ui->aSnapBox->setChecked(perf->angleSnap());
    //Subtract 2 from profile values if using a dark core, since it only supports 3 heights
    if(isDarkCore)
        ui->lHeightBox->setCurrentIndex(perf->liftHeight() - 3);
    else
        ui->lHeightBox->setCurrentIndex(perf->liftHeight() - 1);
    ui->indicBox->setChecked(perf->dpiIndicator());
    highlightDpiBox(perf->getDpiIdx());
    connect(perf, &KbPerf::dpiChanged, this, &MPerfWidget::highlightDpiBox);
}

void MPerfWidget::colorClicked(int index){
    // Set all colors at once if Alt is held down
    colorLink = !!(qApp->keyboardModifiers() & Qt::AltModifier);
}

void MPerfWidget::colorChanged(int index){
    QColor color = stages[index].indicator->color();
    if(colorLink){
        // Alt was held down - set all
        for(int i = 0; i < DPI_COUNT; i++){
            stages[i].indicator->color(color);
            perf->dpiColor(i, color);
        }
        ui->iButtonO->color(color);
        perf->dpiColor(KbPerf::OTHER, color);
    } else {
        // Set one
        perf->dpiColor(index, color);
    }
    colorLink = false;
}

static inline int dpiExp(int value, int min, int max){
    const double DPI_BASE = (double)max / (double)KbPerf::DPI_MIN;
    double val = (value - min) / (double)(max - min);
    val = pow(DPI_BASE, val);
    return round(KbPerf::DPI_MIN * val);
}

static inline int dpiLog(int value, int min, int max){
    const double DPI_BASE = (double)max / (double)KbPerf::DPI_MIN;
    double val = value / (double)KbPerf::DPI_MIN;
    val = log(val) / log(DPI_BASE);
    return round(val * (max - min) + min);
}

static inline int dpiRound(int value){
    if(value > 1000)
        return round(value / 100.) * 100;
    else
        return round(value / 50.) * 50;
}

static inline int calculateSliderDpi(QSlider* slider){
    return dpiRound(dpiExp(slider->sliderPosition(), slider->minimum(), slider->maximum()));
}


// Potential recursion in sliderMoved/boxMoved due to rounding
#define SET_START   if(isSetting) return; isSetting = true
#define SET_END     isSetting = false



void MPerfWidget::sliderXValueChanged(int index){
    SET_START;
    QSlider* slider = stages[index].xSlider;
    int value = calculateSliderDpi(slider);
    if(_xyLink)
        perf->dpi(index, value);
    else
        perf->dpi(index, QPoint(value, perf->dpi(index).y()));
    stages[index].xBox->setValue(value);
    if(_xyLink){
        stages[index].ySlider->setValue(slider->sliderPosition());
        stages[index].yBox->setValue(value);
    }
    SET_END;
}

void MPerfWidget::sliderXMoved(int index){
    SET_START;
    QSlider* slider = stages[index].xSlider;
    int value = calculateSliderDpi(slider);
    stages[index].xBox->setValue(value);
    if(_xyLink){
        stages[index].ySlider->setValue(slider->sliderPosition());
        stages[index].yBox->setValue(value);
    }
    SET_END;
}

void MPerfWidget::sliderYValueChanged(int index){
    SET_START;
    QSlider* slider = stages[index].ySlider;
    int value = calculateSliderDpi(slider);
    if(_xyLink)
        perf->dpi(index, value);
    else
        perf->dpi(index, QPoint(perf->dpi(index).x(), value));
    stages[index].yBox->setValue(value);
    if(_xyLink){
        stages[index].xSlider->setValue(slider->sliderPosition());
        stages[index].xBox->setValue(value);
    }
    SET_END;
}

void MPerfWidget::sliderYMoved(int index){
    SET_START;
    QSlider* slider = stages[index].ySlider;
    int value = calculateSliderDpi(slider);
    stages[index].yBox->setValue(value);
    if(_xyLink){
        stages[index].xSlider->setValue(slider->sliderPosition());
        stages[index].xBox->setValue(value);
    }
    SET_END;
}

void MPerfWidget::setLegacyM95(){
    ui->eBox4->setChecked(false);
    ui->eBox5->setChecked(false);
    QWidget* w[] =  {
        ui->indicBox,
        ui->spinBox,

        // Hide colour buttons
        ui->iButton0,
        ui->iButton1,
        ui->iButton2,
        ui->iButton3,
        ui->iButton4,
        ui->iButton5,
        ui->iButtonO,

        // Hide stages 4, 5, and other completely
        ui->label_3,
        ui->label_15,

        // Hide checkboxes as we can't disable DPI stages
        ui->eBox1,
        ui->eBox2,
        ui->eBox3,
        ui->eBox4,
        ui->eBox5,
        ui->label_13,
        ui->label_8,
        ui->xSlider4,
        ui->xSlider5,
        ui->ySlider4,
        ui->ySlider5,
        ui->xBox4,
        ui->xBox5,
        ui->yBox5,
        ui->yBox4
    };
    for(size_t i = 0; i < sizeof(w) / sizeof(QWidget*); i++)
        w[i]->hide();
}

void MPerfWidget::setDarkCore(){
    isDarkCore = true;

    // Remove lift height elements, which do not work
    ui->lHeightBox->removeItem(1);
    ui->lHeightBox->removeItem(2);

    // Disable DPI profile 4 and 5
    ui->eBox4->setChecked(false);
    ui->eBox5->setChecked(false);

    // Hide DPI profiles 4 and 5
    QWidget* w[] = {
        ui->iButton4,
        ui->iButton5,
        ui->eBox4,
        ui->eBox5,
        ui->label_13,
        ui->label_8,
        ui->xSlider4,
        ui->xSlider5,
        ui->ySlider4,
        ui->ySlider5,
        ui->xBox4,
        ui->xBox5,
        ui->yBox5,
        ui->yBox4
    };

    for(size_t i = 0; i < sizeof(w) / sizeof(QWidget*); i++)
        w[i]->hide();
}

void MPerfWidget::boxXChanged(int index){
    SET_START;
    QSpinBox* box = stages[index].xBox;
    QSlider* slider = stages[index].xSlider;
    int value = box->value();
    slider->setValue(dpiLog(value, slider->minimum(), slider->maximum()));
    if(_xyLink)
        perf->dpi(index, value);
    else
        perf->dpi(index, QPoint(value, perf->dpi(index).y()));
    SET_END;
    if(_xyLink)
        stages[index].yBox->setValue(value);
}

void MPerfWidget::boxYChanged(int index){
    SET_START;
    QSpinBox* box = stages[index].yBox;
    QSlider* slider = stages[index].ySlider;
    int value = box->value();
    slider->setValue(dpiLog(value, slider->minimum(), slider->maximum()));
    if(_xyLink)
        perf->dpi(index, value);
    else
        perf->dpi(index, QPoint(perf->dpi(index).x(), value));
    SET_END;
    if(_xyLink)
        stages[index].xBox->setValue(value);
}

void MPerfWidget::enableChanged(int index){
    perf->dpiEnabled(index, stages[index].enableCheck->isChecked());
}

void MPerfWidget::on_xyBox_clicked(bool checked){
    _xyLink = !checked;
    CkbSettings::set(xyLinkPath, checked);
    // FIXME: update other MPerfWidgets, since this is a global setting
}

void MPerfWidget::on_indicBox_clicked(bool checked){
    perf->dpiIndicator(checked);
}

void MPerfWidget::on_aSnapBox_clicked(bool checked){
    perf->angleSnap(checked);
}

void MPerfWidget::on_lHeightBox_activated(int index){
    int height = index + 1;

    // Add 2 to the selected index, since the dark cores lift height ranges from 3 to 5, not 1 to 5
    if (isDarkCore)
        height += 2;

    perf->liftHeight((KbPerf::height)height);
}

void MPerfWidget::on_copyButton_clicked(){
    ModeSelectDialog dialog(this, profile->currentMode(), profile->modes(), tr("Copy performance settings to:"));
    if(dialog.exec() != QDialog::Accepted)
        return;
    QList<KbMode*> selectedModes = dialog.selection();
    foreach(KbMode* mode, selectedModes){
        *mode->perf() = *perf;
    }
}

void MPerfWidget::on_spinBox_valueChanged(int arg1){
    if(!perf)
        return;
    perf->iOpacity(arg1 / 100.f);
}

void MPerfWidget::highlightDpiBox(int index){
    for(int i = 0; i < DPI_COUNT; i++){
        stages[i].indicatorLabel->setVisible(false);
    }
    ui->iLabelO->setVisible(false);

    if(index > DPI_COUNT - 1){
        ui->iLabelO->setVisible(true);
        return;
    }

    stages[index].indicatorLabel->setVisible(true);
}

void MPerfWidget::setMaxDpi(ushort dpi){
    for(int i = 0; i < DPI_COUNT; i++){
        stages[i].xBox->setMaximum(dpi);
        stages[i].yBox->setMaximum(dpi);
        stages[i].xSlider->setMaximum(dpi);
        stages[i].ySlider->setMaximum(dpi);
    }
}
