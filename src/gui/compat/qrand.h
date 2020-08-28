#ifndef QRANDCOMPAT_H
#define QRANDCOMPAT_H
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#define Q_SRAND(seed)
#include <QRandomGenerator>
#define Q_RAND() QRandomGenerator::global()->generate()
#else
#define Q_SRAND(seed) qsrand(seed)
#define Q_RAND() qrand()
#endif
#endif // QRANDCOMPAT_H
