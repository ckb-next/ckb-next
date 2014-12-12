#ifndef KBWIDGET_H
#define KBWIDGET_H

#include <QFile>
#include <QListWidgetItem>
#include <QWidget>
#include "kbprofile.h"

namespace Ui {
class KbWidget;
}

class KbWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KbWidget(QWidget *parent, const QString& path, const QString& prefsBase);
    ~KbWidget();

    QString devpath, cmdpath, notifypath;
    QString serial;
    QString model;
    QString firmware;

    bool disconnect;

    void save();

public slots:
    void frameUpdate();

private slots:
    void on_layoutBox_currentIndexChanged(int index);
    void on_tabWidget_currentChanged(int index);

    void on_profileBox_currentIndexChanged(int index);
    void on_profilesList_currentRowChanged(int currentRow);
    void on_profilesList_itemChanged(QListWidgetItem *item);
    void on_profilesList_customContextMenuRequested(const QPoint &pos);

    void on_modeBox_currentIndexChanged(int index);
    void on_modesList_currentRowChanged(int currentRow);
    void on_modesList_itemChanged(QListWidgetItem *item);
    void on_modesList_customContextMenuRequested(const QPoint &pos);

    void on_modeUpButton_clicked();
    void on_modeDownButton_clicked();
    void on_newModeButton_clicked();

    void on_profileUpButton_clicked();
    void on_profileDownButton_clicked();
    void on_newProfileButton_clicked();

    void on_hwSaveButton_clicked();

    void on_showModes_toggled(bool checked);
    void on_showProfiles_toggled(bool checked);
    void on_showModes_2_toggled(bool checked);
    void on_showProfiles_2_toggled(bool checked);

    void on_inactiveSwitchBox_activated(int index);
    void on_inactiveSwitchCheck_clicked(bool checked);
    void on_muteCheck_clicked(bool checked);

private:
    QString prefsPath;

    int notifyNumber;

    bool layoutLoaded;
    bool hwLoading;
    bool queueProfileSwitch;
    int hwModeCount;
    KbProfile* hwProfile;
    KbProfile* currentProfile;
    QList<KbProfile*> profiles;
    int hwModes[3];
    inline int currentMode() { return currentProfile->currentMode(); }

    void setProfileName(KbProfile* profile, const QString& name);
    void setModeName(KbProfile* profile, int mode, const QString& name);

    KbLight* prevLight;
    inline KbLight* currentLight() { return !currentProfile ? 0 : currentProfile->modeLight(currentMode()); }
    void updateUI();

    void addProfile(KbProfile* profile);
    inline void addProfile() { addProfile(new KbProfile(this, getKeyMap(), QUuid::createUuid().toString(), "0")); }
    void duplicateProfile(KbProfile* profile);
    void deleteProfile(KbProfile* profile);

    void addMode(KbProfile* profile);
    void duplicateMode(KbProfile* profile, int mode);
    void deleteMode(KbProfile* profile, int mode);

    void setCurrentProfile(KbProfile* profile);
    void setCurrentMode(KbProfile* profile, int mode);

    KeyMap getKeyMap();

    void getCmd(QFile& file);
    void readInput(QFile& cmd);
    void switchProfile(QFile& cmd);

    Ui::KbWidget *ui;
};

#endif // KBWIDGET_H
