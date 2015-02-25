#ifndef BINDDIALOG_H
#define BINDDIALOG_H

#include <QWidget>
#include "kbbind.h"
#include "kbprofile.h"

namespace Ui {
class RebindWidget;
}

class RebindWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RebindWidget(QWidget* parent);
    ~RebindWidget();

    void setBind(KbBind* newBind, KbProfile* newProfile);
    void setSelection(const QStringList& newSelection, bool applyPrevious = false);

private slots:
    void applyChanges(const QStringList& keys, bool doUnbind);
    void on_applyButton_clicked();
    void on_cancelButton_clicked();

    void on_typingBox_currentIndexChanged(int index);
    void on_modBox_currentIndexChanged(int index);
    void on_fnBox_currentIndexChanged(int index);
    void on_numBox_currentIndexChanged(int index);
    void on_mediaBox_currentIndexChanged(int index);
    void on_modeBox_currentIndexChanged(int index);
    void on_lightBox_currentIndexChanged(int index);
    void on_lockBox_currentIndexChanged(int index);
    void on_programKpBox_textChanged(const QString &arg1);
    void on_programKrBox_textChanged(const QString &arg1);

    void on_typingButton_clicked(bool checked);
    void on_modButton_clicked(bool checked);
    void on_fnButton_clicked(bool checked);
    void on_numButton_clicked(bool checked);
    void on_mediaButton_clicked(bool checked);
    void on_modeButton_clicked(bool checked);
    void on_lightButton_clicked(bool checked);
    void on_lockButton_clicked(bool checked);
    void on_programKpButton_clicked(bool checked);
    void on_programKrButton_clicked(bool checked);

private:
    Ui::RebindWidget *ui;

    void setBox(QWidget* box);

    KbBind* bind;
    KbProfile* profile;
    QStringList selection;

    QStringList typingKeys;
    QStringList modKeys;
    QStringList fnKeys;
    QStringList numKeys;
    QStringList mediaKeys;
};

#endif // BINDDIALOG_H
