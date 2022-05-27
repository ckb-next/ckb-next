#ifndef KBPROFILEDIALOG_H
#define KBPROFILEDIALOG_H

#include <QDialog>
#include "kbwidget.h"
#include <QListWidgetItem>

namespace Ui {
class KbProfileDialog;
}

class KbProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KbProfileDialog(KbWidget* parent = nullptr);
    ~KbProfileDialog();

private slots:
    void profileList_reordered();
    void on_profileList_itemClicked(QListWidgetItem* item);
    void on_profileList_itemChanged(QListWidgetItem* item);
    void on_profileList_customContextMenuRequested(const QPoint &pos);
    void on_exportButton_clicked();
    void on_importButton_clicked();
    void on_profileList_itemSelectionChanged();

private:
    Ui::KbProfileDialog *ui;

    Kb* device;
    KbProfile* activeProfile;
    const static int GUID = Qt::UserRole;
    const static int NEW_FLAG = Qt::UserRole + 1;

    void repopulate();
    void addNewProfileItem();

    bool verifyHash(const QString& file);
    void importCleanup(const QStringList& extracted, const QList<QPair<CkbExternalSettings*, QString> >& profileptrs);
    void extractedFileCleanup(const QStringList& extracted);

};

#endif // KBPROFILEDIALOG_H
