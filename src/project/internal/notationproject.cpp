/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include <QDir>
#include <QFile>

#include "global/io/buffer.h"
#include "global/io/file.h"
#include "global/io/ioretcodes.h"
#include "global/io/devtools/allzerosfilecorruptor.h"

#include "engraving/dom/undo.h"

#include "engraving/dom/masterscore.h"
#include "engraving/engravingproject.h"
#include "engraving/compat/engravingcompat.h"
#include "engraving/infrastructure/mscio.h"
#include "engraving/engravingerrors.h"

#include "iprojectautosaver.h"
#include "notation/notationerrors.h"
#include "projectaudiosettings.h"
#include "projectfileinfoprovider.h"
#include "projecterrors.h"

#include "defer.h"
#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

static void setupScoreMetaTags(mu::engraving::MasterScore* masterScore, const ProjectCreateOptions& projectOptions)
{
    if (!projectOptions.title.isEmpty()) {
        masterScore->setMetaTag(WORK_TITLE_TAG, projectOptions.title);
    }
    if (!projectOptions.subtitle.isEmpty()) {
        masterScore->setMetaTag(SUBTITLE_TAG, projectOptions.subtitle);
    }
    if (!projectOptions.composer.isEmpty()) {
        masterScore->setMetaTag(COMPOSER_TAG, projectOptions.composer);
    }
    if (!projectOptions.lyricist.isEmpty()) {
        masterScore->setMetaTag(LYRICIST_TAG, projectOptions.lyricist);
    }
    if (!projectOptions.copyright.isEmpty()) {
        masterScore->setMetaTag(COPYRIGHT_TAG, projectOptions.copyright);
    }
    if (!projectOptions.templatePath.empty()) {
        masterScore->setMetaTag(CREATION_DATE_TAG, QDate::currentDate().toString(Qt::ISODate));
    }
}

static QString scoreDefaultTitle()
{
    return muse::qtrc("project", "Untitled score");
}

NotationProject::~NotationProject()
{
    m_projectAudioSettings = nullptr;
    m_masterNotation = nullptr;
    m_engravingProject = nullptr;
}

void NotationProject::setupProject()
{
    TRACEFUNC;

    m_engravingProject = EngravingProject::create(iocContext());
    m_engravingProject->setFileInfoProvider(std::make_shared<ProjectFileInfoProvider>(this));
    m_masterNotation = notationCreator()->newMasterNotationPtr(iocContext());
    m_projectAudioSettings = std::shared_ptr<ProjectAudioSettings>(new ProjectAudioSettings());
}

Ret NotationProject::load(const muse::io::path_t& path, const muse::io::path_t& stylePath, bool forceMode, const std::string& format_)
{
    TRACEFUNC;

    std::string format = format_.empty() ? io::suffix(path) : format_;

    LOGD() << "try load: " << path << ", format: " << format;

    setupProject();
    setPath(path);

    if (!isMuseScoreFile(format)) {
        Ret ret = doImport(path, stylePath.empty() ? notationConfiguration()->styleFileImportPath() : stylePath, forceMode);
        if (ret) {
            listenIfNeedSaveChanges();
        }

        return ret;
    }

    Ret ret = doLoad(path, stylePath, forceMode, format);
    if (!ret) {
        LOGE() << "failed load, err: " << ret.toString();
        return ret;
    }

    bool treatAsImported = m_masterNotation->mscVersion() < 400 && !isCloudProject();

    listenIfNeedSaveChanges();
    setNeedSave(treatAsImported);

    m_isNewlyCreated = treatAsImported;
    m_isImported = treatAsImported;

    return ret;
}

Ret NotationProject::doLoad(const muse::io::path_t& path, const muse::io::path_t& stylePath, bool forceMode, const std::string& format)
{
    TRACEFUNC;

    MscReader::Params params;
    params.filePath = path.toQString();
    params.mode = mscIoModeBySuffix(format);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Ret::Code::InternalError);
    }

    MscReader reader(params);
    Ret ret = reader.open();
    if (!ret) {
        return ret;
    }

    // Load engraving project
    m_engravingProject->setFileInfoProvider(std::make_shared<ProjectFileInfoProvider>(this));

    SettingsCompat settingsCompat;
    ret = m_engravingProject->loadMscz(reader, settingsCompat, forceMode);
    if (!ret) {
        return ret;
    }

    MasterScore* masterScore = m_engravingProject->masterScore();
    IF_ASSERT_FAILED(masterScore) {
        return engraving::make_ret(engraving::Err::UnknownError, reader.params().filePath);
    }

    masterScore->lockUpdates(true);
    DEFER {
        masterScore->lockUpdates(false);
    };

    // Setup master score
    ret = m_engravingProject->setupMasterScore(forceMode);
    if (!ret) {
        return ret;
    }

    // Migration
    if (migrator()) {
        masterScore->lockUpdates(false); // because migration needs a second layout
        ret = migrator()->migrateEngravingProjectIfNeed(m_engravingProject);
        if (!ret) {
            return ret;
        }
        masterScore->lockUpdates(true);
    }

    // Load style if present
    if (!stylePath.empty()) {
        muse::io::File styleFile(stylePath);
        m_engravingProject->masterScore()->loadStyle(styleFile);
    }

    mu::engraving::compat::EngravingCompat::doPreLayoutCompatIfNeeded(m_engravingProject->masterScore());

    masterScore->lockUpdates(false);
    masterScore->setLayoutAll();
    masterScore->update();

    mu::engraving::compat::EngravingCompat::doPostLayoutCompatIfNeeded(m_engravingProject->masterScore());

    // Load audio settings
    bool tryCompatAudio = false;
    ret = m_projectAudioSettings->read(reader);
    if (!ret) {
        m_projectAudioSettings->makeDefault();
        tryCompatAudio = true;
    }

    // Load cloud info
    {
        m_cloudInfo.sourceUrl = masterScore->metaTags()[SOURCE_TAG].toQString();
        m_cloudInfo.revisionId = masterScore->metaTags()[SOURCE_REVISION_ID_TAG].toInt();

        m_cloudAudioInfo.url = masterScore->metaTags()[AUDIO_COM_URL_TAG].toQString();

        if (configuration()->isLegacyCloudProject(path)) {
            m_cloudInfo.name = io::filename(path, false).toQString();
        }
    }

    // Set current if all success
    m_masterNotation->setMasterScore(masterScore);

    // Load view settings & solo-mute states (needs to be done after notations are created)
    m_masterNotation->notation()->viewState()->read(reader);
    m_masterNotation->notation()->soloMuteState()->read(reader);
    for (IExcerptNotationPtr excerpt : m_masterNotation->excerpts()) {
        if (!excerpt->hasFileName()) {
            continue;
        }

        muse::io::path_t ePath = u"Excerpts/" + excerpt->fileName() + u"/";
        excerpt->notation()->viewState()->read(reader, ePath);
        excerpt->notation()->soloMuteState()->read(reader, ePath);
    }

    // Apply compat audio settings (needs to be done after notations are created)
    if (tryCompatAudio && !settingsCompat.audioSettings.empty()) {
        for (const auto& audioCompat : settingsCompat.audioSettings) {
            notation::INotationSoloMuteState::SoloMuteState state = { audioCompat.second.mute, audioCompat.second.solo };
            INotationSoloMuteStatePtr soloMuteStatePtr = m_masterNotation->notation()->soloMuteState();
            soloMuteStatePtr->setTrackSoloMuteState(audioCompat.second.instrumentId, state);
        }
    }

    return make_ret(Ret::Code::Ok);
}

Ret NotationProject::doImport(const muse::io::path_t& path, const muse::io::path_t& stylePath, bool forceMode)
{
    TRACEFUNC;

    // Find import reader
    std::string suffix = io::suffix(path);
    INotationReaderPtr scoreReader = readers()->reader(suffix);
    if (!scoreReader) {
        return make_ret(engraving::Err::FileUnknownType, path);
    }

    // Setup import reader
    INotationReader::Options options;
    if (forceMode) {
        options[INotationReader::OptionKey::ForceMode] = Val(forceMode);
    }

    // Setup engraving project
    mu::engraving::ScoreLoad sl;
    m_engravingProject->setFileInfoProvider(std::make_shared<ProjectFileInfoProvider>(this));
    mu::engraving::MasterScore* score = m_engravingProject->masterScore();

    // The order of the steps matches the order in MS3
    // (see https://github.com/musescore/MuseScore/blob/2513676e512d29d554cb6c4d37d3efaf53ea2c5b/mscore/file.cpp#L2260)

    // Load style if present
    if (!stylePath.empty()) {
        muse::io::File styleFile(stylePath);
        score->loadStyle(styleFile);
    }

    // Init ChordList
    score->checkChordList();

    // Read (import) master score
    Ret ret = scoreReader->read(score, path, options);
    if (!ret) {
        return ret;
    }

    // Setup master score post-reading
    ret = m_engravingProject->setupMasterScore(forceMode);
    if (!ret) {
        return ret;
    }

    // Setup audio settings
    m_projectAudioSettings->makeDefault();

    // Setup view state
    m_masterNotation->notation()->viewState()->makeDefault();
    for (IExcerptNotationPtr excerpt : m_masterNotation->excerpts()) {
        excerpt->notation()->viewState()->makeDefault();
    }

    // Set current if all success
    m_masterNotation->setMasterScore(score);
    setPath(path);
    setNeedSave(false);
    score->setMetaTag(u"originalFormat", QString::fromStdString(suffix));

    m_isNewlyCreated = true;
    m_isImported = true;

    return make_ret(Ret::Code::Ok);
}

Ret NotationProject::createNew(const ProjectCreateOptions& projectOptions)
{
    TRACEFUNC;

    setupProject();

    // Load template if present
    if (!projectOptions.templatePath.empty()) {
        Ret ret = loadTemplate(projectOptions);
        if (ret) {
            listenIfNeedSaveChanges();
        }

        return ret;
    }

    // Create new engraving project
    setPath(projectOptions.title.isEmpty() ? scoreDefaultTitle() : projectOptions.title);
    m_engravingProject->setFileInfoProvider(std::make_shared<ProjectFileInfoProvider>(this));

    mu::engraving::MasterScore* masterScore = m_engravingProject->masterScore();
    setupScoreMetaTags(masterScore, projectOptions);

    // Setup new master score
    Ret ret = m_masterNotation->setupNewScore(masterScore, projectOptions.scoreOptions);
    if (!ret) {
        return ret;
    }

    // Setup audio settings
    m_projectAudioSettings->makeDefault();

    // Setup view state
    m_masterNotation->notation()->viewState()->makeDefault();
    for (IExcerptNotationPtr excerpt : m_masterNotation->excerpts()) {
        excerpt->notation()->viewState()->makeDefault();
    }

    listenIfNeedSaveChanges();
    markAsUnsaved();

    m_isNewlyCreated = true;

    return make_ret(Ret::Code::Ok);
}

Ret NotationProject::loadTemplate(const ProjectCreateOptions& projectOptions)
{
    TRACEFUNC;

    Ret ret = load(projectOptions.templatePath);

    if (ret) {
        setPath(projectOptions.title.isEmpty() ? scoreDefaultTitle() : projectOptions.title);

        mu::engraving::MasterScore* masterScore = m_masterNotation->masterScore();
        setupScoreMetaTags(masterScore, projectOptions);

        m_masterNotation->notation()->undoStack()->lock();
        m_masterNotation->applyOptions(masterScore, projectOptions.scoreOptions, true /*createdFromTemplate*/);
        m_masterNotation->notation()->undoStack()->unlock();

        setNeedSave(false);
        m_isNewlyCreated = true;
    }

    return ret;
}

muse::io::path_t NotationProject::path() const
{
    return m_path;
}

void NotationProject::setPath(const muse::io::path_t& path)
{
    if (m_path == path) {
        return;
    }

    m_path = path;
    m_pathChanged.notify();
    m_displayNameChanged.notify();
}

muse::async::Notification NotationProject::pathChanged() const
{
    return m_pathChanged;
}

QString NotationProject::displayName() const
{
    if (isNewlyCreated()) {
        if (m_path.empty()) {
            QString workTitle = m_masterNotation->notation()->workTitle();
            if (workTitle.isEmpty()) {
                return scoreDefaultTitle();
            }
            return workTitle;
        }
        return io::filename(m_path).toQString();
    }

    if (isCloudProject()) {
        return m_cloudInfo.name;
    }

    bool isSuffixInteresting = io::suffix(m_path) != engraving::MSCZ;
    return io::filename(m_path, isSuffixInteresting).toQString();
}

muse::async::Notification NotationProject::displayNameChanged() const
{
    return m_displayNameChanged;
}

bool NotationProject::isCloudProject() const
{
    return configuration()->isCloudProject(m_path);
}

const CloudProjectInfo& NotationProject::cloudInfo() const
{
    return m_cloudInfo;
}

void NotationProject::setCloudInfo(const CloudProjectInfo& info)
{
    m_cloudInfo = info;
    m_masterNotation->masterScore()->setMetaTag(SOURCE_TAG, info.sourceUrl.toString());
    m_masterNotation->masterScore()->setMetaTag(SOURCE_REVISION_ID_TAG, String::number(info.revisionId));

    m_displayNameChanged.notify();
}

const CloudAudioInfo& NotationProject::cloudAudioInfo() const
{
    return m_cloudAudioInfo;
}

void NotationProject::setCloudAudioInfo(const CloudAudioInfo& audioInfo)
{
    m_cloudAudioInfo = audioInfo;
    m_masterNotation->masterScore()->setMetaTag(AUDIO_COM_URL_TAG, audioInfo.url.toString());
}

Ret NotationProject::save(const muse::io::path_t& path, SaveMode saveMode, bool createBackup)
{
    TRACEFUNC;

    switch (saveMode) {
    case SaveMode::SaveSelection:
        return saveSelectionOnScore(path);
    case SaveMode::Save:
    case SaveMode::SaveAs:
    case SaveMode::SaveCopy: {
        muse::io::path_t savePath = path;
        if (savePath.empty()) {
            IF_ASSERT_FAILED(!m_path.empty()) {
                return false;
            }

            savePath = m_path;
        }

        std::string suffix = io::suffix(savePath);

        // Whether a backup file will be created depends on both the caller's and user's will
        bool shouldCreateBackup = createBackup && configuration()->createBackupBeforeSaving();

        Ret ret = saveScore(savePath, suffix, shouldCreateBackup);
        if (ret) {
            if (saveMode != SaveMode::SaveCopy) {
                markAsSaved(savePath);
            }
        }

        return ret;
    }
    case SaveMode::AutoSave:
        std::string suffix = io::suffix(path);
        if (suffix == IProjectAutoSaver::AUTOSAVE_SUFFIX) {
            suffix = io::suffix(io::completeBasename(path));
        }

        if (suffix.empty()) {
            // Then it must be a MSCX folder
            suffix = engraving::MSCX;
        }

        return saveScore(path, suffix, false /*generateBackup*/, false /*createThumbnail*/, true /*isAutosave*/);
    }

    return make_ret(notation::Err::UnknownError);
}

Ret NotationProject::writeToDevice(QIODevice* device)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(!m_path.empty()) {
        return make_ret(notation::Err::UnknownError);
    }

    Buffer buf;
    buf.open(IODevice::OpenMode::WriteOnly);

    MscWriter::Params params;
    params.device = &buf;
    params.filePath = m_path.toQString();
    params.mode = MscIoMode::Zip;

    MscWriter msczWriter(params);
    msczWriter.open();

    Ret ret = writeProject(msczWriter, false);
    msczWriter.close();

    if (ret) {
        if (msczWriter.hasError()) {
            LOGE() << "MSCZ writer has error";
            return make_ret(Ret::Code::UnknownError);
        }

        buf.open(IODevice::OpenMode::ReadOnly);
        ByteArray ba = buf.readAll();

        if ((size_t)device->write(ba.toQByteArrayNoCopy()) != ba.size()) {
            LOGE() << "Error writing to device";
            return make_ret(Ret::Code::UnknownError);
        }
    }

    return ret;
}

Ret NotationProject::saveScore(const muse::io::path_t& path, const std::string& fileSuffix,
                               bool generateBackup, bool createThumbnail, bool isAutosave)
{
    if (!isMuseScoreFile(fileSuffix) && !fileSuffix.empty()) {
        return exportProject(path, fileSuffix);
    }

    MscIoMode ioMode = mscIoModeBySuffix(fileSuffix);

    return doSave(path, ioMode, generateBackup, createThumbnail, isAutosave);
}

Ret NotationProject::doSave(const muse::io::path_t& path, engraving::MscIoMode ioMode,
                            bool generateBackup, bool createThumbnail, bool isAutosave)
{
    TRACEFUNC;

    QString targetContainerPath = engraving::containerPath(path).toQString();
    muse::io::path_t targetMainFilePath = engraving::mainFilePath(path);
    muse::io::path_t targetMainFileName = engraving::mainFileName(path);
    QString savePath = targetContainerPath + "_saving";

    // Step 1: check writable
    {
        if ((fileSystem()->exists(savePath) && !fileSystem()->isWritable(savePath))
            || (fileSystem()->exists(targetContainerPath) && !fileSystem()->isWritable(targetContainerPath))) {
            LOGE() << "failed save, not writable path: " << targetContainerPath;
            return make_ret(io::Err::FSWriteError);
        }

        if (ioMode == engraving::MscIoMode::Dir) {
            // Dir needs to be created, otherwise we can't move to it
            if (!QDir(targetContainerPath).mkpath(".")) {
                LOGE() << "Couldn't create container directory: " << targetContainerPath;
                return make_ret(io::Err::FSMakingError);
            }
        }
    }

    // Step 2: write project
    {
        MscWriter::Params params;
        params.filePath = savePath;
        params.mainFileName = targetMainFileName.toQString();
        params.mode = ioMode;
        IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
            return make_ret(Ret::Code::InternalError);
        }
        if (ioMode == MscIoMode::Zip
            && !isAutosave
            && globalConfiguration()->devModeEnabled()
            && savePath.contains(" - ALL_ZEROS_CORRUPTED.mscz")) {
            // Create a corrupted file so devs/qa can simulate a saved corrupted file.
            params.device = new AllZerosFileCorruptor(savePath);
        }

        MscWriter msczWriter(params);
        Ret ret = writeProject(msczWriter, false /*onlySelection*/, createThumbnail);
        msczWriter.close();
        if (params.device) {
            delete params.device;
            params.device = nullptr;
        }

        if (!ret) {
            LOGE() << "failed write project to buffer: " << ret.toString();
            return ret;
        }

        if (msczWriter.hasError()) {
            LOGE() << "MscWriter has error after writing project";
            return make_ret(Ret::Code::UnknownError);
        }
    }

    // Step 3: create backup if need
    {
        if (generateBackup) {
            makeBackup(targetContainerPath);
        }
    }

    // Step 4: replace to saved file
    {
        if (ioMode == MscIoMode::Dir) {
            RetVal<io::paths_t> filesToBeMoved = fileSystem()->scanFiles(savePath, { "*" }, io::ScanMode::FilesAndFoldersInCurrentDir);
            if (!filesToBeMoved.ret) {
                return filesToBeMoved.ret;
            }

            Ret ret = muse::make_ok();

            for (const muse::io::path_t& fileToBeMoved : filesToBeMoved.val) {
                muse::io::path_t destinationFile
                    = muse::io::path_t(targetContainerPath).appendingComponent(io::filename(fileToBeMoved));
                LOGD() << fileToBeMoved << " to " << destinationFile;
                ret = fileSystem()->move(fileToBeMoved, destinationFile, true);
                if (!ret) {
                    return ret;
                }
            }

            // Try to remove the temp save folder (not problematic if fails)
            ret = fileSystem()->remove(savePath, true);
            if (!ret) {
                LOGW() << ret.toString();
            }
        } else {
            Ret ret = muse::make_ok();

            ret = fileSystem()->copy(savePath, targetContainerPath, true);
            if (!ret) {
                return ret;
            }

            if (!isAutosave) {
                ret = checkSavedFileForCorruption(ioMode, targetContainerPath, targetMainFileName.toQString());
                if (!ret) {
                    if (ret.code() == (int)Err::CorruptionUponSavingError) {
                        // Validate the temporary "saving" file too.
                        Ret ret2 = checkSavedFileForCorruption(ioMode, savePath, targetMainFileName.toQString());
                        if (!ret2) {
                            return ret2;
                        }
                    }
                    return ret;
                }
            }

            // Remove the temp save file (not problematic if fails)
            fileSystem()->remove(savePath);
        }
    }

    // make file readable by all
    {
        QFile::setPermissions(targetMainFilePath.toQString(),
                              QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    }

    LOGI() << "success save file: " << targetContainerPath;
    return make_ret(Ret::Code::Ok);
}

Ret NotationProject::makeBackup(muse::io::path_t filePath)
{
    TRACEFUNC;

    if (io::suffix(filePath) != engraving::MSCZ) {
        LOGW() << "backup allowed only for MSCZ, currently: " << filePath;
        return make_ret(Ret::Code::Ok);
    }

    Ret ret = fileSystem()->exists(filePath);
    if (!ret) {
        LOGI() << "Backup won't be created, file does not exist: " << filePath;
        return make_ret(Ret::Code::Ok);
    }

    muse::io::path_t backupPath = configuration()->projectBackupPath(filePath);
    muse::io::path_t backupDir = io::absoluteDirpath(backupPath);
    ret = fileSystem()->makePath(backupDir);
    if (!ret) {
        LOGE() << "failed to create backup directory: " << backupDir;
        return ret;
    }

    fileSystem()->setAttribute(backupDir, io::IFileSystem::Attribute::Hidden);

    ret = fileSystem()->copy(filePath, backupPath, true);
    if (!ret) {
        LOGE() << "failed to copy: " << filePath << " to: " << backupPath;
        return ret;
    }

    fileSystem()->setAttribute(backupPath, io::IFileSystem::Attribute::Hidden);

    return ret;
}

Ret NotationProject::writeProject(MscWriter& msczWriter, bool onlySelection, bool createThumbnail)
{
    TRACEFUNC;

    // Create MsczWriter
    Ret ret = msczWriter.open();
    if (!ret) {
        LOGE() << "failed open writer: " << ret.toString();
        return ret;
    }

    // Write engraving project
    ret = m_engravingProject->writeMscz(msczWriter, onlySelection, createThumbnail);
    if (!ret) {
        LOGE() << "failed write engraving project to mscz: " << ret.toString();
        return make_ret(notation::Err::UnknownError);
    }

    // Write master audio settings
    ret = m_projectAudioSettings->write(msczWriter, m_masterNotation->notation()->soloMuteState());
    if (!ret) {
        LOGE() << "failed write project audio settings, err: " << ret.toString();
        return ret;
    }

    // Write view settings and excerpt solo-mute states
    m_masterNotation->notation()->viewState()->write(msczWriter);
    for (IExcerptNotationPtr excerpt : m_masterNotation->excerpts()) {
        muse::io::path_t path = u"Excerpts/" + excerpt->fileName() + u"/";
        excerpt->notation()->viewState()->write(msczWriter, path);

        ByteArray soloMuteData;
        Buffer soloMuteBuf(&soloMuteData);
        soloMuteBuf.open(IODevice::WriteOnly);
        excerpt->notation()->soloMuteState()->write(&soloMuteBuf /*out*/);
        msczWriter.writeAudioSettingsJsonFile(soloMuteData, path);
    }

    return make_ret(Ret::Code::Ok);
}

Ret NotationProject::saveSelectionOnScore(const muse::io::path_t& path)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(path != m_path) {
        return make_ret(notation::Err::UnknownError);
    }

    if (m_engravingProject->masterScore()->selectionEmpty()) {
        LOGE() << "failed save, empty selection";
        return make_ret(notation::Err::EmptySelection);
    }
    // Check writable
    if (fileSystem()->exists(path) && !fileSystem()->isWritable(path)) {
        LOGE() << "failed save, not writable path: " << path;
        return make_ret(notation::Err::UnknownError);
    }

    // Write project
    std::string suffix = io::suffix(path);
    MscWriter::Params params;
    params.filePath = path.toQString();
    params.mode = mscIoModeBySuffix(suffix);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Ret::Code::InternalError);
    }

    MscWriter msczWriter(params);
    Ret ret = writeProject(msczWriter, true);

    if (ret) {
        QFile::setPermissions(path.toQString(),
                              QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    }
    LOGI() << "success save file: " << path;
    return ret;
}

Ret NotationProject::checkSavedFileForCorruption(MscIoMode ioMode, const muse::io::path_t& path,
                                                 const muse::io::path_t& scoreFileName)
{
    TRACEFUNC;

    if (ioMode != MscIoMode::Zip) {
        return muse::make_ok();
    }

    MscReader::Params params;
    params.filePath = path;
    params.mainFileName = scoreFileName.toString();
    params.mode = MscIoMode::Zip;

    MscReader msczReader(params);
    Ret ret = msczReader.open();
    if (!ret) {
        return Ret(static_cast<int>(Err::CorruptionUponSavingError),
                   muse::mtrc("project/save", "File “%1” could not be opened for validation. %2")
                   .arg(path.toString(), String(ret.toString().c_str()))
                   .toStdString());
    }

    // Try to extract the main score file.
    ByteArray scoreFile = msczReader.readScoreFile();
    msczReader.close();

    if (scoreFile.empty()) {
        return Ret(static_cast<int>(Err::CorruptionUponSavingError),
                   muse::mtrc("project/save", "“%1” is corrupted or damaged.")
                   .arg(path.toString())
                   .toStdString());
    }

    return muse::make_ok();
}

Ret NotationProject::exportProject(const muse::io::path_t& path, const std::string& suffix)
{
    TRACEFUNC;

    File file(path);
    file.open(File::WriteOnly);

    auto writer = writers()->writer(suffix);
    if (!writer) {
        LOGE() << "Unknown export format: " << suffix;
        return false;
    }

    Ret ret = writer->write(m_masterNotation->notation(), file);
    file.close();

    return ret;
}

IMasterNotationPtr NotationProject::masterNotation() const
{
    return m_masterNotation;
}

bool NotationProject::isNewlyCreated() const
{
    return m_isNewlyCreated;
}

void NotationProject::markAsNewlyCreated()
{
    m_isNewlyCreated = true;

    mu::engraving::MasterScore* masterScore = m_masterNotation->masterScore();

    QString title = masterScore->metaTag(WORK_TITLE_TAG);
    setPath(!title.isEmpty() ? title : scoreDefaultTitle());

    markAsUnsaved();

    m_masterNotation->notation()->undoStack()->stackChanged().notify();
}

bool NotationProject::isImported() const
{
    return m_isImported;
}

void NotationProject::markAsUnsaved()
{
    setNeedSave(true);
}

void NotationProject::listenIfNeedSaveChanges()
{
    m_masterNotation->notation()->undoStack()->changesChannel().onReceive(this, [this](const ScoreChangesRange&) {
        bool isStackClean = m_masterNotation && m_masterNotation->notation()->undoStack()->isStackClean();

        if (isStackClean && !m_hasNonUndoStackChanges) {
            setNeedSave(false);
        } else {
            setNeedSave(true);
        }
    });

    auto listenNonUndoStackChanges = [this](const INotationPtr& notation) {
        notation->openChanged().onNotify(this, [this]() {
            markAsUnsaved();
            m_hasNonUndoStackChanges = true;
        });

        notation->viewState()->stateChanged().onNotify(this, [this]() {
            markAsUnsaved();
            m_hasNonUndoStackChanges = true;
        });
    };

    listenNonUndoStackChanges(m_masterNotation->notation());

    for (const IExcerptNotationPtr& excerpt : m_masterNotation->excerpts()) {
        listenNonUndoStackChanges(excerpt->notation());
    }

    m_masterNotation->excerptsChanged().onNotify(this, [this, listenNonUndoStackChanges]() {
        for (const IExcerptNotationPtr& excerpt : m_masterNotation->excerpts()) {
            listenNonUndoStackChanges(excerpt->notation());
        }
    });

    m_projectAudioSettings->settingsChanged().onNotify(this, [this]() {
        markAsUnsaved();
        m_hasNonUndoStackChanges = true;
    });
}

void NotationProject::markAsSaved(const muse::io::path_t& path)
{
    TRACEFUNC;

    //! NOTE: order is important
    m_isNewlyCreated = false;

    setNeedSave(false);

    setPath(path);

    m_masterNotation->notation()->undoStack()->stackChanged().notify();
}

void NotationProject::setNeedSave(bool needSave)
{
    mu::engraving::MasterScore* score = m_masterNotation->masterScore();
    if (!score) {
        return;
    }

    setNeedAutoSave(needSave);

    bool saved = !needSave;

    if (saved) {
        m_hasNonUndoStackChanges = false;
    }

    if (score->saved() == saved) {
        return;
    }

    score->setSaved(saved);
    m_needSaveNotification.notify();
}

ValNt<bool> NotationProject::needSave() const
{
    const mu::engraving::MasterScore* score = m_masterNotation->masterScore();

    ValNt<bool> needSave;
    needSave.val = score && !score->saved();
    needSave.notification = m_needSaveNotification;

    return needSave;
}

Ret NotationProject::canSave() const
{
    if (!m_masterNotation->hasParts()) {
        return make_ret(Err::NoPartsError);
    }

    Ret ret = m_engravingProject->checkCorrupted();
    if (!ret) {
        Err errorCode = m_engravingProject->isCorruptedUponLoading() ? Err::CorruptionUponOpenningError : Err::CorruptionError;
        ret.setCode(static_cast<int>(errorCode));
    }

    return ret;
}

bool NotationProject::needAutoSave() const
{
    return m_needAutoSave;
}

void NotationProject::setNeedAutoSave(bool val)
{
    m_needAutoSave = val;
}

ProjectMeta NotationProject::metaInfo() const
{
    TRACEFUNC;

    mu::engraving::MasterScore* score = m_masterNotation->masterScore();

    ProjectMeta meta;
    meta.filePath = m_path;

    auto allTags = score->metaTags();

    meta.title = allTags[WORK_TITLE_TAG];
    meta.subtitle = allTags[SUBTITLE_TAG];
    meta.composer = allTags[COMPOSER_TAG];
    meta.arranger = allTags[ARRANGER_TAG];
    meta.lyricist = allTags[LYRICIST_TAG];
    meta.translator = allTags[TRANSLATOR_TAG];
    meta.copyright = allTags[COPYRIGHT_TAG];
    meta.creationDate = QDate::fromString(allTags[CREATION_DATE_TAG], Qt::ISODate);

    meta.partsCount = score->excerpts().size();

    meta.source = allTags[SOURCE_TAG];
    meta.audioComUrl = allTags[AUDIO_COM_URL_TAG];
    meta.platform = allTags[PLATFORM_TAG];
    meta.musescoreVersion = score->mscoreVersion();
    meta.musescoreRevision = score->mscoreRevision();
    meta.mscVersion = score->mscVersion();

    for (const String& tag : muse::keys(allTags)) {
        if (isRepresentedInProjectMeta(tag)) {
            continue;
        }

        meta.additionalTags[tag] = allTags[tag].toQString();
    }

    return meta;
}

void NotationProject::setMetaInfo(const ProjectMeta& meta, bool undoable)
{
    if (meta == metaInfo()) {
        return;
    }

    std::map<String, String> tags {
        { WORK_TITLE_TAG, meta.title },
        { SUBTITLE_TAG, meta.subtitle },
        { COMPOSER_TAG, meta.composer },
        { ARRANGER_TAG, meta.arranger },
        { LYRICIST_TAG, meta.lyricist },
        { TRANSLATOR_TAG, meta.translator },
        { COPYRIGHT_TAG, meta.copyright },
        { CREATION_DATE_TAG, meta.creationDate.toString(Qt::ISODate) },
        { SOURCE_TAG, meta.source },
        { AUDIO_COM_URL_TAG, meta.audioComUrl },
        { PLATFORM_TAG, meta.platform },
    };

    for (const QString& tag : meta.additionalTags.keys()) {
        tags[tag] = meta.additionalTags[tag].toString();
    }

    MasterScore* score = m_masterNotation->masterScore();

    if (undoable) {
        m_masterNotation->notation()->undoStack()->prepareChanges(TranslatableString("undoableAction", "Edit project properties"));
        score->undo(new mu::engraving::ChangeMetaTags(score, tags));
        m_masterNotation->notation()->undoStack()->commitChanges();
        m_masterNotation->notation()->notationChanged().notify();
    } else {
        score->setMetaTags(tags);
    }
}

IProjectAudioSettingsPtr NotationProject::audioSettings() const
{
    return m_projectAudioSettings;
}
