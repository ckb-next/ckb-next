#ifndef DAEMONDIALOG_H
#define DAEMONDIALOG_H

#include <QDialog>

namespace Ui {
class DaemonDialog;
}

class DaemonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DaemonDialog(QWidget *parent = 0, QString cmdMsgStart = "", QString cmdMsgEnable = "");
    ~DaemonDialog();

private:
    Ui::DaemonDialog *ui;
};

#endif // DAEMONDIALOG_H
