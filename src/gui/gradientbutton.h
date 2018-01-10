#ifndef GRADIENTBUTTON_H
#define GRADIENTBUTTON_H

#include <QPushButton>

class GradientButton : public QPushButton
{
    Q_OBJECT
public:
    explicit GradientButton(QWidget* parent = 0, bool allowAlpha = false);

    void    fromString(const QString& string);
    QString toString() const;

signals:
    void gradientChanged();

private slots:
    void pickGradient();

private:
    QGradientStops _stops;
    bool _alpha;

    void updateImage();
};

#endif // GRADIENTBUTTON_H
