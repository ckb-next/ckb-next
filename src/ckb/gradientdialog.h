#ifndef GRADIENTDIALOG_H
#define GRADIENTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QMap>

namespace Ui {
class GradientDialog;
}

class GradientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GradientDialog(QWidget *parent = 0);
    ~GradientDialog();

    QGradientStops getGradient(const QGradientStops& prevGradient);
private:
    // Preset gradients, sorted by lower case name
    struct Preset {
        QString         name;
        QGradientStops  gradient;
        bool            builtIn;
        inline Preset(QString _name, bool _builtIn = false)             : name(_name), builtIn(_builtIn) {}
        inline Preset(QString _name, const QGradientStops& _gradient)   : name(_name), gradient(_gradient), builtIn(false) {}
        inline Preset()                                                 : builtIn(false) {}
    };
    QMap<QString, Preset>   presets;
    QString                 currentPreset;

    inline void addPreset(const Preset& preset) { presets[preset.name.toLower()] = preset; }
    QIcon       makeIcon(const Preset& preset);
    void        updatePresets();
    void        setPreset(const QString& newPreset);

    Ui::GradientDialog *ui;

private slots:
    void currentChanged(QColor color, bool spontaneous, int position);
    void colorChanged(QColor color);

    void on_presetList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_presetName_textEdited(const QString &arg1);
    void on_presetSave_clicked();
    void on_presetDelete_clicked();
    void on_stopPos_valueChanged(int arg1);
    void on_stopOpacity_valueChanged(int arg1);
};

#endif // GRADIENTDIALOG_H
