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

Q_SIGNALS:
    void gradientChanged();

private Q_SLOTS:
    void pickGradient();

private:
    QGradientStops _stops;
    bool _alpha;

    void updateImage();
};

#endif // GRADIENTBUTTON_H
