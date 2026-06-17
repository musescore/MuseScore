/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "intinputvalidator.h"
#include "global/realfn.h"
#include <qvalidator.h>

using namespace muse::uicomponents;

IntInputValidator::IntInputValidator(QObject* parent)
    : QValidator(parent)
{
}

void IntInputValidator::fixup(QString& string) const
{
    QLocale locale;

    if (string.isEmpty() || string.endsWith("-")) {
        string.append("0");
    }

    QString stripped = string;
    stripped.remove(locale.groupSeparator());

    bool ok = false;
    int val = stripped.toInt(&ok);

    if (!ok) {
        string = "0";
        return;
    }

    if (val > m_top) {
        val = m_top;
    } else if (val < m_bottom) {
        val = m_bottom;
    }

    string = locale.toString(val);
}

QValidator::State IntInputValidator::validate(QString& inputStr, int& cursorPos) const
{
    QLocale locale;
    QString groupSep = locale.groupSeparator();

    if (inputStr.contains(groupSep)) {
        bool ok = false;
        locale.toInt(inputStr, &ok);
        if (!ok) {
            // Handle group separator in the wrong place
            int sepsBefore = inputStr.left(cursorPos).count(groupSep);
            inputStr.remove(groupSep);
            cursorPos -= sepsBefore;
        }
    }

    QString digits = inputStr;
    digits.remove(groupSep);

    QValidator::State state = Invalid;

    const int maxAbsoluteValue = std::max(std::abs(m_top), std::abs(m_bottom));
    const int maxNumberOfDigits = maxAbsoluteValue > 0
                                  ? std::floor(std::log10(maxAbsoluteValue)) + 1
                                  : 1;
    if (digits.contains(QRegularExpression(QString("^\\-?\\d{1,%1}$").arg(maxNumberOfDigits)))) {
        if ((maxNumberOfDigits >= 2 && digits.contains(QRegularExpression(QString("^\\-?0{2,%1}").arg(maxNumberOfDigits))))
            || (digits.startsWith("-") && muse::RealIsNull(digits.toDouble()))) {
            state = Intermediate;
        } else {
            state = Acceptable;
        }
    } else if (digits.isEmpty()) {
        return Intermediate;
    } else if ((digits == "-")) {
        if (m_bottom < 0) {
            return Intermediate;
        }

        cursorPos = 0;
        return Invalid;
    }

    int val = digits.toInt();
    if (val > m_top) {
        // A negative value above the maximum may still be an in-progress prefix
        return val < 0 ? Intermediate : Invalid;
    }

    if (val < m_bottom) {
        // Symmetrically, a positive not 0 value below the minimum may still be an in-progress
        return val > 0 ? Intermediate : Invalid;
    }

    return state;
}

int IntInputValidator::top() const
{
    return m_top;
}

int IntInputValidator::bottom() const
{
    return m_bottom;
}

void IntInputValidator::setTop(int top)
{
    m_top = top;
}

void IntInputValidator::setBottom(int bottom)
{
    m_bottom = bottom;
}
