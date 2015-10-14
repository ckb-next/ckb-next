#ifndef BINDDIALOG_H
#define BINDDIALOG_H

#include <QWidget>
#include "kbbind.h"
#include "kbprofile.h"

// Key rebinding widget

namespace Ui {
class RebindWidget;
}

class RebindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RebindWidget(QWidget* parent);
    ~RebindWidget();

    // Sets current binding setup
    void setBind(KbBind* newBind, KbProfile* newProfile);
    // Sets current selection (chooses keys to rebind)
    void setSelection(const QStringList& newSelection, bool applyPrevious = false);

private slots:
    void applyChanges(const QStringList& keys, bool doUnbind);
    void on_applyButton_clicked();
    void on_cancelButton_clicked();
    void on_resetButton_clicked();
    void on_unbindButton_clicked();

    void on_typingBox_currentIndexChanged(int index);
    void on_modBox_currentIndexChanged(int index);
    void on_fnBox_currentIndexChanged(int index);
    void on_numBox_currentIndexChanged(int index);
    void on_mediaBox_currentIndexChanged(int index);
    void on_mbBox_currentIndexChanged(int index);
    void on_mb2Box_currentIndexChanged(int index);
    void on_wheelBox_currentIndexChanged(int index);
    void on_dpiBox_currentIndexChanged(int index);
    void on_modeBox_currentIndexChanged(int index);
    void on_lightBox_currentIndexChanged(int index);
    void on_lockBox_currentIndexChanged(int index);
    void on_programKpBox_textChanged(const QString &arg1);
    void on_programKrBox_textChanged(const QString &arg1);
    void on_animBox_currentIndexChanged(int index);

    void on_typingButton_clicked(bool checked);
    void on_modButton_clicked(bool checked);
    void on_fnButton_clicked(bool checked);
    void on_numButton_clicked(bool checked);
    void on_mediaButton_clicked(bool checked);
    void on_mbButton_clicked(bool checked);
    void on_mb2Button_clicked(bool checked);
    void on_wheelButton_clicked(bool checked);
    void on_dpiButton_clicked(bool checked);
    void on_modeButton_clicked(bool checked);
    void on_lightButton_clicked(bool checked);
    void on_lockButton_clicked(bool checked);
    void on_programKpButton_clicked(bool checked);
    void on_programKrButton_clicked(bool checked);
    void on_programKpSIBox_clicked(bool checked);
    void on_programKrSIBox_clicked(bool checked);
    void on_animButton_clicked(bool checked);

private:
    Ui::RebindWidget *ui;

    // Tab indices
    const static int TAB_KB = 0, TAB_MOUSE = 1, TAB_ANIM = 2, TAB_SPECIAL = 3, TAB_PROGRAM = 4;

    void setBox(QWidget* box);

    KbBind* bind;
    KbProfile* profile;
    QStringList selection;

    QStringList typingKeys;
    QStringList modKeys;
    QStringList fnKeys;
    QStringList numKeys;
    QStringList mediaKeys;
    QStringList mouseKeys;
    QStringList mouseExtKeys;
    QStringList wheelKeys;
};

#endif // BINDDIALOG_H
