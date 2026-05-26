#ifndef DAEMONWARNDIALOG_H
#define DAEMONWARNDIALOG_H

#include <QDialog>
#include <QLabel>

namespace Ui {
class DaemonWarnDialog;
}

class DaemonWarnDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DaemonWarnDialog(QWidget *parent);
    ~DaemonWarnDialog();

private slots:
    void copyText(QLabel *label);

private:
    Ui::DaemonWarnDialog *ui;
};

#endif // DAEMONWARNDIALOG_H
