#ifndef NOUSERINPUTLINEEDIT_H
#define NOUSERINPUTLINEEDIT_H

#include <QObject>
#include <QPlainTextEdit>

class NoUserInputTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    NoUserInputTextEdit(QWidget* parent);
    inline void eatKeyEvents(bool e) { redirectKeyEvents = e; }
signals:
    void macroKeyEvent(int keycode, bool keydown, Qt::KeyboardModifiers modifiers);
private:
    bool eventFilter(QObject* obj, QEvent* evt);
    bool redirectKeyEvents;
};

#endif // NOUSERINPUTLINEEDIT_H
