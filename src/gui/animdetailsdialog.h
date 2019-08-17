#ifndef ANIMDETAILSDIALOG_H
#define ANIMDETAILSDIALOG_H

#include <QDialog>
#include "animscript.h"

namespace Ui {
class AnimDetailsDialog;
}

class AnimDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnimDetailsDialog(QWidget *parent = nullptr);
    ~AnimDetailsDialog();

private:
    Ui::AnimDetailsDialog *ui;
};

#endif // ANIMDETAILSDIALOG_H
