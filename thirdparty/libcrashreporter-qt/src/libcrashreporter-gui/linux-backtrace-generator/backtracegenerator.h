/*****************************************************************
 * drkonqi - The KDE Crash Handler
 *
 * Copyright (C) 2000-2003 Hans Petter Bieker <bieker@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************/

#ifndef BACKTRACEGENERATOR_H
#define BACKTRACEGENERATOR_H

#include <QProcess>
#include <QTemporaryFile>

#include "debugger.h"

class BacktraceParser;

class BacktraceGenerator : public QObject
{
    Q_OBJECT

public:
    enum State { NotLoaded, Loading, Loaded, Failed, FailedToStart };

    BacktraceGenerator(const Debugger & debugger,
                       const CrashedApplication* crashedApp,
                       QObject *parent);
    ~BacktraceGenerator() override;

    State state() const {
        return m_state;
    }

    BacktraceParser *parser() const {
        return m_parser;
    }

    QString backtrace() const {
        return m_parsedBacktrace;
    }

    const Debugger debugger() const {
        return m_debugger;
    }

public Q_SLOTS:
    bool start();

Q_SIGNALS:
    void starting();
    void newLine(const QString &str); // emitted for every line
    void someError();
    void failedToStart();
    void done();

private Q_SLOTS:
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReadInput();

private:
    const Debugger    m_debugger;
    QProcess *        m_proc;
    QTemporaryFile *  m_temp;
    QByteArray        m_output;
    State             m_state;
    BacktraceParser * m_parser;
    QString           m_parsedBacktrace;
    const CrashedApplication* m_crashedApplication;
};

#endif
