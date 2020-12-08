#ifndef WINDOWINFO_H
#define WINDOWINFO_H
#include <QString>
#include <QSettings>
#include "ckbsettings.h"
#include <QVector>

class KbMode;
class KbWindowInfo : public QObject
{
    Q_OBJECT
public:
    KbWindowInfo(KbMode* parent);
    KbWindowInfo(KbMode* parent, const KbWindowInfo& other);

    // Match data
    enum MatchFlag {
        MATCH_CASE_INSENSITIVE = 1,
        MATCH_SUBSTRING = 2,
        MATCH_STARTS_WITH = 4,
        MATCH_ENDS_WITH = 8,
    };
    Q_DECLARE_FLAGS(MatchFlags, MatchFlag)
    Q_FLAG(MatchFlags)

    enum MatchType {
        MATCH_TYPE_WINDOW_TITLE,
        MATCH_TYPE_PROGRAM_PATH,
        MATCH_TYPE_WM_INSTANCE_NAME,
        MATCH_TYPE_WM_CLASS_NAME,
    };

    enum MatchOperator {
        MATCH_OP_OR,
        MATCH_OP_AND,
    };

    struct MatchPair {
        MatchType type;
        QString item;
        MatchFlags flags;
        MatchOperator op;
    };

    inline bool needsSave() const { return _needsSave; }
    void load(CkbSettingsBase& settings);
    void save(CkbSettingsBase& settings);
    inline bool isEmpty() { return items.isEmpty(); }
    inline void setEnabled(const bool e) {
        if(enabled == e)
            return;
        _needsSave = true;
        enabled = e;
        emit enableStateChanged();
    }
    inline bool isEnabled() const {
        return enabled;
    }
    QVector<MatchPair> items;
signals:
    void enableStateChanged();
private:
    bool _needsSave;
    bool enabled;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KbWindowInfo::MatchFlags)

#endif // WINDOWINFO_H
