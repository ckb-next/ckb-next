#ifndef LAYOUTDIALOG_H
#define LAYOUTDIALOG_H

#include <QDialog>
#include "keymap.h"

// Dialog for choosing a keyboard layout

namespace Ui {
class LayoutDialog;
}

class LayoutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayoutDialog(QWidget *parent = 0);
    ~LayoutDialog();

    KeyMap::Layout selected() const;

private:
    Ui::LayoutDialog *ui;
};

#endif // LAYOUTDIALOG_H
