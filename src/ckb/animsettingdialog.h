#ifndef ANIMSETTINGDIALOG_H
#define ANIMSETTINGDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QFrame>
#include "kbanim.h"

namespace Ui {
class AnimSettingDialog;
}

class AnimSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnimSettingDialog(QWidget* parent, KbAnim* anim);
    ~AnimSettingDialog();

    void applySettings();

private:
    Ui::AnimSettingDialog *ui;

    KbAnim* _anim;
    QCheckBox* repCheck;
    QCheckBox* kpRepCheck;
    double lastDuration;
    QMap<QString, QWidget*> settingWidgets;

    QFrame* hLine();

private slots:
    void newDuration(double duration);
};

#endif // ANIMSETTINGDIALOG_H
