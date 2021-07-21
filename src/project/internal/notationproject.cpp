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
#include <QFileInfo>
#include <QFile>

#include "engraving/engravingproject.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/mscxcompat.h"
#include "engraving/engravingerrors.h"
#include "engraving/style/defaultstyle.h"

#include "notation/notationerrors.h"
#include "projectaudiosettings.h"

#include "log.h"

using namespace mu::project;
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

    std::string suffix = io::suffix(path);
    if (!isMuseScoreFile(suffix)) {
        return doImport(path, stylePath, forceMode);
    }

    MscReader::Params params;
    params.filePath = path.toQString();
    params.mode = mcsIoModeBySuffix(suffix);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Ret::Code::InternalError);
    }

    MscReader reader(params);
    if (!reader.open()) {
        return make_ret(engraving::Err::FileOpenError);
    }

    return doLoad(reader, stylePath, forceMode);
}

mu::Ret NotationProject::doLoad(engraving::MscReader& reader, const io::path& stylePath, bool forceMode)
{
    // Create new engraving project
    EngravingProjectPtr project = EngravingProject::create();
    project->setPath(reader.params().filePath);

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
    std::string suffix = io::suffix(path);
    INotationReaderPtr scoreReader = readers()->reader(suffix);
    if (!scoreReader) {
        return make_ret(notation::Err::FileUnknownType, path);
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

mu::Ret NotationProject::createNew(const ProjectCreateOptions& projectOptions)
{
    // Create new engraving project
    EngravingProjectPtr project = EngravingProject::create();

    // Load template if present
    io::path templatePath = projectOptions.templatePath;
    EngravingProjectPtr templateProject;
    if (!templatePath.empty()) {
        templateProject = EngravingProject::create(DefaultStyle::baseStyle());
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

    Ms::MasterScore* masterScore = project->masterScore();

    // Setup project properties
    if (!projectOptions.title.isEmpty()) {
        masterScore->fileInfo()->setFile(projectOptions.title);
        masterScore->setMetaTag("workTitle", projectOptions.title);
    }
    if (!projectOptions.subtitle.isEmpty()) {
        masterScore->setMetaTag("subtitle", projectOptions.subtitle);
    }
    if (!projectOptions.composer.isEmpty()) {
        masterScore->setMetaTag("composer", projectOptions.composer);
    }
    if (!projectOptions.lyricist.isEmpty()) {
        masterScore->setMetaTag("lyricist", projectOptions.lyricist);
    }
    if (!projectOptions.copyright.isEmpty()) {
        masterScore->setMetaTag("copyright", projectOptions.copyright);
    }

    // Make new master score
    MasterNotationPtr masterNotation = std::shared_ptr<MasterNotation>(new MasterNotation());
    Ms::MasterScore* templateScore = templateProject ? templateProject->masterScore() : nullptr;
    Ret ret = masterNotation->setupNewScore(masterScore, templateScore, projectOptions.scoreOptions);
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

    return make_ret(notation::Err::UnknownError);
}

mu::Ret NotationProject::writeToDevice(io::Device* device)
{
    if (m_engravingProject->path().isEmpty()) {
        m_engravingProject->setPath(m_masterNotation->metaInfo().title + ".mscz");
    }

    MscWriter::Params params;
    params.device = device;
    params.filePath = m_engravingProject->path();
    params.mode = MscIoMode::Zip;

    MscWriter msczWriter(params);
    msczWriter.open();

    Ret ret = writeProject(msczWriter, false);
    return ret;
}

mu::Ret NotationProject::saveScore(const io::path& path, SaveMode saveMode)
{
    std::string suffix = io::suffix(path);
    if (!isMuseScoreFile(suffix) && !suffix.empty()) {
        return exportProject(path, suffix);
    }

    io::path oldFilePath = m_engravingProject->path();
    if (!path.empty()) {
        m_engravingProject->setPath(path.toQString());
    }

    Ret ret = doSave(true);
    if (!ret) {
        ret.setText(Ms::MScore::lastError.toStdString());
    } else if (saveMode != SaveMode::SaveCopy || oldFilePath == path) {
        m_masterNotation->onSaveCopy();
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::doSave(bool generateBackup)
{
    // Step 1: create backup if need
    if (generateBackup) {
        //! TODO Make backup
        NOT_IMPLEMENTED << "generate backup";
    }

    // Step 2: check writable
    QFileInfo info(m_engravingProject->path());
    if (info.exists() && !info.isWritable()) {
        LOGE() << "failed save, not writable path: " << info.filePath();
        return make_ret(notation::Err::UnknownError);
    }

    // Step 3: write project
    std::string suffix = io::suffix(info.fileName());
    MscWriter::Params params;
    params.filePath = m_engravingProject->path();
    params.mode = mcsIoModeBySuffix(suffix);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Ret::Code::InternalError);
    }

    MscWriter msczWriter(params);
    Ret ret = writeProject(msczWriter, false);
    if (!ret) {
        LOGE() << "failed write project to buffer";
        return ret;
    }

    msczWriter.close();

    // make file readable by all
    QFile::setPermissions(info.filePath(), QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);

    LOGI() << "success save file: " << info.filePath();
    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::writeProject(MscWriter& msczWriter, bool onlySelection)
{
    // Create MsczWriter
    bool ok = msczWriter.open();
    if (!ok) {
        LOGE() << "failed open writer";
        return make_ret(notation::Err::FileOpenError);
    }

    // Write engraving project
    ok = m_engravingProject->writeMscz(msczWriter, onlySelection, true);
    if (!ok) {
        LOGE() << "failed write engraving project to mscz";
        return make_ret(notation::Err::UnknownError);
    }

    // Write other stuff
    Ret ret = m_projectAudioSettings->write(msczWriter);
    if (!ret) {
        LOGE() << "failed write project audio settings, err: " << ret.toString();
        return ret;
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::saveSelectionOnScore(const mu::io::path& path)
{
    IF_ASSERT_FAILED(path.toQString() != m_engravingProject->path()) {
        return make_ret(notation::Err::UnknownError);
    }

    // Check writable
    QFileInfo info(path.toQString());
    if (info.exists() && !info.isWritable()) {
        LOGE() << "failed save, not writable path: " << info.filePath();
        return make_ret(notation::Err::UnknownError);
    }

    // Write project
    std::string suffix = io::suffix(info.fileName());
    MscWriter::Params params;
    params.filePath = m_engravingProject->path();
    params.mode = mcsIoModeBySuffix(suffix);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Ret::Code::InternalError);
    }

    MscWriter msczWriter(params);
    Ret ret = writeProject(msczWriter, false);

    if (ret) {
        QFile::setPermissions(info.filePath(),
                              QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    }
    LOGI() << "success save file: " << info.filePath();
    return ret;
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
