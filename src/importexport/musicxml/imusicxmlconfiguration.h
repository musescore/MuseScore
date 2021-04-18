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
#ifndef MU_IMPORTEXPORT_IMUSICXMLCONFIGURATION_H
#define MU_IMPORTEXPORT_IMUSICXMLCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "io/path.h"

namespace mu::iex::musicxml {
class IMusicXmlConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMusicXmlConfiguration)

public:
    virtual ~IMusicXmlConfiguration() = default;

    virtual bool musicxmlImportBreaks() const = 0;
    virtual void setMusicxmlImportBreaks(bool value) = 0;

    virtual bool musicxmlImportLayout() const = 0;
    virtual void setMusicxmlImportLayout(bool value) = 0;

    virtual bool musicxmlExportLayout() const = 0;
    virtual void setMusicxmlExportLayout(bool value) = 0;

    enum class MusicxmlExportBreaksType {
        All, Manual, No
    };

    virtual MusicxmlExportBreaksType musicxmlExportBreaksType() const = 0;

    virtual bool needUseDefaultFont() const = 0;
    virtual void setNeedUseDefaultFont(bool value) = 0;

    virtual bool needAskAboutApplyingNewStyle() const = 0;
    virtual void setNeedAskAboutApplyingNewStyle(bool value) = 0;

    virtual io::path styleFileImportPath() const = 0;
    virtual void setStyleFileImportPath(const io::path& path) = 0;
};
}

#endif // MU_IMPORTEXPORT_IMUSICXMLCONFIGURATION_H
