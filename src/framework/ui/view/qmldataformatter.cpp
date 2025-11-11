/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "qmldataformatter.h"

#include "global/dataformatter.h"

using namespace muse::ui;

QmlDataFormatter::QmlDataFormatter(QObject* parent)
    : QObject{parent}
{
}

QString QmlDataFormatter::formatReal(double value, int decimals) const
{
    QLocale locale;
    QString formatted = locale.toString(value, 'f', decimals);
    if (decimals > 0) {
        // Remove trailing zeros after the decimal separator
        QString decSepStr = locale.decimalPoint();
        QChar decSep = decSepStr.isEmpty() ? QChar('.') : decSepStr.at(0);
        int decPos = formatted.indexOf(decSep);
        if (decPos != -1) {
            int last = formatted.length() - 1;
            // Remove trailing zeros
            while (last > decPos && formatted[last] == '0') {
                --last;
            }
            // Remove trailing decimal separator if needed
            if (last == decPos) {
                --last;
            }
            formatted = formatted.left(last + 1);
        }
    }
    return formatted;
}
