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

static QString joinWithOr(QStringList items)
{
    if (items.size() <= 1) {
        return items.join(QString());
    }

    const QString last = items.takeLast();
    const QString beforeLastSeparator = items.size() > 1 ? QString(", ") : QString(" ");

    return items.join(", ") + beforeLastSeparator + muse::qtrc("global", "or") + " " + last;
}

static QString formatsText(const QStringList& extensions)
{
    QStringList upperExtensions;
    upperExtensions.reserve(extensions.size());
    for (const QString& ext : extensions) {
        upperExtensions << ext.toUpper();
    }

    return joinWithOr(upperExtensions);
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

static QString combinedImagesNoteText()
{
    return muse::qtrc("project/convert", "Images will be combined into one score in the order you upload them");
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

static bool containsPdf(const QStringList& extensionsOrPaths)
{
    for (const QString& item : extensionsOrPaths) {
        if (isPdf(item)) {
            return true;
        }
    }

    return false;
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

QVariantList ConvertFileToScoreModel::fileRequirements() const
{
    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();
    const cloud::OmrConfig& omr = config.omr;
    const cloud::Audio2ScoreConfig& a2s = config.audio2score;

    QVariantList result;

    QStringList imageExtensions;
    for (const QString& ext : omr.allowedExtensions) {
        if (!isPdf(ext)) {
            imageExtensions << ext;
        }
    }

    if (containsPdf(omr.allowedExtensions)) {
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
        imageItems << combinedImagesNoteText();
        result << requirementsSection(muse::qtrc("project/convert", "Images"), imageItems);
    }

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

QString ConvertFileToScoreModel::combinedFilesNote() const
{
    if (m_convertType != ConvertType::Omr) {
        return QString();
    }

    return combinedImagesNoteText();
}

bool ConvertFileToScoreModel::canSelectMultipleFiles() const
{
    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();

    if (m_convertType == ConvertType::Audio2Score) {
        return allowsMultipleFiles(config.audio2score.maxFiles);
    }

    if (containsPdf(m_selectedPaths)) {
        return false;
    }

    return allowsMultipleFiles(config.omr.maxImages);
}

static QString linkHintTextFor(const QStringList& sources)
{
    if (sources.isEmpty()) {
        return QString();
    }

    return muse::qtrc("project/convert", "Or paste a link from %1 (beta)").arg(joinWithOr(sources));
}

static QStringList linkSourceDisplayNames(const QStringList& allowedLinkSources)
{
    QStringList names;
    for (const QString& source : allowedLinkSources) {
        if (source.compare("youtube", Qt::CaseInsensitive) == 0) {
            names << "Youtube";
        } else if (source.compare("audio_com", Qt::CaseInsensitive) == 0) {
            names << "Audio.com";
        } else {
            names << source;
        }
    }

    return names;
}

QString ConvertFileToScoreModel::linkHintText() const
{
    const cloud::Audio2ScoreConfig& a2s = convertFileToScoreScenario()->convertConfig().audio2score;

    QStringList sources;
    for (const QString& source : a2s.allowedLinkSources) {
        if (source.compare("youtube", Qt::CaseInsensitive) == 0) {
            sources << "<b>Youtube</b>";
        } else if (source.compare("audio_com", Qt::CaseInsensitive) == 0) {
            const QString url = audioComService()->cloudInfo().url.toString();
            sources << "<b><a href=\"" + url + "\">Audio.com</a></b>";
        } else {
            sources << "<b>" + source + "</b>";
        }
    }

    return linkHintTextFor(sources);
}

QString ConvertFileToScoreModel::linkHintPlainText() const
{
    const cloud::Audio2ScoreConfig& a2s = convertFileToScoreScenario()->convertConfig().audio2score;
    return linkHintTextFor(linkSourceDisplayNames(a2s.allowedLinkSources));
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

void ConvertFileToScoreModel::validateFiles(const QStringList& pathsOrUrls)
{
    io::paths_t ioPaths;
    ioPaths.reserve(pathsOrUrls.size());

    QStringList normalizedPaths;
    normalizedPaths.reserve(pathsOrUrls.size());

    for (const QString& pathOrUrl : pathsOrUrls) {
        QString path = localPath(pathOrUrl);
        ioPaths.push_back(io::path_t(path));
        normalizedPaths << path;
    }

    convertFileToScoreScenario()->validate(ioPaths).onResolve(this, [this, normalizedPaths](const RetVal<ConvertType>& result) {
        if (result.ret) {
            setSelectedPaths(normalizedPaths);
            setSelectedLink(QString());
            setConvertType(int(result.val));

            emit validationFinished();
        }
    });
}

void ConvertFileToScoreModel::selectAndValidateFiles(const QStringList& existingPaths)
{
    QStringList files = selectFiles(existingPaths);
    if (!files.isEmpty()) {
        validateFiles(existingPaths + files);
    }
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
