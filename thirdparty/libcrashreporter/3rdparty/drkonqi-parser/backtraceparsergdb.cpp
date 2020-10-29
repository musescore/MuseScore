/*
    Copyright (C) 2009-2010  George Kiagiadakis <gkiagia@users.sourceforge.net>

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
#include "backtraceparsergdb.h"
#include "backtraceparser_p.h"
#include <QtCore/QRegExp>
#include <QtCore/QDebug>

//BEGIN BacktraceLineGdb

class BacktraceLineGdb : public BacktraceLine
{
public:
    BacktraceLineGdb(const QString & line);

private:
    void parse();
    void rate();
};

BacktraceLineGdb::BacktraceLineGdb(const QString & lineStr)
        : BacktraceLine()
{
    d->m_line = lineStr;
    d->m_functionName = QLatin1String("??");
    parse();
    if (d->m_type == StackFrame) {
        rate();
    }
}

void BacktraceLineGdb::parse()
{
    QRegExp regExp;

    if (d->m_line == QLatin1String("\n")) {
        d->m_type = EmptyLine;
        return;
    } else if (d->m_line == QLatin1String("[KCrash Handler]\n")) {
        d->m_type = KCrash;
        return;
    } else if (d->m_line.contains(QStringLiteral("<signal handler called>"))) {
        d->m_type = SignalHandlerStart;
        return;
    }

    regExp.setPattern(QStringLiteral("^#([0-9]+)" //matches the stack frame number, ex. "#0"
                      "[\\s]+(0x[0-9a-f]+[\\s]+in[\\s]+)?" // matches " 0x0000dead in " (optionally)
                      "((\\(anonymous namespace\\)::)?[^\\(]+)" //matches the function name
                      //(anything except left parenthesis, which is the start of the arguments section)
                      //and optionally the prefix "(anonymous namespace)::"
                      "(\\(.*\\))?" //matches the function arguments
                                    //(when the app doesn't have debugging symbols)
                      "[\\s]+(const[\\s]+)?" //matches a traling const, if it exists
                      "\\(.*\\)" //matches the arguments of the function with their values
                                 //(when the app has debugging symbols)
                      "([\\s]+" //beginning of optional file information
                      "(from|at)[\\s]+" //matches "from " or "at "
                      "(.+)" //matches the filename (source file or shared library file)
                      ")?\n$")); //matches trailing newline.
                    //the )? at the end closes the parenthesis before [\\s]+(from|at) and
                    //notes that the whole expression from there is optional.

    if (regExp.exactMatch(d->m_line)) {
        d->m_type = StackFrame;
        d->m_stackFrameNumber = regExp.cap(1).toInt();
        d->m_functionName = regExp.cap(3).trimmed();

        if (!regExp.cap(7).isEmpty()) { //we have file information (stuff after from|at)
            if (regExp.cap(8) == QLatin1String("at")) { //'at' means we have a source file
                d->m_file = regExp.cap(9);
            } else { //'from' means we have a library
                d->m_library = regExp.cap(9);
            }
        }

        qDebug() << d->m_stackFrameNumber << d->m_functionName << d->m_file << d->m_library;
        return;
    }

    regExp.setPattern(QStringLiteral(".*\\(no debugging symbols found\\).*|"
                      ".*\\[Thread debugging using libthread_db enabled\\].*|"
                      ".*\\[New .*|"
                      "0x[0-9a-f]+.*|"
                      "Current language:.*"));
    if (regExp.exactMatch(d->m_line)) {
        qDebug() << "garbage detected:" << d->m_line;
        d->m_type = Crap;
        return;
    }

    regExp.setPattern(QStringLiteral("Thread [0-9]+\\s+\\(Thread [0-9a-fx]+\\s+\\(.*\\)\\):\n"));
    if (regExp.exactMatch(d->m_line)) {
        qDebug() << "thread start detected:" << d->m_line;
        d->m_type = ThreadStart;
        return;
    }

    regExp.setPattern(QStringLiteral("\\[Current thread is [0-9]+ \\(.*\\)\\]\n"));
    if (regExp.exactMatch(d->m_line)) {
        qDebug() << "thread indicator detected:" << d->m_line;
        d->m_type = ThreadIndicator;
        return;
    }

    qDebug() << "line" << d->m_line << "did not match";
}

void BacktraceLineGdb::rate()
{
    LineRating r;

    //for explanations, see the LineRating enum definition
    if (!fileName().isEmpty()) {
        r = Good;
    } else if (!libraryName().isEmpty()) {
        if (functionName() == QLatin1String("??")) {
            r = MissingFunction;
        } else {
            r = MissingSourceFile;
        }
    } else {
        if (functionName() == QLatin1String("??")) {
            r = MissingEverything;
        } else {
            r = MissingLibrary;
        }
    }

    d->m_rating = r;
}

//END BacktraceLineGdb

//BEGIN BacktraceParserGdb

class BacktraceParserGdbPrivate : public BacktraceParserPrivate
{
public:
    BacktraceParserGdbPrivate()
        : BacktraceParserPrivate(),
          m_possibleKCrashStart(0), m_threadsCount(0),
          m_isBelowSignalHandler(false), m_frameZeroAppeared(false) {}

    QString m_lineInputBuffer;
    int m_possibleKCrashStart;
    int m_threadsCount;
    bool m_isBelowSignalHandler;
    bool m_frameZeroAppeared;
};

BacktraceParserGdb::BacktraceParserGdb(QObject *parent)
        : BacktraceParser(parent)
{
}

BacktraceParserPrivate* BacktraceParserGdb::constructPrivate() const
{
    return new BacktraceParserGdbPrivate;
}

void BacktraceParserGdb::newLine(const QString & lineStr)
{
    Q_D(BacktraceParserGdb);

    //when the line is too long, gdb splits it into two lines.
    //This breaks parsing and results in two Unknown lines instead of a StackFrame one.
    //Here we workaround this by joining the two lines when such a scenario is detected.
    if (d->m_lineInputBuffer.isEmpty()) {
        d->m_lineInputBuffer = lineStr;
    } else if (lineStr.startsWith(QLatin1Char(' ')) || lineStr.startsWith(QLatin1Char('\t'))) {
        //gdb always adds some whitespace at the beginning of the second line
        d->m_lineInputBuffer.append(lineStr);
    } else {
        parseLine(d->m_lineInputBuffer);
        d->m_lineInputBuffer = lineStr;
    }
}

void BacktraceParserGdb::parseLine(const QString & lineStr)
{
    Q_D(BacktraceParserGdb);

    BacktraceLineGdb line(lineStr);
    switch (line.type()) {
    case BacktraceLine::Crap:
        break; //we don't want crap in the backtrace ;)
    case BacktraceLine::ThreadStart:
        d->m_linesList.append(line);
        d->m_possibleKCrashStart = d->m_linesList.size();
        d->m_threadsCount++;
        //reset the state of the flags that need to be per-thread
        d->m_isBelowSignalHandler = false;
        d->m_frameZeroAppeared = false; // gdb bug workaround flag, see below
        break;
    case BacktraceLine::SignalHandlerStart:
        if (!d->m_isBelowSignalHandler) {
            //replace the stack frames of KCrash with a nice message
            d->m_linesList.erase(d->m_linesList.begin() + d->m_possibleKCrashStart, d->m_linesList.end());
            d->m_linesList.insert(d->m_possibleKCrashStart, BacktraceLineGdb(QStringLiteral("[KCrash Handler]\n")));
            d->m_isBelowSignalHandler = true; //next line is the first below the signal handler
        } else {
            //this is not the first time we see a crash handler frame on the same thread,
            //so we just add it to the list
            d->m_linesList.append(line);
        }
        break;
    case BacktraceLine::StackFrame:
        // gdb workaround - (v6.8 at least) - 'thread apply all bt' writes
        // the #0 stack frame again at the end.
        // Here we ignore this frame by using a flag that tells us whether
        // this is the first or the second time that the #0 frame appears in this thread.
        // The flag is cleared on each thread start.
        if (line.frameNumber() == 0) {
            if (d->m_frameZeroAppeared) {
                break; //break from the switch so that the frame is not added to the list.
            } else {
                d->m_frameZeroAppeared = true;
            }
        }

        //rate the stack frame if we are below the signal handler
        if (d->m_isBelowSignalHandler) {
            d->m_linesToRate.append(line);
        }

        //fall through and append the line to the list
    default:
        d->m_linesList.append(line);
        break;
    }
}

QString BacktraceParserGdb::parsedBacktrace() const
{
    Q_D(const BacktraceParserGdb);

    QString result;
    if (d) {
        QList<BacktraceLine>::const_iterator i;
        for (i = d->m_linesList.constBegin(); i != d->m_linesList.constEnd(); ++i) {
            //if there is only one thread, we can omit the thread indicator,
            //the thread header and all the empty lines.
            if (d->m_threadsCount == 1 && ((*i).type() == BacktraceLine::ThreadIndicator
                                        || (*i).type() == BacktraceLine::ThreadStart
                                        || (*i).type() == BacktraceLine::EmptyLine))
            {
                continue;
            }
            result += i->toString();
        }
    }
    return result;
}

QList<BacktraceLine> BacktraceParserGdb::parsedBacktraceLines() const
{
    Q_D(const BacktraceParserGdb);

    QList<BacktraceLine> result;
    if (d) {
        QList<BacktraceLine>::const_iterator i;
        for (i = d->m_linesList.constBegin(); i != d->m_linesList.constEnd(); ++i) {
            //if there is only one thread, we can omit the thread indicator,
            //the thread header and all the empty lines.
            if (d->m_threadsCount == 1 && ((*i).type() == BacktraceLine::ThreadIndicator
                                        || (*i).type() == BacktraceLine::ThreadStart
                                        || (*i).type() == BacktraceLine::EmptyLine))
            {
                continue;
            }
            result.append(*i);
        }
    }
    return result;
}

//END BacktraceParserGdb


