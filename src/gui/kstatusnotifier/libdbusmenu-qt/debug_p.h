/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2009 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef DEBUG_P_H
#define DEBUG_P_H

#include <QDebug>

#define _DMBLUE "\033[34m"
#define _DMRED "\033[31m"
#define _DMRESET "\033[0m"
#define _DMTRACE(level, color) (level().nospace() << color << __PRETTY_FUNCTION__ << _DMRESET ":").space()

// Simple macros to get KDebug like support
#define DMDEBUG _DMTRACE(qDebug, _DMBLUE)
#define DMWARNING _DMTRACE(qWarning, _DMRED)

// Log a variable name and value
#define DMVAR(var) DMDEBUG << #var ":" << var

#define DMRETURN_IF_FAIL(cond)                                                                                                                                 \
    if (!(cond)) {                                                                                                                                             \
        DMWARNING << "Condition failed: " #cond;                                                                                                               \
        return;                                                                                                                                                \
    }

#define DMRETURN_VALUE_IF_FAIL(cond, value)                                                                                                                    \
    if (!(cond)) {                                                                                                                                             \
        DMWARNING << "Condition failed: " #cond;                                                                                                               \
        return (value);                                                                                                                                        \
    }

#endif /* DEBUG_P_H */
