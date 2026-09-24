/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "convertfiletoscoremodel.h"

#include <QFileInfo>
#include <QUrl>

#include "filelistmodel.h"

#include "global/io/path.h"
#include "global/dataformatter.h"
#include "global/translation.h"

using namespace mu::project;
using namespace muse;

static QString localPath(const QString& pathOrUrl)
{
    QUrl url(pathOrUrl);
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }

    return pathOrUrl;
}

static QString formatsText(const QStringList& extensions)
{
    QStringList upperExtensions;
    upperExtensions.reserve(extensions.size());
    for (const QString& ext : extensions) {
        upperExtensions << ext.toUpper();
    }

    return upperExtensions.join(", ");
}

static QString maxFileSizeText(qint64 maxFileSizeBytes)
{
    QString size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes));
    //: %1 is a pre-formatted file size including units, e.g. "20 MB max"; shown as a short label/badge
    return muse::qtrc("project/convert", "%1 max").arg(size);
}

static QString maxCombinedFileSizeText(qint64 maxFileSizeBytes)
{
    QString size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes));
    //: %1 is a pre-formatted file size including units, e.g. "20 MB max combined"; shown as a short label/badge
    return muse::qtrc("project/convert", "%1 max combined").arg(size);
}

static QVariantMap requirementsSection(const QString& title, const QStringList& items)
{
    QVariantMap result;
    result["title"] = title;
    result["items"] = items;
    return result;
}

static bool allowsMultipleFiles(int maxCount)
{
    return maxCount <= 0 || maxCount > 1;
}

static QStringList resolveExtensionsForFilePicker(const QStringList& paths)
{
    QStringList extensions;
    for (const QString& path : paths) {
        const QString ext = QFileInfo(path).suffix().toLower();
        if (ext.isEmpty()) {
            continue;
        }

        if (ext == "jpg" || ext == "jpeg") {
            if (!extensions.contains("jpg")) {
                extensions << "jpg";
            }
            if (!extensions.contains("jpeg")) {
                extensions << "jpeg";
            }
        } else if (!extensions.contains(ext)) {
            extensions << ext;
        }
    }

    return extensions;
}

static QString pasteLinkHintTextFor(const QStringList& sources)
{
    if (sources.size() >= 2) {
        //: %1 and %2 are source names, e.g. "YouTube"; may include HTML markup (bold/link) depending on where this text is shown
        return muse::qtrc("project/convert", "Paste a link from %1 or %2 (beta)").arg(sources.at(0), sources.at(1));
    }

    if (sources.size() == 1) {
        //: %1 is a source name, e.g. "YouTube"; may include HTML markup (bold/link) depending on where this text is shown
        return muse::qtrc("project/convert", "Paste a link from %1 (beta)").arg(sources.at(0));
    }

    return QString();
}

ConvertFileToScoreModel::ConvertFileToScoreModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this)), m_fileListModel(new FileListModel(this))
{
    connect(m_fileListModel, &FileListModel::pathsChanged, this, [this]() {
        emit defaultSaveAsNameChanged();
        emit fileRequirementsChanged();
    });
}

QString ConvertFileToScoreModel::accountAvatarUrl() const
{
    return museScoreComService()->authorization()->accountInfo().avatarUrl.toString();
}

QString ConvertFileToScoreModel::guidelinesUrl() const
{
    return configuration()->scoreUploadingGuidelinesUrl().toString();
}

int ConvertFileToScoreModel::convertType() const
{
    return int(m_convertType);
}

void ConvertFileToScoreModel::setConvertType(int type)
{
    if (m_convertType == ConvertType(type)) {
        return;
    }

    m_convertType = ConvertType(type);
    emit convertTypeChanged();
    emit fileRequirementsChanged();
}

FileListModel* ConvertFileToScoreModel::fileListModel() const
{
    return m_fileListModel;
}

QString ConvertFileToScoreModel::selectedLink() const
{
    return m_selectedLink;
}

void ConvertFileToScoreModel::setSelectedLink(const QString& link)
{
    if (m_selectedLink == link) {
        return;
    }

    m_selectedLink = link;
    emit selectedLinkChanged();

    if (!link.isEmpty()) {
        setConvertType(int(ConvertType::Audio2Score));
        m_fileListModel->clear();
    }
}

QString ConvertFileToScoreModel::defaultSaveAsName() const
{
    const QStringList& paths = m_fileListModel->paths();
    if (paths.isEmpty()) {
        return muse::qtrc("project", "Untitled score");
    }

    return QFileInfo(paths.first()).completeBaseName();
}

QVariantList ConvertFileToScoreModel::fileRequirements() const
{
    const ConvertConfig& config = convertFileToScoreScenario()->config();
    const cloud::OmrConfig& omr = config.omr;
    const cloud::Audio2ScoreConfig& a2s = config.audio2score;

    //! NOTE: before a file is selected, category is Unknown, so every section is shown
    const FileCategory category = m_fileListModel->fileCategory();
    QVariantList result;

    if (category == FileCategory::Unknown || category == FileCategory::Pdf) {
        QStringList pdfItems;

        if (omr.pdf.maxFileSizeBytes > 0) {
            pdfItems << maxFileSizeText(omr.pdf.maxFileSizeBytes);
        }

        if (omr.pdf.maxPages > 0) {
            //: %n is the maximum number of pages; shown as a short label/badge, e.g. "20 pages max"
            pdfItems << muse::qtrc("project/convert", "%n page(s) max.", nullptr, omr.pdf.maxPages);
        }

        if (omr.pdf.maxFiles > 0) {
            pdfItems << muse::qtrc("project/convert", "%n file(s) per conversion", nullptr, omr.pdf.maxFiles);
        }

        result << requirementsSection(muse::qtrc("project/convert", "PDF"), pdfItems);
    }

    if (category == FileCategory::Unknown || category == FileCategory::Image) {
        QStringList imageExtensions;
        for (const QString& ext : omr.images.allowedExtensions) {
            // Display only JPG in the UI, not both JPG & JPEG
            const QString normalizedExt = ext == "jpeg" ? "jpg" : ext;
            if (!imageExtensions.contains(normalizedExt)) {
                imageExtensions << normalizedExt;
            }
        }

        QStringList imageItems;

        if (!imageExtensions.isEmpty()) {
            imageItems << formatsText(imageExtensions);
        }

        if (omr.images.maxFileSizeBytes > 0) {
            imageItems << maxCombinedFileSizeText(omr.images.maxFileSizeBytes);
        }

        if (omr.images.maxFiles > 0) {
            imageItems << muse::qtrc("project/convert", "Max. %n image(s)", nullptr, omr.images.maxFiles);
        }

        if (!imageItems.isEmpty()) {
            result << requirementsSection(muse::qtrc("project/convert", "Images"), imageItems);
        }
    }

    if (category == FileCategory::Unknown || category == FileCategory::Audio) {
        QStringList a2sItems;

        if (!a2s.file.allowedExtensions.empty()) {
            //: %1 is one or more file format names, e.g. "MP3 format" or "MP3, WAV format"
            a2sItems << muse::qtrc("project/convert", "%1 format").arg(formatsText(a2s.file.allowedExtensions));
        }

        if (a2s.file.maxFileSizeBytes > 0) {
            a2sItems << maxFileSizeText(a2s.file.maxFileSizeBytes);
        }

        if (a2s.file.maxFiles > 0) {
            a2sItems << muse::qtrc("project/convert", "%n file(s) per conversion", nullptr, a2s.file.maxFiles);
        }

        if (!a2sItems.empty()) {
            a2sItems << muse::qtrc("project/convert", "Recommended for solo arrangements only");
            result << requirementsSection(muse::qtrc("project/convert", "Audio"), a2sItems);
        }
    }

    return result;
}

bool ConvertFileToScoreModel::canSelectMultipleFiles() const
{
    if (!m_selectedLink.isEmpty()) {
        return false;
    }

    const ConvertConfig& config = convertFileToScoreScenario()->config();

    switch (m_fileListModel->fileCategory()) {
    case FileCategory::Audio:
        return allowsMultipleFiles(config.audio2score.file.maxFiles);
    case FileCategory::Pdf:
        return allowsMultipleFiles(config.omr.pdf.maxFiles);
    case FileCategory::Image:
    case FileCategory::Unknown:
        return allowsMultipleFiles(config.omr.images.maxFiles);
    }

    return false;
}

QStringList ConvertFileToScoreModel::boldLinkSources() const
{
    const cloud::Audio2ScoreLinkConfig& link = convertFileToScoreScenario()->config().audio2score.link;

    QStringList sources;
    if (link.allowedSources.testFlag(cloud::LinkSource::YouTube)) {
        sources << "<b>YouTube</b>";
    }
    if (link.allowedSources.testFlag(cloud::LinkSource::AudioCom)) {
        const QString url = audioComService()->cloudInfo().url.toString();
        sources << "<b><a href=\"" + url + "\">Audio.com</a></b>";
    }

    return sources;
}

QString ConvertFileToScoreModel::linkHintText() const
{
    const QStringList sources = boldLinkSources();

    if (sources.size() >= 2) {
        //: %1 and %2 are source names, e.g. "YouTube", with HTML markup (bold/link)
        return muse::qtrc("project/convert", "Or paste a link from %1 or %2 (beta)").arg(sources.at(0), sources.at(1));
    }

    if (sources.size() == 1) {
        //: %1 is a source name, e.g. "YouTube", with HTML markup (bold/link)
        return muse::qtrc("project/convert", "Or paste a link from %1 (beta)").arg(sources.at(0));
    }

    return QString();
}

QString ConvertFileToScoreModel::audioComUrl() const
{
    const cloud::Audio2ScoreLinkConfig& link = convertFileToScoreScenario()->config().audio2score.link;
    if (!link.allowedSources.testFlag(cloud::LinkSource::AudioCom)) {
        return QString();
    }

    return audioComService()->cloudInfo().url.toString();
}

QString ConvertFileToScoreModel::linkPageHintText() const
{
    return pasteLinkHintTextFor(boldLinkSources());
}

QString ConvertFileToScoreModel::linkPageHintPlainText() const
{
    const cloud::Audio2ScoreLinkConfig& link = convertFileToScoreScenario()->config().audio2score.link;

    QStringList names;
    if (link.allowedSources.testFlag(cloud::LinkSource::YouTube)) {
        names << "YouTube";
    }
    if (link.allowedSources.testFlag(cloud::LinkSource::AudioCom)) {
        names << "Audio.com";
    }

    return pasteLinkHintTextFor(names);
}

int ConvertFileToScoreModel::maxLinkLength() const
{
    return convertFileToScoreScenario()->config().audio2score.link.maxLength;
}

void ConvertFileToScoreModel::load(const QStringList& paths, int type)
{
    setSelectedLink(QString());
    m_fileListModel->load(paths, convertFileToScoreScenario()->config());
    setConvertType(type);
}

QStringList ConvertFileToScoreModel::selectFiles(const QStringList& existingPaths)
{
    const ConvertConfig& config = convertFileToScoreScenario()->config();

    QStringList extensions = existingPaths.isEmpty()
                             ? QStringList { "pdf" } + config.omr.images.allowedExtensions + config.audio2score.file.allowedExtensions
    : resolveExtensionsForFilePicker(existingPaths);

    QStringList patterns;
    patterns.reserve(extensions.size());
    for (const QString& ext : extensions) {
        patterns << "*." + ext.toLower();
    }

    const std::vector<std::string> filters {
        muse::trc("project", "All supported files") + " (" + patterns.join(' ').toStdString() + ")",
        muse::trc("project", "All") + " (*)"
    };

    const FileCategory category = existingPaths.isEmpty()
                                  ? FileCategory::Unknown
                                  : fileCategoryFromPath(io::path_t(existingPaths.first()));

    bool multiple = false;
    switch (category) {
    case FileCategory::Pdf:
        multiple = allowsMultipleFiles(config.omr.pdf.maxFiles);
        break;
    case FileCategory::Image:
        multiple = allowsMultipleFiles(config.omr.images.maxFiles);
        break;
    case FileCategory::Audio:
        multiple = allowsMultipleFiles(config.audio2score.file.maxFiles);
        break;
    case FileCategory::Unknown:
        multiple = allowsMultipleFiles(config.omr.pdf.maxFiles)
                   || allowsMultipleFiles(config.omr.images.maxFiles)
                   || allowsMultipleFiles(config.audio2score.file.maxFiles);
        break;
    }

    io::paths_t files;
    if (multiple) {
        files = interactive()->selectOpeningFilesSync(muse::trc("ui", "Choose file"),
                                                      configuration()->defaultConvertFilePath(), filters);
    } else {
        io::path_t file = interactive()->selectOpeningFileSync(muse::trc("ui", "Choose file"),
                                                               configuration()->defaultConvertFilePath(), filters);
        if (!file.empty()) {
            files.push_back(file);
        }
    }

    if (!files.empty()) {
        configuration()->setLastOpenedConvertFilePath(io::dirpath(files.back()));
    }

    QStringList paths;
    paths.reserve(files.size());
    for (const io::path_t& file : files) {
        paths << file.toQString();
    }

    return paths;
}

bool ConvertFileToScoreModel::selectAndValidateFiles()
{
    QStringList files = selectFiles(m_fileListModel->paths());
    if (files.isEmpty()) {
        return false;
    }

    return validateAndAddFiles(files);
}

bool ConvertFileToScoreModel::validateAndAddFiles(const QStringList& pathsOrUrls)
{
    QStringList newPaths;
    newPaths.reserve(pathsOrUrls.size());

    for (const QString& pathOrUrl : pathsOrUrls) {
        newPaths << localPath(pathOrUrl);
    }

    const QStringList allPaths = m_fileListModel->paths() + newPaths;

    io::paths_t ioPaths;
    ioPaths.reserve(allPaths.size());
    for (const QString& path : allPaths) {
        ioPaths.push_back(io::path_t(path));
    }

    RetVal<ConvertFilesValidation> result = convertFileToScoreScenario()->validateFiles(ioPaths);
    if (!result.ret) {
        return false;
    }

    m_fileListModel->setPaths(allPaths, result.val.category);
    setSelectedLink(QString());
    setConvertType(int(result.val.type));

    return true;
}

bool ConvertFileToScoreModel::validateLink(const QString& link) const
{
    return convertFileToScoreScenario()->validateLink(QUrl(link)).success();
}

QString ConvertFileToScoreModel::validateFileName(const QString& name) const
{
    if (name.isEmpty()) {
        return QString();
    }

    if (!io::isAllowedFileName(io::path_t(name))) {
        return muse::qtrc("project/convert", "“%1” cannot be used as a file name. Please choose a different name.").arg(name);
    }

    return QString();
}

void ConvertFileToScoreModel::confirmCancel()
{
    constexpr int noBtn = int(IInteractive::Button::No);
    constexpr int yesBtn = int(IInteractive::Button::Yes);

    IInteractive::ButtonData stay(noBtn, muse::trc("project/convert", "No, stay here"));
    stay.role = IInteractive::ButtonRole::RejectRole;

    IInteractive::ButtonData cancel(yesBtn, muse::trc("project/convert", "Yes, cancel"), /*accent*/ true);
    cancel.role = IInteractive::ButtonRole::AcceptRole;

    interactive()->question(
        muse::trc("project/convert", "Are you sure you want to cancel?"),
        muse::trc("project/convert", "Your current selection will be lost."),
        { stay, cancel }, noBtn, IInteractive::WithIcon)
    .onResolve(this, [this, yesBtn](const IInteractive::Result& result) {
        if (result.isButton(yesBtn)) {
            emit cancelConfirmed();
        }
    });
}

void ConvertFileToScoreModel::confirmGoingBack()
{
    constexpr int noBtn = int(IInteractive::Button::No);
    constexpr int yesBtn = int(IInteractive::Button::Yes);

    IInteractive::ButtonData stay(noBtn, muse::trc("project/convert", "No, stay here"));
    stay.role = IInteractive::ButtonRole::RejectRole;

    IInteractive::ButtonData goBack(yesBtn, muse::trc("project/convert", "Yes, go back"), /*accent*/ true);
    goBack.role = IInteractive::ButtonRole::AcceptRole;

    interactive()->question(
        muse::trc("project/convert", "Are you sure you want to go back?"),
        muse::trc("project/convert", "Your current selection will be lost."),
        { stay, goBack }, noBtn, IInteractive::WithIcon)
    .onResolve(this, [this, yesBtn](const IInteractive::Result& result) {
        if (result.isButton(yesBtn)) {
            emit goingBackConfirmed();
        }
    });
}

void ConvertFileToScoreModel::clearSelection()
{
    setSelectedLink(QString());
    m_fileListModel->clear();
}
