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
#ifndef MU_APPSHELL_COMMANDLINECONTROLLER_H
#define MU_APPSHELL_COMMANDLINECONTROLLER_H

#include <QCommandLineParser>
#include <QStringList>

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "ui/iuiconfiguration.h"
#include "importexport/imagesexport/iimagesexportconfiguration.h"

namespace mu::appshell {
class CommandLineController
{
    INJECT(appshell, framework::IApplication, application)
    INJECT(appshell, ui::IUiConfiguration, uiConfiguration)
    INJECT(appshell, iex::imagesexport::IImagesExportConfiguration, imagesExportConfiguration)
public:
    CommandLineController() = default;

    struct ConverterTask {
        bool isBatchMode = false;
        QString inputFile;
        QString outputFile;
    };

    void parse(const QStringList& args);
    void apply();

    ConverterTask converterTask() const;

private:

    QCommandLineParser m_parser;
    ConverterTask m_converterTask;
};
}

#endif // MU_APPSHELL_COMMANDLINECONTROLLER_H
