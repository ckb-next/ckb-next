#ifndef CKBUPDATERWIDGET_H
#define CKBUPDATERWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QNetworkAccessManager>

namespace Ui {
class CkbUpdaterWidget;
}

class CkbUpdaterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CkbUpdaterDialog(const QString& ver,  const QString& changelog, QWidget *parent = nullptr);
    ~CkbUpdaterDialog();

private slots:
    void on_remindMeLaterButton_clicked();
    void on_updateButton_clicked();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(QNetworkReply* reply);

private:
    Ui::CkbUpdaterWidget *ui;
    QString _version, _changelog;
    QNetworkAccessManager* _manager;
    QNetworkReply* _reply;
    int _redirectCount;
    QString _url;
    bool _quitApp;
};

#endif // CKBUPDATERWIDGET_H
