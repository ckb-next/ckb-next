#ifndef MODESELECTDIALOG_H
#define MODESELECTDIALOG_H

#include <QDialog>
#include "kbmode.h"

// Dialog box that allows selection of modes

namespace Ui {
class ModeSelectDialog;
}

class ModeSelectDialog : public QDialog
{
    Q_OBJECT

public:
    // Mode dialog with a list of modes and a custom text label. Use exec() to display. currentMode will not be displayed.
    ModeSelectDialog(QWidget* parent, KbMode* currentMode, QList<KbMode*> modeList, const QString& textLabel);
    ~ModeSelectDialog();

    // Mode(s) selected by the user
    QList<KbMode*> selection();

private slots:
    void on_selAllButton_clicked();
    void on_selNoneButton_clicked();

private:
    Ui::ModeSelectDialog *ui;
    QList<KbMode*> _modeList;
};

#endif // MODESELECTDIALOG_H
