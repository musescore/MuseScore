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
#ifndef MU_NOTATION_NOTATIONPROJECT_H
#define MU_NOTATION_NOTATIONPROJECT_H

#include "../inotationproject.h"
#include "engraving/engravingproject.h"

#include "../inotationreadersregister.h"
#include "../inotationwritersregister.h"

namespace mu::notation {
class MasterNotation;
class NotationProject : public INotationProject
{
    INJECT(notation, INotationReadersRegister, readers)
    INJECT(notation, INotationWritersRegister, writers)

public:
    NotationProject();

    io::path path() const override;

    Ret load(const io::path& path, const io::path& stylePath = io::path(), bool forceMode = false) override;
    Ret createNew(const ScoreCreateOptions& scoreInfo) override;

    RetVal<bool> created() const override;
    ValNt<bool> needSave() const override;

    Ret save(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save) override;
    Ret writeToDevice(io::Device& destinationDevice) override;

    Meta metaInfo() const override;
    void setMetaInfo(const Meta& meta) override;

    IMasterNotationPtr masterNotation() const override;

private:

    Ret load(const io::path& path, const io::path& stylePath, const INotationReaderPtr& reader, bool forceMode);

    Ret saveScore(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save);
    Ret saveSelectionOnScore(const io::path& path = io::path());
    Ret exportProject(const io::path& path, const std::string& suffix);

    mu::engraving::EngravingProjectPtr m_engravingProject = nullptr;
    std::shared_ptr<MasterNotation> m_masterNotation = nullptr;
};
}

#endif // MU_NOTATION_NOTATIONPROJECT_H
