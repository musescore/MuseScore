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

String DataFormatter::formatTimeSince(const Date& date)
{
    Date currentDate = DateTime::currentDateTime().date();
    int days = date.daysTo(currentDate);

    if (days == 0) {
        return mtrc("global", "Today");
    }

    if (days < 7) {
        //: Please provide a translation for the singular and all plural forms available in your language.
        //: See https://github.com/musescore/MuseScore/wiki/Help-translate-MuseScore#plural-forms
        //: Possible translations for the 0 case and any value > 6 are not used, yet mandatory.
        //: You may translate singular case to the equivalent of "Yesterday" instead of the "1 day ago" form, but only do this
        //: if you are sure that the translation is only used for a single value of "%n".
        return mtrc("global", "%n day(s) ago", nullptr, days);
    }

    int weeks = days / 7;

    if (weeks <= 4) {
        //: Please provide a translation for the singular and all plural forms available in your language.
        //: See https://github.com/musescore/MuseScore/wiki/Help-translate-MuseScore#plural-forms
        //: Possible translations for the 0 case and any value > 6 are not used, yet mandatory.
        //: You may translate singular case to the equivalent of "Last week" instead of the "1 week ago" form, but only do this
        //: if you are sure that the translation is only used for a single value of "%n".
        return mtrc("global", "%n week(s) ago", nullptr, weeks);
    }

    constexpr int monthsInYear = 12;

    int months = (currentDate.year() - date.year()) * monthsInYear + (currentDate.month() - date.month());

    if (months < monthsInYear) {
        //: Please provide a translation for the singular and all plural forms available in your language.
        //: See https://github.com/musescore/MuseScore/wiki/Help-translate-MuseScore#plural-forms
        //: Possible translations for the 0 case and any value > 6 are not used, yet mandatory.
        //: You may translate singular case to the equivalent of "Last month" instead of the "1 month ago" form, but only do this
        //: if you are sure that the translation is only used for a single value of "%n".
        return mtrc("global", "%n month(s) ago", nullptr, months);
    }

    int years = currentDate.year() - date.year();

    //: Please provide a translation for the singular and all plural forms available in your language.
    //: See https://github.com/musescore/MuseScore/wiki/Help-translate-MuseScore#plural-forms
    //: Possible translations for the 0 case and any value > 6 are not used, yet mandatory.
    //: You may translate singular case to the equivalent of "Last year" instead of the "1 year ago" form, but only do this
    //: if you are sure that the translation is only used for a single value of "%n".
    return mtrc("global", "%n year(s) ago", nullptr, years);
}
