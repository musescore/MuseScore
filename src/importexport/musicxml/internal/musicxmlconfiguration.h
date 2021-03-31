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
#ifndef MU_IMPORTEXPORT_MUSICXMLCONFIGURATION_H
#define MU_IMPORTEXPORT_MUSICXMLCONFIGURATION_H

#include "../imusicxmlconfiguration.h"

namespace mu::iex::musicxml {
class MusicXmlConfiguration : public IMusicXmlConfiguration
{
public:
    void init();

    bool musicxmlImportBreaks() const override;
    void setMusicxmlImportBreaks(bool value) override;

    bool musicxmlImportLayout() const override;
    void setMusicxmlImportLayout(bool value) override;

    bool musicxmlExportLayout() const override;
    void setMusicxmlExportLayout(bool value) override;

    MusicxmlExportBreaksType musicxmlExportBreaksType() const override;

    bool needUseDefaultFont() const override;
    void setNeedUseDefaultFont(bool value) override;

    bool needAskAboutApplyingNewStyle() const override;
    void setNeedAskAboutApplyingNewStyle(bool value) override;

    io::path styleFileImportPath() const override;
    void setStyleFileImportPath(const io::path& path) override;
};
}

#endif // MU_IMPORTEXPORT_MUSICXMLCONFIGURATION_H
