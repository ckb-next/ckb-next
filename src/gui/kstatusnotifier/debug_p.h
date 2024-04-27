#ifndef DEBUG_P_H
#define DEBUG_P_H

#include <QLoggingCategory>

//Q_DECLARE_LOGGING_CATEGORY(LOG_KSTATUSNOTIFIERITEM)
#define LOG_KSTATUSNOTIFIERITEM
#undef qCDebug
#undef qCWarning
#define qCDebug qDebug
#define qCWarning qWarning


#endif // DEBUG_P_H
