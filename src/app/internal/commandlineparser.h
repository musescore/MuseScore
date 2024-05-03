/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_APP_COMMANDLINEPARSER_H
#define MU_APP_COMMANDLINEPARSER_H

#include <QCommandLineParser>
#include <QStringList>

#include "cmdoptions.h"

class QCoreApplication;

namespace mu::app {
class CommandLineParser
{
    //! NOTE: This parser is created at the earliest stage of the application initialization
    //! You should not inject anything into it
public:
    CommandLineParser() = default;

    void init();
    void parse(int argc, char** argv);
    void processBuiltinArgs(const QCoreApplication& app);

    muse::IApplication::RunMode runMode() const;

    // CmdOptions
    const CmdOptions& options() const;

    // Tasks
    CmdOptions::ConverterTask converterTask() const;
    CmdOptions::Diagnostic diagnostic() const;
    CmdOptions::Autobot autobot() const;
    CmdOptions::AudioPluginRegistration audioPluginRegistration() const;

private:
    void printLongVersion() const;

    QCommandLineParser m_parser;
    CmdOptions m_options;
};
}

#endif // MU_APP_COMMANDLINEPARSER_H
