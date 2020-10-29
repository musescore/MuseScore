/*
    Copyright (C) 2010 George Kiagiadakis <kiagiadakis.george@gmail.com>

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
#include "backtraceparserkdbgwin.h"
#include "backtraceparser_p.h"
#include <QDebug>

//BEGIN BacktraceLineKdbgwin

class BacktraceLineKdbgwin : public BacktraceLine
{
public:
    BacktraceLineKdbgwin(const QString & line);

private:
    void parse();
    void rate();
};

BacktraceLineKdbgwin::BacktraceLineKdbgwin(const QString & line)
    : BacktraceLine()
{
    d->m_line = line;
    parse();
    if (d->m_type == StackFrame) {
        rate();
    }
}

void BacktraceLineKdbgwin::parse()
{
    if (d->m_line == QLatin1String("\n")) {
        d->m_type = EmptyLine;
        return;
    } else if (d->m_line == QLatin1String("[KCrash Handler]\n")) {
        d->m_type = KCrash;
        return;
    } else if (d->m_line.startsWith(QLatin1String("Loaded"))) {
        d->m_type = Crap; //FIXME that's not exactly crap
        return;
    }

    QRegExp regExp;
    regExp.setPattern(QStringLiteral("([^!]+)!" //match the module name, followed by !
                      "([^\\(]+)\\(\\) " //match the function name, followed by ()
                      "\\[([^@]+)@ [\\-\\d]+\\] " // [filename @ line]
                      "at 0x.*")); //at 0xdeadbeef

    if (regExp.exactMatch(d->m_line)) {
        d->m_type = StackFrame;
        d->m_library = regExp.cap(1);
        d->m_functionName = regExp.cap(2);
        d->m_file = regExp.cap(3).trimmed();

        qDebug() << d->m_functionName << d->m_file << d->m_library;
        return;
    }

    qDebug() << "line" << d->m_line << "did not match";
}

void BacktraceLineKdbgwin::rate()
{
    LineRating r;

    //for explanations, see the LineRating enum definition
    if (fileName() != QLatin1String("[unknown]")) {
        r = Good;
    } else if (libraryName() != QLatin1String("[unknown]")) {
        if (functionName() == QLatin1String("[unknown]")) {
            r = MissingFunction;
        } else {
            r = MissingSourceFile;
        }
    } else {
        if (functionName() == QLatin1String("[unknown]")) {
            r = MissingEverything;
        } else {
            r = MissingLibrary;
        }
    }

    d->m_rating = r;
}

//END BacktraceLineKdbgwin

//BEGIN BacktraceParserKdbgwin

BacktraceParserKdbgwin::BacktraceParserKdbgwin(QObject *parent)
    : BacktraceParser(parent)
{
}

void BacktraceParserKdbgwin::newLine(const QString & lineStr)
{
    Q_D(BacktraceParser);

    BacktraceLineKdbgwin line(lineStr);
    switch(line.type()) {
    case BacktraceLine::Crap:
        break; //we don't want crap in the backtrace ;)
    case BacktraceLine::StackFrame:
        d->m_linesToRate.append(line);
    default:
        d->m_linesList.append(line);
    }
}

//END BacktraceParserKdbgwin


