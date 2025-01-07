/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "doubleinputvalidator.h"
#include "global/realfn.h"

using namespace muse::uicomponents;

DoubleInputValidator::DoubleInputValidator(QObject* parent)
    : QValidator(parent)
{
}

void DoubleInputValidator::fixup(QString& string) const
{
    auto removeTrailingZeros = [](QString& str) {
        if (str.isEmpty()) {
            return;
        }

        if (!str.contains('.')) {
            return;
        }

        size_t num = str.size();
        for (size_t i = num - 1; i > 0; i--) {
            if (str[i] == '0') {
                str.remove(i, 1);
            } else if (str[i] == '.') {
                str.remove(i, 1);
                break;
            } else {
                break;
            }
        }
    };

    if (string.isEmpty()) {
        string.append("0");
    }

    if (string.startsWith(".")) {
        string.prepend("0");
    }

    if (string.endsWith(".")) {
        string.remove(string.size() - 1, 1);
    }

    QStringList strList = string.split(".", Qt::SkipEmptyParts);

    QString intPart = strList.at(0);
    QString floatPart = strList.size() > 1 ? strList.at(1) : 0;

    if (intPart.contains(QRegularExpression("^0{1,3}$"))) {
        intPart = QString("0");
    } else if (intPart.contains(QRegularExpression("^\\-0{0,3}$"))) {
        intPart = QString("-0");
    }

    if (intPart == QString("-0") && floatPart.isEmpty()) {
        intPart = QString("0");
    }

    if (floatPart.size() > m_decimal) {
        floatPart = floatPart.remove(m_decimal, floatPart.size() - m_decimal);
    }

    string = QString("%1.%2").arg(intPart).arg(floatPart);

    if (string.toDouble() > m_top) {
        string = QString::number(m_top, 'f', m_decimal);
    } else if (string.toDouble() < m_bottom) {
        string = QString::number(m_bottom, 'f', m_decimal);
    }

    removeTrailingZeros(string);
}

QValidator::State DoubleInputValidator::validate(QString& inputStr, int& cursorPos) const
{
    QValidator::State state = Invalid;

    const QRegularExpression validRegex(QString("^\\-?\\d{1,3}(\\.\\d{1,%1})?$").arg(m_decimal));

    if (inputStr.contains(validRegex)) {
        static const QRegularExpression invalidZeroRegex("^\\-?0{2,3}\\."); // for '-000.'
        static const QRegularExpression invalidTrailingZeroRegex("^\\-?\\d+\\.0{1,}$"); // for '1.00'
        static const QRegularExpression invalidTrailingDotRegex("^\\-?\\d+\\.$"); // for '1.'

        if (inputStr.contains(invalidZeroRegex)
            || (inputStr.startsWith("-") && muse::RealIsNull(inputStr.toDouble())) // for "-000"
            || inputStr.contains(invalidTrailingZeroRegex)
            || inputStr.contains(invalidTrailingDotRegex)) {
            state = Intermediate;
        } else {
            state = Acceptable;
        }
    } else if (inputStr.contains(QRegularExpression("^\\-?\\d{0,3}\\.?$"))
               || inputStr.contains(QRegularExpression(QString("^\\-?\\d{0,3}\\.\\d{0,%1}$").arg(m_decimal)))) {
        state = Intermediate;
    } else {
        cursorPos = 0;
        return Invalid;
    }

    if (inputStr.toDouble() > m_top || inputStr.toDouble() < m_bottom) {
        state = Intermediate;
    }

    return state;
}

qreal DoubleInputValidator::top() const
{
    return m_top;
}

qreal DoubleInputValidator::bottom() const
{
    return m_bottom;
}

int DoubleInputValidator::decimal() const
{
    return m_decimal;
}

void DoubleInputValidator::setTop(qreal top)
{
    m_top = top;
}

void DoubleInputValidator::setBottom(qreal bottom)
{
    m_bottom = bottom;
}

void DoubleInputValidator::setDecimal(int decimal)
{
    m_decimal = decimal;
}
