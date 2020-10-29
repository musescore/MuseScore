/*
    Copyright (C) 2009-2010 George Kiagiadakis <kiagiadakis.george@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "backtraceparsernull.h"
#include "backtraceparser_p.h"

//BEGIN BacktraceLineNull

class BacktraceLineNull : public BacktraceLine
{
public:
    BacktraceLineNull(const QString & line);
};

BacktraceLineNull::BacktraceLineNull(const QString & line)
    : BacktraceLine()
{
    d->m_line = line;
    d->m_rating = MissingEverything;
}

//END BacktraceLineNull

//BEGIN BacktraceParserNull

BacktraceParserNull::BacktraceParserNull(QObject *parent) : BacktraceParser(parent) {}

BacktraceParserPrivate *BacktraceParserNull::constructPrivate() const
{
    BacktraceParserPrivate *d = BacktraceParser::constructPrivate();
    d->m_usefulness = MayBeUseful;
    return d;
}

void BacktraceParserNull::newLine(const QString & lineStr)
{
    d_ptr->m_linesList.append(BacktraceLineNull(lineStr));
}


//END BacktraceParserNull


