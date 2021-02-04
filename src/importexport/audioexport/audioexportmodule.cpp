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
#include "audioexportmodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "notation/inotationwritersregister.h"
#include "internal/mp3writer.h"
#include "internal/wavewriter.h"
#include "internal/oggwriter.h"
#include "internal/flacwriter.h"

using namespace mu::iex::audioexport;
using namespace mu::notation;

std::string AudioExportModule::moduleName() const
{
    return "iex_audioexport";
}

void AudioExportModule::resolveImports()
{
    auto writers = framework::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "wav" }, std::make_shared<WaveWriter>());
        writers->reg({ "mp3" }, std::make_shared<Mp3Writer>());
        writers->reg({ "ogg" }, std::make_shared<OggWriter>());
        writers->reg({ "flac" }, std::make_shared<FlacWriter>());
    }
}
