/* This file is part of the dbusmenu-qt library
   SPDX-FileCopyrightText: 2010 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "utils_p.h"

// Qt
#include <QString>

QString swapMnemonicChar(const QString &in, const QChar &src, const QChar &dst)
{
    QString out;
    bool mnemonicFound = false;

    for (int pos = 0; pos < in.length();) {
        QChar ch = in[pos];
        if (ch == src) {
            if (pos == in.length() - 1) {
                // 'src' at the end of string, skip it
                ++pos;
            } else {
                if (in[pos + 1] == src) {
                    // A real 'src'
                    out += src;
                    pos += 2;
                } else if (!mnemonicFound) {
                    // We found the mnemonic
                    mnemonicFound = true;
                    out += dst;
                    ++pos;
                } else {
                    // We already have a mnemonic, just skip the char
                    ++pos;
                }
            }
        } else if (ch == dst) {
            // Escape 'dst'
            out += dst;
            out += dst;
            ++pos;
        } else {
            out += ch;
            ++pos;
        }
    }

    return out;
}
