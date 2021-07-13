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
#include "notationproject.h"

#include "engraving/engravingproject.h"
#include "engraving/scoreaccess.h"

#include "notationerrors.h"
#include "masternotation.h"
#include "projectaudiosettings.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::engraving;

NotationProject::NotationProject()
{
    m_engravingProject = EngravingProject::create();
    m_masterNotation = std::shared_ptr<MasterNotation>(new MasterNotation());
    m_projectAudioSettings = std::shared_ptr<ProjectAudioSettings>(new ProjectAudioSettings());
}

mu::io::path NotationProject::path() const
{
    return m_engravingProject->path();
}

mu::Ret NotationProject::load(const io::path& path, const io::path& stylePath, bool forceMode)
{
    TRACEFUNC;

    std::string syffix = io::syffix(path);

    //! NOTE For "mscz", "mscx" see MsczNotationReader
    //! for others see readers in importexport module
    auto reader = readers()->reader(syffix);
    if (!reader) {
        return make_ret(notation::Err::FileUnknownType, path);
    }

    return load(path, stylePath, reader, forceMode);
}

mu::Ret NotationProject::load(const io::path& path, const io::path& stylePath, const INotationReaderPtr& reader, bool forceMode)
{
    EngravingProjectPtr project = EngravingProject::create();
    project->setPath(path.toQString());

    INotationReader::Options options;
    if (forceMode) {
        options[INotationReader::OptionKey::ForceMode] = forceMode;
    }

    Ms::ScoreLoad sl;

    Ms::MasterScore* score = project->masterScore();
    Ret ret = reader->read(score, path, options);
    if (!ret) {
        return ret;
    }

    if (!Ms::MScore::lastError.isEmpty()) {
        LOGE() << Ms::MScore::lastError;
    }

    project->setupMasterScore();
    if (!Ms::MScore::lastError.isEmpty()) {
        LOGE() << Ms::MScore::lastError;
    }

    if (!stylePath.empty()) {
        score->loadStyle(stylePath.toQString());

        if (!Ms::MScore::lastError.isEmpty()) {
            LOGE() << Ms::MScore::lastError;
        }
    }

    m_engravingProject = project;
    m_masterNotation->setMasterScore(project->masterScore());

    return make_ret(Ret::Code::Ok);
}

IMasterNotationPtr NotationProject::masterNotation() const
{
    return m_masterNotation;
}

mu::Ret NotationProject::createNew(const ScoreCreateOptions& scoreOptions)
{
    EngravingProjectPtr project = EngravingProject::create();
    Ms::MasterScore* masterScore = project->masterScore();

    io::path templatePath = scoreOptions.templatePath;
    Ms::MasterScore* templateScore = nullptr;
    if (!templatePath.empty()) {
        std::string syffix = io::syffix(templatePath);
        auto reader = readers()->reader(syffix);
        if (!reader) {
            LOGE() << "not found reader for file: " << templatePath;
            return make_ret(Ret::Code::InternalError);
        }

        EngravingProjectPtr templateProject = EngravingProject::create(Ms::MScore::baseStyle());
        templateScore = templateProject->masterScore();
        Ret ret = reader->read(templateScore, templatePath);
        if (!ret) {
            return ret;
        }

        templateProject->setupMasterScore();
        if (!ret) {
            return ret;
        }
    }

    Ret ret = m_masterNotation->setupNewScore(masterScore, templateScore, scoreOptions);
    if (!ret) {
        //! NOTE Return old
        m_masterNotation->setMasterScore(m_engravingProject->masterScore());
        return ret;
    }

    m_engravingProject = project;

    return make_ret(Ret::Code::Ok);
}

mu::RetVal<bool> NotationProject::created() const
{
    return m_masterNotation->created();
}

mu::ValNt<bool> NotationProject::needSave() const
{
    return m_masterNotation->needSave();
}

mu::Ret NotationProject::save(const io::path& path, SaveMode saveMode)
{
    switch (saveMode) {
    case SaveMode::SaveSelection:
        return saveSelectionOnScore(path);
    case SaveMode::Save:
    case SaveMode::SaveAs:
    case SaveMode::SaveCopy:
        return saveScore(path);
    }

    return make_ret(Err::UnknownError);
}

mu::Ret NotationProject::writeToDevice(io::Device& destinationDevice)
{
    bool ok = m_engravingProject->writeMscz(&destinationDevice, m_masterNotation->metaInfo().title + ".mscx");
    return ok;
}

mu::Ret NotationProject::saveScore(const io::path& path, SaveMode saveMode)
{
    std::string suffix = io::syffix(path);
    if (suffix != "mscz" && !suffix.empty()) {
        return exportProject(path, suffix);
    }

    io::path oldFilePath = m_engravingProject->path();

    if (!path.empty()) {
        m_engravingProject->setPath(path.toQString());
    }

    Ret ret = m_engravingProject->saveFile(true);
    if (!ret) {
        ret.setText(Ms::MScore::lastError.toStdString());
    } else if (saveMode != SaveMode::SaveCopy || oldFilePath == path) {
        m_masterNotation->onSaveCopy();
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::exportProject(const io::path& path, const std::string& suffix)
{
    QFile file(path.toQString());
    file.open(QFile::WriteOnly);

    auto writer = writers()->writer(suffix);
    if (!writer) {
        LOGE() << "Unknown export format:" << suffix;
        return false;
    }

    Ret ret = writer->write(m_masterNotation, file);
    file.close();

    return ret;
}

mu::Ret NotationProject::saveSelectionOnScore(const mu::io::path& path)
{
    Ret ret = m_engravingProject->saveSelectionOnScore(path.toQString());
    if (!ret) {
        ret.setText(Ms::MScore::lastError.toStdString());
    }

    return ret;
}

Meta NotationProject::metaInfo() const
{
    return m_masterNotation->metaInfo();
}

void NotationProject::setMetaInfo(const Meta& meta)
{
    m_masterNotation->setMetaInfo(meta);
}

IProjectAudioSettingsPtr NotationProject::audioSettings() const
{
    return m_projectAudioSettings;
}
