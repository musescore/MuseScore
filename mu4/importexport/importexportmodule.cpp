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
#include "importexportmodule.h"

#include "log.h"
#include "config.h"
#include "modularity/ioc.h"
#include "notation/inotationreadersregister.h"
#include "internal/musicxmlreader.h"
#include "internal/notationmidireader.h"
#include "internal/musedatareader.h"
#include "internal/notationbbreader.h"
#include "internal/capellareader.h"
#include "internal/overeader.h"
#include "internal/notationbwwreader.h"
#include "internal/guitarproreader.h"

#include "internal/importexportconfiguration.h"

using namespace mu::importexport;
using namespace mu::notation;

std::string ImportExportModule::moduleName() const
{
    return "importexport";
}

void ImportExportModule::registerExports()
{
    framework::ioc()->registerExport<IImportexportConfiguration>(moduleName(), new ImportexportConfiguration());
}

void ImportExportModule::onInit()
{
    //! NOTE The "importexport" module is linked when the BUILD_UI_MU4 is off
    //! So that the import/export implementation is linked and used in the old code.
    //! But of course there is no "INotationReadersRegister"

#ifdef BUILD_UI_MU4
    auto readers = framework::ioc()->resolve<INotationReadersRegister>(moduleName());
    IF_ASSERT_FAILED(readers) {
        return;
    }

    readers->reg({ "xml", "musicxml", "mxl" }, std::make_shared<MusicXmlReader>());
    readers->reg({ "mid", "midi", "kar" }, std::make_shared<NotationMidiReader>());
    readers->reg({ "md" }, std::make_shared<MuseDataReader>());
    readers->reg({ "mgu", "sgu" }, std::make_shared<NotationBBReader>());
    readers->reg({ "cap", "capx" }, std::make_shared<CapellaReader>());
    readers->reg({ "ove", "scw" }, std::make_shared<OveReader>());
    readers->reg({ "bmw", "bww" }, std::make_shared<NotationBwwReader>());
    readers->reg({ "gtp", "gp3", "gp4", "gp5", "gpx", "gp", "ptb" }, std::make_shared<GuitarProReader>());
#endif
}
