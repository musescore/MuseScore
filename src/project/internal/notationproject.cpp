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
#include <QFile>

#include "engraving/engravingproject.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/mscxcompat.h"
#include "engraving/infrastructure/io/mscio.h"
#include "engraving/engravingerrors.h"
#include "engraving/style/defaultstyle.h"

#include "notation/notationerrors.h"
#include "projectaudiosettings.h"
#include "projectfileinfoprovider.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

static const QString WORK_TITLE_TAG("workTitle");
static const QString WORK_NUMBER_TAG("workNumber");
static const QString SUBTITLE_TAG("subtitle");
static const QString COMPOSER_TAG("composer");
static const QString LYRICIST_TAG("lyricist");
static const QString POET_TAG("poet");
static const QString SOURCE_TAG("source");
static const QString COPYRIGHT_TAG("copyright");
static const QString TRANSLATOR_TAG("translator");
static const QString ARRANGER_TAG("arranger");
static const QString CREATION_DATE_TAG("creationDate");
static const QString PLATFORM_TAG("platform");
static const QString MOVEMENT_TITLE_TAG("movementTitle");
static const QString MOVEMENT_NUMBER_TAG("movementNumber");

static bool isStandardTag(const QString& tag)
{
    static const QSet<QString> standardTags {
        WORK_TITLE_TAG,
        WORK_NUMBER_TAG,
        SUBTITLE_TAG,
        COMPOSER_TAG,
        LYRICIST_TAG,
        POET_TAG,
        SOURCE_TAG,
        COPYRIGHT_TAG,
        TRANSLATOR_TAG,
        ARRANGER_TAG,
        CREATION_DATE_TAG,
        PLATFORM_TAG,
        MOVEMENT_NUMBER_TAG,
        MOVEMENT_TITLE_TAG
    };

    return standardTags.contains(tag);
}

static void setupScoreMetaTags(Ms::MasterScore* masterScore, const ProjectCreateOptions& projectOptions)
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
}

NotationProject::NotationProject()
{
    m_engravingProject = EngravingProject::create();
    m_engravingProject->setFileInfoProvider(std::make_shared<ProjectFileInfoProvider>(this));
    m_masterNotation = std::shared_ptr<MasterNotation>(new MasterNotation());
    m_masterNotation->needSave().notification.onNotify(this, [this]() {
        m_needSaveNotification.notify();
    });

    m_projectAudioSettings = std::shared_ptr<ProjectAudioSettings>(new ProjectAudioSettings());
    m_projectAudioSettings->needSave().notification.onNotify(this, [this]() {
        m_needSaveNotification.notify();
    });

    m_viewSettings = std::shared_ptr<ProjectViewSettings>(new ProjectViewSettings());
    m_viewSettings->needSave().notification.onNotify(this, [this]() {
        m_needSaveNotification.notify();
    });
}

NotationProject::~NotationProject()
{
    m_viewSettings = nullptr;
    m_projectAudioSettings = nullptr;
    m_masterNotation = nullptr;
    m_engravingProject = nullptr;
}

mu::Ret NotationProject::load(const io::path& path, const io::path& stylePath, bool forceMode)
{
    TRACEFUNC;

    LOGD() << "try load: " << path;

    setPath(path);

    std::string suffix = io::suffix(path);
    if (!isMuseScoreFile(suffix)) {
        return doImport(path, stylePath, forceMode);
    }

    MscReader::Params params;

    bool needRestoreUnsavedChanges = projectAutoSaver()->projectHasUnsavedChanges(path);
    if (needRestoreUnsavedChanges) {
        params.filePath = projectAutoSaver()->projectAutoSavePath(path).toQString();
    } else {
        params.filePath = path.toQString();
    }

    params.mode = mscIoModeBySuffix(suffix);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Ret::Code::InternalError);
    }

    MscReader reader(params);
    if (!reader.open()) {
        return make_ret(engraving::Err::FileOpenError);
    }

    Ret ret = doLoad(reader, stylePath, forceMode);
    if (!ret) {
        return ret;
    }

    m_masterNotation->masterScore()->setNewlyCreated(false);
    m_masterNotation->masterScore()->setSaved(!needRestoreUnsavedChanges);

    return ret;
}

mu::Ret NotationProject::doLoad(engraving::MscReader& reader, const io::path& stylePath, bool forceMode)
{
    TRACEFUNC;

    // Load engraving project
    m_engravingProject->setFileInfoProvider(std::make_shared<ProjectFileInfoProvider>(this));
    engraving::Err err = m_engravingProject->loadMscz(reader, forceMode);

    if (err != engraving::Err::NoError) {
        return make_ret(err);
    }

    // Migration
    if (migrator()) {
        Ret ret = migrator()->migrateEngravingProjectIfNeed(m_engravingProject);
        if (!ret) {
            return ret;
        }
    }

    // Setup master score
    err = m_engravingProject->setupMasterScore();
    if (err != engraving::Err::NoError) {
        return make_ret(err);
    }

    // Load style if present
    if (!stylePath.empty()) {
        m_engravingProject->masterScore()->loadStyle(stylePath.toQString());
        if (!Ms::MScore::lastError.isEmpty()) {
            LOGE() << Ms::MScore::lastError;
        }
    }

    // Load other stuff from the project file
    Ret ret = m_projectAudioSettings->read(reader);
    if (!ret) {
        return ret;
    }

    ret = m_viewSettings->read(reader);
    if (!ret) {
        return ret;
    }

    // Set current if all success
    m_masterNotation->setMasterScore(m_engravingProject->masterScore());

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::doImport(const io::path& path, const io::path& stylePath, bool forceMode)
{
    TRACEFUNC;

    // Find import reader
    std::string suffix = io::suffix(path);
    INotationReaderPtr scoreReader = readers()->reader(suffix);
    if (!scoreReader) {
        return make_ret(notation::Err::FileUnknownType, path);
    }

    // Setup import reader
    INotationReader::Options options;
    if (forceMode) {
        options[INotationReader::OptionKey::ForceMode] = forceMode;
    }

    // Read(import) master score
    Ms::ScoreLoad sl;
    m_engravingProject->setFileInfoProvider(std::make_shared<ProjectFileInfoProvider>(this));
    Ms::MasterScore* score = m_engravingProject->masterScore();
    Ret ret = scoreReader->read(score, path, options);
    if (!ret) {
        return ret;
    }

    if (!Ms::MScore::lastError.isEmpty()) {
        LOGE() << Ms::MScore::lastError;
    }

    // Setup master score
    engraving::Err err = m_engravingProject->setupMasterScore();
    if (err != engraving::Err::NoError) {
        return make_ret(err);
    }

    // Load style if present
    if (!stylePath.empty()) {
        score->loadStyle(stylePath.toQString());
        if (!Ms::MScore::lastError.isEmpty()) {
            LOGE() << Ms::MScore::lastError;
        }
    }

    // Setup other stuff
    m_projectAudioSettings->makeDefault();
    m_viewSettings->makeDefault();

    // Set current if all success
    m_masterNotation->setMasterScore(score);
    setPath(path);
    score->setSaved(true);
    score->setNewlyCreated(true);

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::createNew(const ProjectCreateOptions& projectOptions)
{
    TRACEFUNC;

    // Load template if present
    if (!projectOptions.templatePath.empty()) {
        return loadTemplate(projectOptions);
    }

    // Create new engraving project
    setPath(projectOptions.title.isEmpty() ? qtrc("project", "Untitled") : projectOptions.title);
    m_engravingProject->setFileInfoProvider(std::make_shared<ProjectFileInfoProvider>(this));

    Ms::MasterScore* masterScore = m_engravingProject->masterScore();
    setupScoreMetaTags(masterScore, projectOptions);

    // Setup new master score
    m_masterNotation->undoStack()->lock();

    Ret ret = m_masterNotation->setupNewScore(masterScore, projectOptions.scoreOptions);
    if (!ret) {
        m_masterNotation->undoStack()->unlock();
        return ret;
    }

    m_masterNotation->undoStack()->unlock();

    // Setup other stuff
    m_projectAudioSettings->makeDefault();
    m_viewSettings->makeDefault();

    masterScore->setSaved(true);
    masterScore->setNewlyCreated(true);

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::loadTemplate(const ProjectCreateOptions& projectOptions)
{
    TRACEFUNC;

    Ret ret = load(projectOptions.templatePath);

    if (ret) {
        setPath(projectOptions.title.isEmpty() ? qtrc("project", "Untitled") : projectOptions.title);

        Ms::MasterScore* masterScore = m_masterNotation->masterScore();
        setupScoreMetaTags(masterScore, projectOptions);

        m_masterNotation->undoStack()->lock();
        m_masterNotation->applyOptions(masterScore, projectOptions.scoreOptions, true /*createdFromTemplate*/);
        m_masterNotation->undoStack()->unlock();

        masterScore->setSaved(true);
        masterScore->setNewlyCreated(true);
    }

    return ret;
}

io::path NotationProject::path() const
{
    return m_path;
}

async::Notification NotationProject::pathChanged() const
{
    return m_pathChanged;
}

void NotationProject::setPath(const io::path& path)
{
    if (m_path == path) {
        return;
    }

    m_path = path;
    m_pathChanged.notify();
}

QString NotationProject::displayName() const
{
    if (isNewlyCreated()) {
        if (m_path.empty()) {
            QString workTitle = m_masterNotation->workTitle();
            if (workTitle.isEmpty()) {
                return qtrc("project", "Untitled");
            }
            return workTitle;
        }
        return io::filename(m_path).toQString();
    }

    if (isCloudProject()) {
        // TODO(save-to-cloud)
    }

    bool isSuffixInteresting = io::suffix(m_path) != engraving::MSCZ;
    return io::filename(m_path, isSuffixInteresting).toQString();
}

bool NotationProject::isCloudProject() const
{
    return configuration()->isCloudProject(m_path);
}

mu::Ret NotationProject::save(const io::path& path, SaveMode saveMode)
{
    TRACEFUNC;
    switch (saveMode) {
    case SaveMode::SaveSelection:
        return saveSelectionOnScore(path);
    case SaveMode::Save:
    case SaveMode::SaveAs:
    case SaveMode::SaveCopy: {
        io::path savePath = path;
        if (savePath.empty()) {
            IF_ASSERT_FAILED(!m_path.empty()) {
                return false;
            }

            savePath = m_path;
        }

        std::string suffix = io::suffix(savePath);

        Ret ret = saveScore(savePath, suffix);
        if (ret) {
            if (saveMode != SaveMode::SaveCopy) {
                setPath(savePath);
                m_masterNotation->masterScore()->setNewlyCreated(false);
                m_masterNotation->masterScore()->setSaved(true);
                m_masterNotation->undoStack()->stackChanged().notify();
            }
        }

        return ret;
    }
    case SaveMode::AutoSave:
        io::path originalPath = projectAutoSaver()->projectOriginalPath(path);
        std::string suffix = io::suffix(originalPath);

        return saveScore(path, suffix);
    }

    return make_ret(notation::Err::UnknownError);
}

mu::Ret NotationProject::writeToDevice(io::Device* device)
{
    IF_ASSERT_FAILED(!m_path.empty()) {
        return make_ret(notation::Err::UnknownError);
    }

    MscWriter::Params params;
    params.device = device;
    params.filePath = m_path.toQString();
    params.mode = MscIoMode::Zip;

    MscWriter msczWriter(params);
    msczWriter.open();

    Ret ret = writeProject(msczWriter, false);
    return ret;
}

mu::Ret NotationProject::saveScore(const io::path& path, const std::string& fileSuffix)
{
    if (!isMuseScoreFile(fileSuffix) && !fileSuffix.empty()) {
        return exportProject(path, fileSuffix);
    }

    MscIoMode ioMode = mscIoModeBySuffix(fileSuffix);

    return doSave(path, true, ioMode);
}

mu::Ret NotationProject::doSave(const io::path& path, bool generateBackup, engraving::MscIoMode ioMode)
{
    QString currentPath = path.toQString();
    QString savePath = currentPath + "_saving";

    // Step 1: check writable
    {
        QFileInfo fi(savePath);
        if (fi.exists() && !QFileInfo(savePath).isWritable()) {
            LOGE() << "failed save, not writable path: " << savePath;
            return make_ret(notation::Err::UnknownError);
        }
    }

    // Step 2: write project
    {
        MscWriter::Params params;
        params.filePath = savePath;
        params.mode = ioMode;
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
    }

    // Step 3: create backup if need
    {
        if (generateBackup) {
            makeCurrentFileAsBackup();
        }
    }

    // Step 4: replace to saved file
    {
        Ret ret = fileSystem()->move(savePath, currentPath, true);
        if (!ret) {
            return ret;
        }
    }

    // make file readable by all
    {
        QFile::setPermissions(currentPath,
                              QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    }

    LOGI() << "success save file: " << currentPath;
    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::makeCurrentFileAsBackup()
{
    if (isNewlyCreated()) {
        LOGD() << "project just created";
        return make_ret(Ret::Code::Ok);
    }

    io::path filePath = m_path;
    if (io::suffix(filePath) != engraving::MSCZ) {
        LOGW() << "backup allowed only for MSCZ, currently: " << filePath;
        return make_ret(Ret::Code::Ok);
    }

    Ret ret = fileSystem()->exists(filePath);
    if (ret) {
        LOGE() << "project file does not exist";
        return ret;
    }

    io::path backupFilePath = filePath + "~";
    ret = fileSystem()->move(filePath, backupFilePath, true);
    if (!ret) {
        LOGE() << "failed to move from: " << filePath << ", to: " << backupFilePath;
        return ret;
    }

    fileSystem()->setAttribute(backupFilePath, system::IFileSystem::Attribute::Hidden);

    return ret;
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

    ret = m_viewSettings->write(msczWriter);
    if (!ret) {
        LOGE() << "failed write project view settings, err: " << ret.toString();
        return ret;
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret NotationProject::saveSelectionOnScore(const mu::io::path& path)
{
    IF_ASSERT_FAILED(path != m_path) {
        return make_ret(notation::Err::UnknownError);
    }

    if (m_engravingProject->masterScore()->selectionEmpty()) {
        LOGE() << "failed save, empty selection";
        return make_ret(notation::Err::EmptySelection);
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
    params.filePath = path.toQString();
    params.mode = mscIoModeBySuffix(suffix);
    IF_ASSERT_FAILED(params.mode != MscIoMode::Unknown) {
        return make_ret(Ret::Code::InternalError);
    }

    MscWriter msczWriter(params);
    Ret ret = writeProject(msczWriter, true);

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

bool NotationProject::isNewlyCreated() const
{
    return m_masterNotation->isNewlyCreated();
}

mu::ValNt<bool> NotationProject::needSave() const
{
    ValNt<bool> needSave;
    needSave.val = m_masterNotation->needSave().val
                   || m_projectAudioSettings->needSave().val
                   || m_viewSettings->needSave().val;
    needSave.notification = m_needSaveNotification;

    return needSave;
}

ProjectMeta NotationProject::metaInfo() const
{
    TRACEFUNC;

    Ms::MasterScore* score = m_masterNotation->masterScore();

    ProjectMeta meta;
    auto allTags = score->metaTags();

    meta.title = allTags[WORK_TITLE_TAG];
    meta.subtitle = allTags[SUBTITLE_TAG];
    meta.composer = allTags[COMPOSER_TAG];
    meta.lyricist = allTags[LYRICIST_TAG];
    meta.copyright = allTags[COPYRIGHT_TAG];
    meta.translator = allTags[TRANSLATOR_TAG];
    meta.arranger = allTags[ARRANGER_TAG];
    meta.source = allTags[SOURCE_TAG];
    meta.creationDate = QDate::fromString(allTags[CREATION_DATE_TAG], Qt::ISODate);
    meta.platform = allTags[PLATFORM_TAG];
    meta.musescoreVersion = score->mscoreVersion();
    meta.musescoreRevision = score->mscoreRevision();
    meta.mscVersion = score->mscVersion();

    for (const QString& tag : allTags.keys()) {
        if (isStandardTag(tag)) {
            continue;
        }

        meta.additionalTags[tag] = allTags[tag];
    }

    meta.filePath = m_path;

    meta.partsCount = score->excerpts().count();

    return meta;
}

void NotationProject::setMetaInfo(const ProjectMeta& meta)
{
    QMap<QString, QString> tags {
        { WORK_TITLE_TAG, meta.title },
        { SUBTITLE_TAG, meta.subtitle },
        { COMPOSER_TAG, meta.composer },
        { LYRICIST_TAG, meta.lyricist },
        { COPYRIGHT_TAG, meta.copyright },
        { TRANSLATOR_TAG, meta.translator },
        { ARRANGER_TAG, meta.arranger },
        { SOURCE_TAG, meta.source },
        { PLATFORM_TAG, meta.platform },
        { CREATION_DATE_TAG, meta.creationDate.toString(Qt::ISODate) }
    };

    for (const QString& tag : meta.additionalTags.keys()) {
        tags[tag] = meta.additionalTags[tag].toString();
    }

    Ms::MasterScore* score = m_masterNotation->masterScore();
    score->setMetaTags(tags);
}

IProjectAudioSettingsPtr NotationProject::audioSettings() const
{
    return m_projectAudioSettings;
}

IProjectViewSettingsPtr NotationProject::viewSettings() const
{
    return m_viewSettings;
}
