#include <QDir>
#include <QMessageBox>
#include "fwupgradedialog.h"
#include "kbfirmware.h"
#include "ui_fwupgradedialog.h"

// IDs for verifying firmware suitability
struct KbId {
    ushort vendor;
    ushort product;
    const char* feature;
};

static const int DIALOG_WIDTH = 420;
static const int DIALOG_HEIGHT_MIN = 200, DIALOG_HEIGHT_MAX(240);

FwUpgradeDialog::FwUpgradeDialog(QWidget* parent, float newV, const QByteArray& fwBlob, Kb* device) :
    QDialog(parent),
    ui(new Ui::FwUpgradeDialog),
    blob(fwBlob), kb(device), evLoop(0), exitSuccess(true)
{
    ui->setupUi(this);
    ui->curLabel->setText(kb->firmware);
    ui->newLabel->setText(QString::number(newV, 'f', 2));
    ui->devLabel->setText(kb->usbModel);

    connect(device, SIGNAL(destroyed()), this, SLOT(removeDev()));
    connect(device, SIGNAL(fwUpdateProgress(int,int)), this, SLOT(fwUpdateProgress(int,int)));
    connect(device, SIGNAL(fwUpdateFinished(bool)), this, SLOT(fwUpdateFinished(bool)));

    setFixedSize(DIALOG_WIDTH, DIALOG_HEIGHT_MIN);
}

FwUpgradeDialog::~FwUpgradeDialog(){
    cleanBlob();
    delete ui;
}

void FwUpgradeDialog::closeEvent(QCloseEvent* event){
    event->ignore();
}

const QString& FwUpgradeDialog::saveBlob(){
    if(!savePath.isEmpty())
        return savePath;
    QDir tmp = QDir::temp();
    qint64 pid = QCoreApplication::applicationPid();
    QString path = tmp.absoluteFilePath(QString("ckb-%1-fwblob.bin").arg(pid));
    QFile output(path);
    if(!output.open(QIODevice::WriteOnly)){
        return savePath;
    }
    if(!output.write(blob)){
        output.close();
        tmp.remove(path);
        return savePath;
    }
    output.close();
    savePath = path;
    return savePath;
}

void FwUpgradeDialog::cleanBlob(){
    if(savePath.isEmpty())
        return;
    QFile(savePath).remove();
    savePath = "";
}

// Returns firmware version if valid for device, 0 if invalid
static float verifyFw(const QByteArray& blob, const ushort productID) {
    ushort vendor = 0x1b1c;      ///< Corsair has at the moment just one vendorID
    if(blob.length() < 0x0108)
        return 0.f;
    const char* bData = blob.data();
    // Make sure it matches this device based on the vendor and product IDs embedded in the blob

    if (memcmp(&vendor, bData + 0x102, 2) || memcmp(&productID, bData + 0x104, 2)) {
        qCritical() << "Something really bad happened - wrong firmware file detected";
        return 0.f; ///< Something really bad has happened - wrong firmware file detected.
    }

    // Copy the version from the blob
    short version;
    memcpy(&version, bData + 0x106, 2);
    // Un-hexify it
    return QString::number(version, 16).toFloat() / 100.f;
}

int FwUpgradeDialog::exec(){
    ushort productID = kb->productID;

    if(!blob.isEmpty()){
        // If a blob was already specified, check its version and validity
        float newV = verifyFw(blob, productID);
        if(newV == 0.f){
            QMessageBox::warning(parentWidget(), tr("Error"), tr("<center>Not a valid firmware for this device.</center>"));
            return QDialog::Rejected;
        }
        ui->newLabel->setText(QString::number(newV, 'f', 2));
    } else {
        // Download a new blob file
        ui->progressBar->show();
        ui->cancelButton->setEnabled(false);
        ui->actionButton->setEnabled(false);
        show();
        // This can take a while
        blob = KbFirmware::dataForBoard(productID);
        // Check validity
        float newV = verifyFw(blob, productID);
        if(newV == 0.f){
            hide();
            QMessageBox::warning(parentWidget(), tr("Error"), tr("<center>There was a problem with the downloaded file.<br />Please try again later.</center>"));
            return QDialog::Rejected;
        }
    }
    // Save temporary file
    if(saveBlob().isEmpty()){
        hide();
        QMessageBox::warning(parentWidget(), tr("Error"), tr("<center>Unable to save temporary file.</center>"));
        return QDialog::Rejected;
    }
    // Set up UI
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setTextVisible(false);
    ui->statusLabel->setText(tr("Ready to install new firmware.<br /><br /><b>Disclaimer:</b> ckb-next is not endorsed by Corsair.<br />This is <i>unlikely</i> to cause any damage, however the developers of this software do not accept any responsibility in such an event."));
    ui->cancelButton->setEnabled(true);
    ui->actionButton->setEnabled(true);
    setFixedSize(DIALOG_WIDTH, DIALOG_HEIGHT_MAX);
    show();
    // Run modal event loop
    evLoop = new QEventLoop(this);
    evLoop->exec();
    delete evLoop;
    hide();
    return exitSuccess ? QDialog::Accepted : QDialog::Rejected;
}

void FwUpgradeDialog::removeDev(){
    kb = 0;
    // Assume success if upgrade in progress
    if(!savePath.isEmpty())
        fwUpdateFinished(true);
}

void FwUpgradeDialog::fwUpdateProgress(int current, int total){
    if(current > 0 && total > 0){
        ui->progressBar->setMaximum(total);
        ui->progressBar->setValue(current);
    }
}

void FwUpgradeDialog::fwUpdateFinished(bool succeeded){
    cleanBlob();
    if(succeeded)
        ui->statusLabel->setText(tr("Update successful!"));
    else
        ui->statusLabel->setText(tr("Update failed."));
    ui->actionButton->setText(tr("OK"));
    ui->actionButton->setEnabled(true);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);
    // Exit after 10s
    if(evLoop)
        QTimer::singleShot(10000, evLoop, SLOT(quit()));
}

void FwUpgradeDialog::on_cancelButton_clicked(){
    exitSuccess = false;
    if(evLoop)
        evLoop->quit();
}

void FwUpgradeDialog::on_actionButton_clicked(){
    if(!savePath.isEmpty() && kb){
        // Start upgrade
        setFixedSize(DIALOG_WIDTH, DIALOG_HEIGHT_MIN);
        ui->progressBar->show();
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(0);
        ui->progressBar->setTextVisible(true);
        ui->cancelButton->hide();
        ui->actionButton->setEnabled(false);
        ui->actionButton->setText(tr("Please wait"));
        ui->statusLabel->setText(tr("Installing firmware..."));
        kb->fwUpdate(savePath);
    } else {
        // Finished, close dialog.
        if(evLoop)
            evLoop->quit();
    }
}
