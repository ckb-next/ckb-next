#include <QClipboard>
#include <QLabel>
#include <QScreen>
#include <QStyle>
#include <QTextDocument>

#include "daemonwarndialog.h"
#include "ui_daemonwarndialog.h"

DaemonWarnDialog::DaemonWarnDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DaemonWarnDialog)
{
    ui->setupUi(this);

    // Release resources on exit.
    this->setAttribute(Qt::WA_DeleteOnClose);

    // Display Critical Message Box icon.
    QStyle *style = this->style();
    int imageSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, this);
    QIcon icon = style->standardIcon(QStyle::SP_MessageBoxCritical, nullptr, this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    ui->iconLabel->setPixmap(icon.pixmap(QSize(imageSize, imageSize), this->devicePixelRatio()));
#else // QT_VERSION < 6.0.0
    ui->iconLabel->setPixmap(icon.pixmap(QSize(imageSize, imageSize)));
#endif

    // Display icon instead of name on copy buttons.
    QIcon fallbackIcon = style->standardIcon(QStyle::SP_FileIcon, nullptr, this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    icon = QIcon::fromTheme(QIcon::ThemeIcon::EditCopy, fallbackIcon);
#else // QT_VERSION < 6.7.0
    icon = QIcon::fromTheme("edit-copy", fallbackIcon);
#endif
    ui->pushButton->setIcon(icon);
    ui->pushButton_2->setIcon(icon);
    ui->pushButton_3->setIcon(icon);

#ifdef Q_OS_MACOS
    // On Mac only one command is displayed. Hide the remaining elements.
    ui->label_4->hide();
    ui->label_5->hide();
    ui->label_6->hide();
    ui->label_7->hide();
    ui->pushButton_2->hide();
    ui->pushButton_3->hide();

    ui->label_2->setText(tr("Start and enable it with:"));
    ui->label_3->setText("<code>sudo launchctl load -w /Library/LaunchDaemons/org.ckb-next.daemon.plist</code>");
#endif // Q_OS_MACOS

    // Call copyText() with the appropriate label, when the copy buttons are
    // pressed.
    QObject::connect(ui->pushButton, &QPushButton::clicked, this, [=] () {
        this->copyText(ui->label_3);
    });
    QObject::connect(ui->pushButton_2, &QPushButton::clicked, this, [=] () {
        this->copyText(ui->label_5);
    });
    QObject::connect(ui->pushButton_3, &QPushButton::clicked, this, [=] () {
        this->copyText(ui->label_7);
    });

    ui->gridLayout->activate();
    this->setFixedSize(ui->gridLayoutWidget->geometry().size());
}

DaemonWarnDialog::~DaemonWarnDialog() {
    delete ui;
}

void
DaemonWarnDialog::copyText(QLabel *label)
{
    QClipboard *clipboard = QApplication::clipboard();

    // Strip HTML tags when setting the clipboard content.
    QTextDocument doc;
    doc.setHtml(label->text());
    clipboard->setText(doc.toPlainText());
}
