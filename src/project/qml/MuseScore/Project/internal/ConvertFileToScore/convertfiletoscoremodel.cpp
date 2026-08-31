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
    return muse::qtrc("project/convert", "%1 max").arg(size);
}

static QString maxCombinedFileSizeText(qint64 maxFileSizeBytes)
{
    QString size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes));
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

static bool isPdf(const QString& extOrPath)
{
    return extOrPath.compare("pdf", Qt::CaseInsensitive) == 0 || extOrPath.endsWith(".pdf", Qt::CaseInsensitive);
}

static QStringList resolveExtensions(const QStringList& paths)
{
    QStringList extensions;
    for (const QString& path : paths) {
        QString ext = QFileInfo(path).suffix();
        if (!ext.isEmpty() && !extensions.contains(ext, Qt::CaseInsensitive)) {
            extensions << ext;
        }
    }

    return extensions;
}

static QString pasteLinkHintTextFor(const QStringList& sources)
{
    if (sources.size() >= 2) {
        return muse::qtrc("project/convert", "Paste a link from %1 or %2 (beta)").arg(sources.at(0), sources.at(1));
    }

    if (sources.size() == 1) {
        return muse::qtrc("project/convert", "Paste a link from %1 (beta)").arg(sources.at(0));
    }

    return QString();
}

ConvertFileToScoreModel::ConvertFileToScoreModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
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

FileCategory ConvertFileToScoreModel::selectedFileCategory() const
{
    if (!m_selectedLink.isEmpty()) {
        return FileCategory::Audio;
    }

    if (m_selectedPaths.isEmpty()) {
        return FileCategory::Unknown;
    }

    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();
    return fileCategoryFromPath(io::path_t(m_selectedPaths.first()), config);
}

QStringList ConvertFileToScoreModel::selectedPaths() const
{
    return m_selectedPaths;
}

void ConvertFileToScoreModel::setSelectedPaths(const QStringList& paths)
{
    if (m_selectedPaths == paths) {
        return;
    }

    m_selectedPaths = paths;
    emit selectedPathsChanged();
    emit fileRequirementsChanged();
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
        setSelectedPaths({});
    }
}

QString ConvertFileToScoreModel::defaultSaveAsName() const
{
    if (m_selectedPaths.isEmpty()) {
        return muse::qtrc("project", "Untitled score");
    }

    return QFileInfo(m_selectedPaths.first()).completeBaseName();
}

QVariantList ConvertFileToScoreModel::fileRequirements() const
{
    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();
    const cloud::OmrConfig& omr = config.omr;
    const cloud::Audio2ScoreConfig& a2s = config.audio2score;

    //! NOTE: before a file is selected, category is Unknown, so every section is shown
    const FileCategory category = selectedFileCategory();
    QVariantList result;

    if (category == FileCategory::Unknown || category == FileCategory::Pdf) {
        QStringList pdfItems;

        if (omr.maxFileSizeBytes > 0) {
            pdfItems << maxFileSizeText(omr.maxFileSizeBytes);
        }

        if (omr.maxPages > 0) {
            pdfItems << muse::qtrc("project/convert", "%1 pages max").arg(omr.maxPages);
        }

        if (!pdfItems.isEmpty()) {
            result << requirementsSection(muse::qtrc("project/convert", "PDF"), pdfItems);
        }
    }

    if (category == FileCategory::Unknown || category == FileCategory::Image) {
        QStringList imageExtensions;
        for (const QString& ext : omr.allowedExtensions) {
            if (!isPdf(ext)) {
                imageExtensions << ext;
            }
        }

        QStringList imageItems;

        if (!imageExtensions.isEmpty()) {
            imageItems << formatsText(imageExtensions);
        }

        if (omr.maxFileSizeBytes > 0) {
            imageItems << maxCombinedFileSizeText(omr.maxFileSizeBytes);
        }

        if (omr.maxImages > 0) {
            imageItems << muse::qtrc("project/convert", "Max %1 images").arg(omr.maxImages);
        }

        if (!imageItems.isEmpty()) {
            result << requirementsSection(muse::qtrc("project/convert", "Images"), imageItems);
        }
    }

    if (category == FileCategory::Unknown || category == FileCategory::Audio) {
        QStringList a2sItems;

        if (!a2s.allowedExtensions.empty()) {
            a2sItems << muse::qtrc("project/convert", "%1 format").arg(formatsText(a2s.allowedExtensions));
        }

        if (a2s.maxFileSizeBytes > 0) {
            a2sItems << maxFileSizeText(a2s.maxFileSizeBytes);
        }

        if (a2s.maxFiles == 1) {
            a2sItems << muse::qtrc("project/convert", "1 file per upload");
        } else if (a2s.maxFiles > 1) {
            a2sItems << muse::qtrc("project/convert", "%1 files per upload").arg(a2s.maxFiles);
        }

        if (!a2sItems.empty()) {
            result << requirementsSection(muse::qtrc("project/convert", "Audio"), a2sItems);
        }
    }

    return result;
}

QVariantMap ConvertFileToScoreModel::convertLimits() const
{
    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();

    QVariantMap result;

    if (m_convertType == ConvertType::Audio2Score) {
        result["maxFileCount"] = config.audio2score.maxFiles;
        result["maxCombinedSizeBytes"] = config.audio2score.maxFileSizeBytes;
    } else {
        result["maxFileCount"] = config.omr.maxImages;
        result["maxCombinedSizeBytes"] = config.omr.maxFileSizeBytes;
    }

    return result;
}

bool ConvertFileToScoreModel::canSelectMultipleFiles() const
{
    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();

    if (m_convertType == ConvertType::Audio2Score) {
        return allowsMultipleFiles(config.audio2score.maxFiles);
    }

    if (selectedFileCategory() == FileCategory::Pdf) {
        return false;
    }

    return allowsMultipleFiles(config.omr.maxImages);
}

QStringList ConvertFileToScoreModel::boldLinkSources() const
{
    const cloud::Audio2ScoreConfig& a2s = convertFileToScoreScenario()->convertConfig().audio2score;

    QStringList sources;
    if (a2s.allowedLinkSources.testFlag(cloud::LinkSource::YouTube)) {
        sources << "<b>YouTube</b>";
    }
    if (a2s.allowedLinkSources.testFlag(cloud::LinkSource::AudioCom)) {
        const QString url = audioComService()->cloudInfo().url.toString();
        sources << "<b><a href=\"" + url + "\">Audio.com</a></b>";
    }

    return sources;
}

QString ConvertFileToScoreModel::linkHintText() const
{
    const QStringList sources = boldLinkSources();

    if (sources.size() >= 2) {
        return muse::qtrc("project/convert", "Or paste a link from %1 or %2 (beta)").arg(sources.at(0), sources.at(1));
    }

    if (sources.size() == 1) {
        return muse::qtrc("project/convert", "Or paste a link from %1 (beta)").arg(sources.at(0));
    }

    return QString();
}

QString ConvertFileToScoreModel::linkPageHintText() const
{
    return pasteLinkHintTextFor(boldLinkSources());
}

QString ConvertFileToScoreModel::linkPageHintPlainText() const
{
    const cloud::Audio2ScoreConfig& a2s = convertFileToScoreScenario()->convertConfig().audio2score;

    QStringList names;
    if (a2s.allowedLinkSources.testFlag(cloud::LinkSource::YouTube)) {
        names << "YouTube";
    }
    if (a2s.allowedLinkSources.testFlag(cloud::LinkSource::AudioCom)) {
        names << "Audio.com";
    }

    return pasteLinkHintTextFor(names);
}

int ConvertFileToScoreModel::maxLinkLength() const
{
    return convertFileToScoreScenario()->convertConfig().audio2score.maxLinkLength;
}

QStringList ConvertFileToScoreModel::selectFiles(const QStringList& existingPaths)
{
    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();

    QStringList extensions = existingPaths.isEmpty()
                             ? config.omr.allowedExtensions + config.audio2score.allowedExtensions
                             : resolveExtensions(existingPaths);

    if (extensions.isEmpty()) {
        extensions = { "pdf", "jpg", "jpeg", "png", "mp3" }; // fallback
    }

    QStringList patterns;
    patterns.reserve(extensions.size());
    for (const QString& ext : extensions) {
        patterns << "*." + ext.toLower();
    }

    const std::vector<std::string> filters {
        muse::trc("project/convert", "Supported files") + " (" + patterns.join(' ').toStdString() + ")",
        muse::trc("project/convert", "All files") + " (*)"
    };

    const bool multiple = allowsMultipleFiles(config.omr.maxImages) || allowsMultipleFiles(config.audio2score.maxFiles);

    io::paths_t files;
    if (multiple) {
        files = interactive()->selectOpeningFilesSync(muse::trc("project/convert", "Choose file"),
                                                      configuration()->defaultOpenProjectsPath(), filters);
    } else {
        io::path_t file = interactive()->selectOpeningFileSync(muse::trc("project/convert", "Choose file"),
                                                               configuration()->defaultOpenProjectsPath(), filters);
        if (!file.empty()) {
            files.push_back(file);
        }
    }

    QStringList paths;
    paths.reserve(files.size());
    for (const io::path_t& file : files) {
        paths << file.toQString();
    }

    return paths;
}

bool ConvertFileToScoreModel::selectAndValidateFiles(const QStringList& existingPaths)
{
    QStringList files = selectFiles(existingPaths);
    if (files.isEmpty()) {
        return false;
    }

    return validateAndApplyFiles(existingPaths + files);
}

bool ConvertFileToScoreModel::validateAndApplyFiles(const QStringList& pathsOrUrls)
{
    io::paths_t ioPaths;
    ioPaths.reserve(pathsOrUrls.size());

    QStringList localPaths;
    localPaths.reserve(pathsOrUrls.size());

    for (const QString& pathOrUrl : pathsOrUrls) {
        QString path = localPath(pathOrUrl);
        ioPaths.push_back(io::path_t(path));
        localPaths << path;
    }

    RetVal<ConvertType> result = convertFileToScoreScenario()->validateFiles(ioPaths);
    if (!result.ret) {
        return false;
    }

    setSelectedPaths(localPaths);
    setSelectedLink(QString());
    setConvertType(int(result.val));

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

void ConvertFileToScoreModel::confirmGoingBack()
{
    constexpr int noBtn = int(IInteractive::Button::No);
    constexpr int yesBtn = int(IInteractive::Button::Yes);

    IInteractive::ButtonData stay(noBtn, muse::trc("project/convert", "No, stay here"));
    IInteractive::ButtonData goBack(yesBtn, muse::trc("project/convert", "Yes, go back"), /*accent*/ true);

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
    setSelectedPaths({});
}
