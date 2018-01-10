#ifndef FWUPGRADEDIALOG_H
#define FWUPGRADEDIALOG_H

#include <QCloseEvent>
#include <QDialog>
#include <QEventLoop>
#include "kb.h"

namespace Ui {
class FwUpgradeDialog;
}

class FwUpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    FwUpgradeDialog(QWidget* parent, float newV, const QByteArray& fwBlob, Kb* device);
    ~FwUpgradeDialog();

    int exec();

private slots:
    void fwUpdateProgress(int current, int total);
    void fwUpdateFinished(bool succeeded);
    void removeDev();

    void on_cancelButton_clicked();
    void on_actionButton_clicked();

private:
    Ui::FwUpgradeDialog *ui;
    void closeEvent(QCloseEvent* event);

    // FW blob has to be saved to a temporary path for processing.
    // saveBlob() to save, cleanBlob() to remove.
    QString         savePath;
    const QString&  saveBlob();
    void            cleanBlob();

    QByteArray  blob;
    Kb*         kb;

    // Event loop for synchronous exec()
    QEventLoop* evLoop;
    bool        exitSuccess;
};

#endif // FWUPGRADEDIALOG_H
