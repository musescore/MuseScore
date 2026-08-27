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

QString ConvertFileToScoreModel::guidelinesLinkText() const
{
    return "<a href=\"" + configuration()->scoreUploadingGuidelinesUrl().toString() + "\">"
           + muse::qtrc("project/convert", "Uploading guidelines") + "</a>";
}

QString ConvertFileToScoreModel::accountAvatarUrl() const
{
    return museScoreComService()->authorization()->accountInfo().avatarUrl.toString();
}

QString ConvertFileToScoreModel::audioComUrl() const
{
    return audioComService()->cloudInfo().url.toString();
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
        imageItems << muse::qtrc("project/convert", "Images will be combined into one score in the order you upload them");
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

QString ConvertFileToScoreModel::linkPasteText() const
{
    const cloud::Audio2ScoreConfig& a2s = convertFileToScoreScenario()->convertConfig().audio2score;

    if (a2s.allowedLinkSources.isEmpty()) {
        return QString();
    }

    QStringList sources;
    for (const QString& source : a2s.allowedLinkSources) {
        if (source.compare("youtube", Qt::CaseInsensitive) == 0) {
            sources << "<b>Youtube</b>";
        } else if (source.compare("audio_com", Qt::CaseInsensitive) == 0) {
            sources << "<b><a href=\"" + audioComUrl() + "\">Audio.com</a></b>";
        } else {
            sources << "<b>" + source + "</b>";
        }
    }

    return muse::qtrc("project/convert", "Or paste a link from %1 (beta)").arg(joinWithOr(sources));
}

int ConvertFileToScoreModel::maxLinkLength() const
{
    return convertFileToScoreScenario()->convertConfig().audio2score.maxLinkLength;
}

bool ConvertFileToScoreModel::canSelectMultipleFiles(int type, const QStringList& paths) const
{
    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();

    if (ConvertType(type) == ConvertType::Audio2Score) {
        return allowsMultipleFiles(config.audio2score.maxFiles);
    }

    if (containsPdf(paths)) {
        return false;
    }

    return allowsMultipleFiles(config.omr.maxImages);
}

QStringList ConvertFileToScoreModel::selectFiles(const QStringList& existingPaths)
{
    const ConvertConfig& config = convertFileToScoreScenario()->convertConfig();

    QStringList extensions = existingPaths.isEmpty()
                             ? config.omr.allowedExtensions + config.audio2score.allowedExtensions
                             : resolveExtensions(existingPaths);

    if (extensions.isEmpty()) {
        extensions = { "pdf", "jpg", "jpeg", "png", "mp3" };
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

    QVariantList normalizedPaths;
    normalizedPaths.reserve(pathsOrUrls.size());

    for (const QString& pathOrUrl : pathsOrUrls) {
        QString path = localPath(pathOrUrl);
        ioPaths.push_back(io::path_t(path));
        normalizedPaths << path;
    }

    convertFileToScoreScenario()->validate(ioPaths).onResolve(this, [this, normalizedPaths](const RetVal<ConvertType>& result) {
        if (result.ret) {
            emit validationFinished(int(result.val), normalizedPaths);
        }
    });
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
