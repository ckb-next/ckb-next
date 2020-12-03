#ifndef MACROSTRINGEDITDIALOG_H
#define MACROSTRINGEDITDIALOG_H

#include <QDialog>
#include "macrotablemodel.h"

namespace Ui {
class MacroStringEditDialog;
}

class MacroStringEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MacroStringEditDialog(MacroTableModel* mt, QWidget* parent = nullptr);
    ~MacroStringEditDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::MacroStringEditDialog* ui;
    MacroTableModel* m;
};

#endif // MACROSTRINGEDITDIALOG_H
