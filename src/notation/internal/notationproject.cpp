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

#include <QBuffer>

#include "engraving/engravingproject.h"
#include "engraving/scoreaccess.h"
#include "engraving/compat/mscxcompat.h"

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
    if (syffix != "mscz" && syffix != "mscx") {
        return doImport(path, stylePath, forceMode);
    }

    QByteArray msczData;
    if (syffix == "mscz") {
        RetVal<QByteArray> rv = fileSystem()->readFile(path);
        if (!rv.ret) {
            return rv.ret;
        }
        msczData = rv.val;
    } else if (syffix == "mscx") {
        Ms::Score::FileError err = engraving::compat::mscxToMscz(path.toQString(), &msczData);
        if (err != Ms::Score::FileError::FILE_NO_ERROR) {
            return scoreFileErrorToRet(err, path);
        }
    } else {
        UNREACHABLE;
        return make_ret(Ret::Code::InternalError);
    }

    QBuffer msczBuf(&msczData);
    MsczReader reader(&msczBuf);
    reader.setFilePath(path.toQString());
    reader.open();

    return doLoad(reader, stylePath, forceMode);
}

mu::Ret NotationProject::doLoad(engraving::MsczReader& reader, const io::path& stylePath, bool forceMode)
{
    // Create new engraving project
    EngravingProjectPtr project = EngravingProject::create();
    project->setPath(reader.filePath());

    // Load engraving project
    engraving::Err err = project->loadMscz(reader, forceMode);
    if (err != engraving::Err::NoError) {
        return make_ret(err);
    }

    // Setup master score
    err = project->setupMasterScore();
    if (err != engraving::Err::NoError) {
        return make_ret(err);
    }

    // Load style if present
    if (!stylePath.empty()) {
        project->masterScore()->loadStyle(stylePath.toQString());
        if (!Ms::MScore::lastError.isEmpty()) {
            LOGE() << Ms::MScore::lastError;
        }
    }

    // Load other stuff from the project file
    ProjectAudioSettingsPtr audioSettings = std::shared_ptr<ProjectAudioSettings>(new ProjectAudioSettings());
    Ret ret = audioSettings->read(reader);
    if (!ret) {
        return ret;
    }

    // Set current if all success
    m_engravingProject = project;

    m_masterNotation = std::shared_ptr<MasterNotation>(new MasterNotation());
    m_masterNotation->setMasterScore(project->masterScore());

    m_projectAudioSettings = audioSettings;

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::doImport(const io::path& path, const io::path& stylePath, bool forceMode)
{
    // Find import reader
    std::string syffix = io::syffix(path);
    INotationReaderPtr scoreReader = readers()->reader(syffix);
    if (!scoreReader) {
        return make_ret(Err::FileUnknownType, path);
    }

    // Create new engraving project
    EngravingProjectPtr project = EngravingProject::create();
    project->setPath(path.toQString());

    // Setup import reader
    INotationReader::Options options;
    if (forceMode) {
        options[INotationReader::OptionKey::ForceMode] = forceMode;
    }

    // Read(import) master score
    Ms::ScoreLoad sl;
    Ms::MasterScore* score = project->masterScore();
    Ret ret = scoreReader->read(score, path, options);
    if (!ret) {
        return ret;
    }

    if (!Ms::MScore::lastError.isEmpty()) {
        LOGE() << Ms::MScore::lastError;
    }

    // Setup master score
    engraving::Err err = project->setupMasterScore();
    if (err != engraving::Err::NoError) {
        return make_ret(err);
    }

    // Load style if present
    if (!stylePath.empty()) {
        project->masterScore()->loadStyle(stylePath.toQString());
        if (!Ms::MScore::lastError.isEmpty()) {
            LOGE() << Ms::MScore::lastError;
        }
    }

    // Setup other stuff
    ProjectAudioSettingsPtr audioSettings = std::shared_ptr<ProjectAudioSettings>(new ProjectAudioSettings());
    audioSettings->makeDefault();

    // Set current if all success
    m_engravingProject = project;

    m_masterNotation = std::shared_ptr<MasterNotation>(new MasterNotation());
    m_masterNotation->setMasterScore(project->masterScore());

    m_projectAudioSettings = audioSettings;

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::createNew(const ScoreCreateOptions& scoreOptions)
{
    // Create new engraving project
    EngravingProjectPtr project = EngravingProject::create();

    // Load template if present
    io::path templatePath = scoreOptions.templatePath;
    EngravingProjectPtr templateProject;
    if (!templatePath.empty()) {
        templateProject = EngravingProject::create(Ms::MScore::baseStyle());
        engraving::Err err = engraving::compat::loadMsczOrMscx(templateProject, templatePath.toQString());
        if (err != engraving::Err::NoError) {
            LOGE() << "failed load template";
            return make_ret(err);
        }

        err = templateProject->setupMasterScore();
        if (err != engraving::Err::NoError) {
            LOGE() << "failed load template";
            return make_ret(err);
        }
    }

    // Make new master score
    MasterNotationPtr masterNotation = std::shared_ptr<MasterNotation>(new MasterNotation());
    Ms::MasterScore* templateScore = templateProject ? templateProject->masterScore() : nullptr;
    Ret ret = masterNotation->setupNewScore(project->masterScore(), templateScore, scoreOptions);
    if (!ret) {
        return ret;
    }

    // Setup other stuff
    ProjectAudioSettingsPtr audioSettings = std::shared_ptr<ProjectAudioSettings>(new ProjectAudioSettings());
    audioSettings->makeDefault();

    // Set current if all success
    m_engravingProject = project;
    m_masterNotation = masterNotation;
    m_projectAudioSettings = audioSettings;

    return make_ret(Ret::Code::Ok);
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

IMasterNotationPtr NotationProject::masterNotation() const
{
    return m_masterNotation;
}

mu::RetVal<bool> NotationProject::created() const
{
    return m_masterNotation->created();
}

mu::ValNt<bool> NotationProject::needSave() const
{
    return m_masterNotation->needSave();
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
