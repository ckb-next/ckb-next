#ifndef NOUSERINPUTLINEEDIT_H
#define NOUSERINPUTLINEEDIT_H

#include <QObject>
#include <QPlainTextEdit>

class NoUserInputTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    NoUserInputTextEdit(QWidget* parent);
    inline void eatKeyEvents(bool e) { redirectKeyEvents = e; }

    // Awful hack to make the RebindWidget not take up too much space
    QSize sizeHint() const override {
        QSize s = QPlainTextEdit::sizeHint();
        s.setHeight(40);
        return s;
    }
    QSize minimumSizeHint() const override {
        QSize s = QPlainTextEdit::minimumSizeHint();
        s.setHeight(40);
        return s;
    }
signals:
    void macroKeyEvent(int keycode, bool keydown, Qt::KeyboardModifiers modifiers);
private:
    bool eventFilter(QObject* obj, QEvent* evt) override;
    bool redirectKeyEvents;
};

#endif // NOUSERINPUTLINEEDIT_H
