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

#include "translation.h"

using namespace mu;

double DataFormatter::roundDouble(const double& val, const int decimals)
{
    return String::number(val, decimals).toDouble();
}

String DataFormatter::formatReal(double val, int prec)
{
    return String::number(val, prec);
}

String DataFormatter::formatTimeSince(const Date& dateTime)
{
    DateTime currentDateTime = DateTime::currentDateTime();
    int days = dateTime.daysTo(currentDateTime.date());

    if (days == 0) {
        return mtrc("global", "Today");
    }

    if (days >= 1 && days < 7) {
        //: Omit the "%n" for the singular translation and translate to (the equivalent of) "Yesterday"
        return mtrc("global", "%n day(s) ago", nullptr, days);
    }

    int weeks = days / 7;

    if (weeks >= 1 && weeks <= 4) {
        //: Omit the "%n" for the singular translation and translate to (the equivalent of) "Last week"
        return mtrc("global", "%n week(s) ago", nullptr, weeks);
    }

    Date currentDate = currentDateTime.date();
    constexpr int monthInYear = 12;
    int months = (currentDate.year() - dateTime.year()) * monthInYear + (currentDate.month() - dateTime.month());

    if (months >= 1 && months < monthInYear) {
        //: Omit the "%n" for the singular translation and translate (the equivalent of) to "Last month"
        return mtrc("global", "%n month(s) ago", nullptr, months);
    }

    int years = currentDate.year() - dateTime.year();

    //: Omit the "%n" for the singular translation and translate to (the equivalent of) "Last year"
    return mtrc("global", "%n year(s) ago", nullptr, years);
}
