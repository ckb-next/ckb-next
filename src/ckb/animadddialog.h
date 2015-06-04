#ifndef ANIMADDDIALOG_H
#define ANIMADDDIALOG_H

#include <QDialog>
#include "animscript.h"
#include "kblight.h"

namespace Ui {
class AnimAddDialog;
}

class AnimAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnimAddDialog(QWidget *parent, KbLight* light, const QStringList& keys);
    ~AnimAddDialog();

    const AnimScript*   chosenScript();
    int                 chosenPreset();

private slots:
    void on_animBox_activated(int index);
    void on_presetBox_activated(int index);
    void on_previewBox_clicked(bool checked);

private:
    Ui::AnimAddDialog *ui;
    QList<const AnimScript*> scripts;

    KbLight* _light;
    QStringList _keys;
    bool showPreview;
};

#endif // ANIMADDDIALOG_H
