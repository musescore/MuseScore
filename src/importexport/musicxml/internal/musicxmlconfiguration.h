/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
    void setMusicxmlExportBreaksType(MusicxmlExportBreaksType breaksType) override;

    bool musicxmlExportInvisibleElements() const override;
    void setMusicxmlExportInvisibleElements(bool value) override;

    bool needUseDefaultFont() const override;
    void setNeedUseDefaultFont(bool value) override;

    bool needAskAboutApplyingNewStyle() const override;
    void setNeedAskAboutApplyingNewStyle(bool value) override;

    io::path_t styleFileImportPath() const override;
    void setStyleFileImportPath(const io::path_t& path) override;
};
}

#endif // MU_IMPORTEXPORT_MUSICXMLCONFIGURATION_H
