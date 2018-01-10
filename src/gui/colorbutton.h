#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget* parent = 0, bool allowAlpha = false);

    // Current color
    inline const QColor&    color() const                   { return _color; }
    void                    color(const QColor& newColor);

    // Enable alpha channel?
    inline bool allowAlpha()  const             { return _alpha; }
    void        allowAlpha(bool newAllowAlpha);
    // Whether or not to set the label based on the current color
    inline bool setLabel()  const               { return _setLabel; }
    inline void setLabel(bool newSetLabel)      { _setLabel = newSetLabel; }
    // Whether to use big icons (24x24) or small icons (24x12)
    inline bool bigIcons()  const               { return _bigIcons; }
    void        bigIcons(bool newBigIcons);

signals:
    void colorChanged(QColor newColor);

private slots:
    void pickColor();

private:
    QColor _color;
    bool _alpha, _setLabel, _bigIcons;

    void updateImage();
};

#endif // COLORBUTTON_H
