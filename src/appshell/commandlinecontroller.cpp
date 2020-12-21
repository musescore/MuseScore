//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "commandlinecontroller.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::framework;

void CommandLineController::parse(const QStringList& args)
{
    // Common
    m_parser.addHelpOption(); // -?, -h, --help
    m_parser.addVersionOption(); // -v, --version

    m_parser.addOption(QCommandLineOption({ "D", "monitor-resolution" }, "Specify monitor resolution", "DPI"));

    // Converter mode
    m_parser.addOption(QCommandLineOption({ "j", "job" }, "Process a conversion job", "file"));
    m_parser.addOption(QCommandLineOption({ "o", "export-to" }, "Export to 'file'. Format depends on file's extension", "file"));

    m_parser.process(args);
}

void CommandLineController::apply()
{
    // Common
    if (m_parser.isSet("D")) {
        bool ok;
        float val = m_parser.value("D").toFloat(&ok);
        if (ok) {
            uiConfiguration()->setPhysicalDotsPerInch(val);
        } else {
            LOGE() << "Not recognized DPI value: " << m_parser.value("D");
        }
    }

    // Converter mode
    if (m_parser.isSet("o")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.exportFile = m_parser.value("o");
    }

    if (m_parser.isSet("j")) {
        application()->setRunMode(IApplication::RunMode::Converter);
        m_converterTask.isBatchMode = true;
        m_converterTask.batchJobFile = m_parser.value("j");
    }
}

CommandLineController::ConverterTask CommandLineController::converterTask() const
{
    return m_converterTask;
}
