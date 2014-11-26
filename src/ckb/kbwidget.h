#ifndef KBWIDGET_H
#define KBWIDGET_H

#include <QFile>
#include <QListWidgetItem>
#include <QWidget>
#include "kblightwidget.h"

namespace Ui {
class KbWidget;
}

class KbWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KbWidget(QWidget *parent, const QString& path);
    ~KbWidget();

    QString devpath, cmdpath, notifypath;
    QString serial;
    QString model;
    QString firmware;

    QString lastProfileName;
    QStringList lastModeNames;

    bool disconnect;

    int notifyNumber;
    int modeCount;
    int currentMode;

public slots:
    void frameUpdate();

private slots:
    void on_layoutBox_currentIndexChanged(int index);

    void on_modeList_currentRowChanged(int currentRow);

    void on_profileText_editingFinished();

    void on_modeList_itemChanged(QListWidgetItem *item);

    void on_pushButton_clicked();

    void on_tabWidget_currentChanged(int index);

private:
    Ui::KbWidget *ui;

    void addMode();
    KeyMap getKeyMap();

    void getCmd(QFile& file);
    void readInput(QFile& cmd);
};

#endif // KBWIDGET_H
