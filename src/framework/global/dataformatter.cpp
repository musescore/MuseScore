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
#include "dataformatter.h"

#include <QString>
#include <QDateTime>
#include <QRegularExpression>

#include "translation.h"

using namespace mu;

double DataFormatter::roundDouble(const double& val, const int decimals)
{
    return QString::number(val, 'f', decimals).toDouble();
}

QString DataFormatter::formatReal(double val, int prec)
{
    return QString::number(val, 'f', prec);
}

QString DataFormatter::formatTimeSince(const QDate& dateTime)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    int days = dateTime.daysTo(currentDateTime.date());

    if (days == 0) {
        return qtrc("global", "Today");
    }

    if (days == 1) {
        return qtrc("global", "Yesterday");
    }

    if (days < 7) {
        return qtrc("global", "%1 days ago").arg(days);
    }

    int weeks = days / 7;

    if (weeks == 1) {
        return qtrc("global", "Last week");
    }

    if (weeks == 2) {
        return qtrc("global", "Two weeks ago");
    }

    if (weeks == 3) {
        return qtrc("global", "Three weeks ago");
    }

    if (weeks == 4) {
        return qtrc("global", "Four weeks ago");
    }

    QDate currentDate = currentDateTime.date();
    constexpr int monthInYear = 12;
    int months = (currentDate.year() - dateTime.year()) * monthInYear + (currentDate.month() - dateTime.month());

    if (months == 1) {
        return qtrc("global", "Last month");
    }

    if (months < monthInYear) {
        return qtrc("global", "%1 months ago").arg(months);
    }

    int years = currentDate.year() - dateTime.year();
    if (years == 1) {
        return qtrc("global", "1 year ago");
    }

    return qtrc("global", "%1 years ago").arg(years);
}
