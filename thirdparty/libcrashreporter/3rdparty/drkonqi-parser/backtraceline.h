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
#ifndef BACKTRACELINE_H
#define BACKTRACELINE_H

#include <QtCore/QSharedData>
#include <QtCore/QString>

class BacktraceLine
{
public:
    enum LineType {
        Unknown, //unknown type. the default
        EmptyLine, //line is empty
        Crap, //line is gdb's crap (like "(no debugging symbols found)",
              //"[New Thread 0x4275c950 (LWP 11931)]", etc...)
        KCrash, //line is "[KCrash Handler]"
        ThreadIndicator, //line indicates the current thread,
                         //ex. "[Current thread is 0 (process 11313)]"
        ThreadStart, //line indicates the start of a thread's stack.
        SignalHandlerStart, //line indicates the signal handler start
                            //(contains "<signal handler called>")
        StackFrame //line is a normal stack frame
    };

    enum LineRating {
        /* RATING           --          EXAMPLE */
        MissingEverything = 0, // #0 0x0000dead in ?? ()
        MissingFunction = 1, // #0 0x0000dead in ?? () from /usr/lib/libfoobar.so.4
        MissingLibrary = 2, // #0 0x0000dead in foobar()
        MissingSourceFile = 3, // #0 0x0000dead in FooBar::FooBar () from /usr/lib/libfoobar.so.4
        Good = 4, // #0 0x0000dead in FooBar::crash (this=0x0) at /home/user/foobar.cpp:204
        InvalidRating = -1 // (dummy invalid value)
    };

    static const LineRating BestRating = Good;

    BacktraceLine() : d(new Data) {}

    QString toString() const {
        return d->m_line;
    }
    LineType type() const {
        return d->m_type;
    }
    LineRating rating() const {
        return d->m_rating;
    }

    int frameNumber() const {
        return d->m_stackFrameNumber;
    }
    QString functionName() const {
        return d->m_functionName;
    }
    QString fileName() const {
        return d->m_file;
    }
    QString libraryName() const {
        return d->m_library;
    }

protected:
    class Data : public QSharedData
    {
    public:
        Data() : m_type(Unknown), m_rating(InvalidRating),
                 m_stackFrameNumber(-1) {}

        QString m_line;
        LineType m_type;
        LineRating m_rating;
        int m_stackFrameNumber;
        QString m_functionName;
        QString m_file;
        QString m_library;
    };
    QExplicitlySharedDataPointer<Data> d;
};

#endif // BACKTRACELINE_H
