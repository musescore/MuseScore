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
#ifndef BACKTRACEPARSERGDB_H
#define BACKTRACEPARSERGDB_H

#include "backtraceparser.h"
class BacktraceParserGdbPrivate;

class BacktraceParserGdb : public BacktraceParser
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BacktraceParserGdb)
public:
    explicit BacktraceParserGdb(QObject *parent = 0);

    QString parsedBacktrace() const override;
    QList<BacktraceLine> parsedBacktraceLines() const override;

protected:
    BacktraceParserPrivate *constructPrivate() const override;

protected Q_SLOTS:
    void newLine(const QString & lineStr) override;

private:
    void parseLine(const QString & lineStr);
};

#endif // BACKTRACEPARSERGDB_H
