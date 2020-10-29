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
#ifndef BACKTRACEPARSER_P_H
#define BACKTRACEPARSER_P_H

#include "backtraceparser.h"

class BacktraceParserPrivate
{
public:
    BacktraceParserPrivate() : m_usefulness(BacktraceParser::InvalidUsefulness) {}
    virtual ~BacktraceParserPrivate() {}

    QList<BacktraceLine> m_linesList;
    QList<BacktraceLine> m_linesToRate;
    QStringList m_firstUsefulFunctions;
    QString m_simplifiedBacktrace;
    QSet<QString> m_librariesWithMissingDebugSymbols;
    BacktraceParser::Usefulness m_usefulness;
};

#endif //BACKTRACEPARSER_P_H
