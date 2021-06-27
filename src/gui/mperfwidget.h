#ifndef MPERFWIDGET_H
#define MPERFWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include "kbperf.h"
#include "kbprofile.h"
#include "colorbutton.h"

namespace Ui {
class MPerfWidget;
}

class MPerfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MPerfWidget(QWidget *parent = 0);
    ~MPerfWidget();

    void setPerf(KbPerf* newPerf, KbProfile* newProfile);

    static const int DPI_COUNT = KbPerf::DPI_COUNT;
    void setLegacyM95();
    void setDarkCore();
    void setMaxDpi(ushort dpi);
private:
    Ui::MPerfWidget *ui;

    KbPerf* perf;
    KbProfile* profile;

    struct DpiUi {
        ColorButton* indicator;
        QSlider* xSlider, *ySlider;
        QSpinBox* xBox, *yBox;
        QCheckBox* enableCheck;
        QLabel* indicatorLabel;
    };
    DpiUi stages[DPI_COUNT];
    bool _xyLink;
    bool colorLink;

    // Hack: prevent recursive slot calls
    bool isSetting;

    bool isDarkCore;

    void hideItemsInLayout(QLayout* layout);

private slots:
    void colorClicked(int index);
    void colorChanged(int index);
    void sliderXValueChanged(int index);
    void sliderXMoved(int index);
    void sliderYValueChanged(int index);
    void sliderYMoved(int index);
    void boxXChanged(int index);
    void boxYChanged(int index);
    void enableChanged(int index);
    void on_xyBox_clicked(bool checked);
    void on_indicBox_clicked(bool checked);
    void on_aSnapBox_clicked(bool checked);
    void on_lHeightBox_activated(int index);
    void on_copyButton_clicked();
    void on_spinBox_valueChanged(int arg1);
    void highlightDpiBox(int index);
};

#endif // MPERFWIDGET_H
