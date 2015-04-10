#ifndef ANIMADDDIALOG_H
#define ANIMADDDIALOG_H

#include <QDialog>
#include "animscript.h"

namespace Ui {
class AnimAddDialog;
}

class AnimAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnimAddDialog(QWidget *parent = 0);
    ~AnimAddDialog();

    const AnimScript* chosenScript();
    int chosenPreset();

private slots:
    void on_animBox_activated(int index);

private:
    Ui::AnimAddDialog *ui;
    QList<const AnimScript*> scripts;

    bool previewActive;
};

#endif // ANIMADDDIALOG_H
