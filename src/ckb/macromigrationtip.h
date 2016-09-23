#ifndef MACROMIGRATIONTIP_H
#define MACROMIGRATIONTIP_H

#include <QDialog>

namespace Ui {
class macroMigrationTip;
}

class macroMigrationTip : public QDialog
{
    Q_OBJECT

public:
    explicit macroMigrationTip(QWidget *parent = 0);
    ~macroMigrationTip();

private:
    Ui::macroMigrationTip *ui;
};

#endif // MACROMIGRATIONTIP_H
