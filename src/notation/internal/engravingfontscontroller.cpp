/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "engravingfontscontroller.h"

#include <QDirIterator>
#include <QFontDatabase>
#include <QStandardPaths>

#include "log.h"

using namespace mu::notation;

void EngravingFontsController::init()
{
    configuration()->userMusicFontsPathChanged().onReceive(this, [this](const muse::io::path_t&) {
        scanAllDirectories();
    });

    scanAllDirectories();
}

void EngravingFontsController::scanAllDirectories() const
{
    engravingFonts()->clearExternalFonts();

    // Standard locations as described in https://w3c.github.io/smufl/latest/specification/font-metadata-locations.html

    // These standard location roughly match up with what the following returns, but some adjustments are needed.
    // QStringList globalFontsPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first(2);

#if defined(Q_OS_WIN)
    // On Windows, the second standard location returned by Qt is %ProgramData%, but we want %CommonProgramFiles%
    QStringList globalFontsPaths {
        QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first(),
        qgetenv("CommonProgramFiles").replace("\\", "/")
    };
#elif defined(Q_OS_MACOS)
    // MacOS is correctly handled by Qt
    QStringList globalFontsPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first(2);
#elif defined(Q_OS_LINUX)
    // On Unix systems, we want $XDG_DATA_HOME (user-specific) and $XDG_DATA_DIRS (system-wide)
    QStringList globalFontsPaths { qgetenv("XDG_DATA_HOME") };
    globalFontsPaths.append(QString::fromLocal8Bit(qgetenv("XDG_DATA_DIRS")).split(':'));
#endif

    // The first location is the user-wide location, so we should iterate in reverse order so that
    // user-specific fonts can override system-wide fonts
    for (auto it = globalFontsPaths.crbegin(); it != globalFontsPaths.crend(); ++it) {
        scanDirectory(*it + "/SMuFL/Fonts", false);
    }

    // Scan MuseScore-specific "private" location last so that it can override global fonts
    muse::io::path_t userFontsPath = configuration()->userMusicFontsPath();
    if (!userFontsPath.empty()) {
        scanDirectory(userFontsPath, true);
    }

    engravingFonts()->loadAllFonts();

    QStringList musicFonts;
    for (const engraving::IEngravingFontPtr& font : engravingFonts()->fonts()) {
        musicFonts << QString::fromStdString(font->name());
        musicFonts << QString::fromStdString(font->name() + " Text");
    }
    uiConfiguration()->setNonTextFonts(musicFonts);
}

void EngravingFontsController::scanDirectory(const muse::io::path_t& path, bool isPrivate) const
{
    using namespace muse::draw;

    std::shared_ptr<IFontsDatabase> fdb = fontsDatabase();

    QDirIterator iterator(path.toQString(), QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

    while (iterator.hasNext()) {
        iterator.next();
        QString fontDir = iterator.filePath();
        muse::io::path_t metadataPath;

        {
            QDirIterator jsonFilesIterator(fontDir, { "*.json" }, QDir::Files);
            while (jsonFilesIterator.hasNext()) {
                jsonFilesIterator.next();
                metadataPath = jsonFilesIterator.filePath();
            }
        }

        if (metadataPath.empty()) {
            continue;
        }

        // We assume the font name is the same as the directory name,
        // but maybe we should instead read the metadata.json file to get the font name

        QString fontName = iterator.fileName();
        QString fontFamily = fontName;

        if (fontName.contains(u"Text")) {
            LOGI() << "Skipping text music font: " << fontName;
            continue;
        }

        auto findFontPath = [this, isPrivate, fontDir](QString fontName) {
            if (isPrivate) {
                return findFontPathPrivate(fontDir, fontName);
            } else {
                return findFontPathGlobal(fontName);
            }
        };

        muse::io::path_t symbolFontPath = findFontPath(fontName);
        if (symbolFontPath.empty()) {
            QString tmp = fontName;
            symbolFontPath = findFontPath(tmp.replace(" ", ""));
        }

        muse::io::path_t textFontPath = findFontPath(fontName + " Text");
        if (symbolFontPath.empty()) {
            QString tmp = fontName;
            symbolFontPath = findFontPath(tmp.replace(" ", "") + "Text");
        }

        if (textFontPath.empty()) {
            textFontPath = symbolFontPath;
        }

        if (symbolFontPath.empty()) {
            LOGE() << "Music font \"" << fontName << "\" for " << metadataPath << " not found";
            continue;
        }

        muse::String fontNameStr = muse::String::fromQString(fontName);
        engravingFonts()->addExternalFont(fontName.toStdString(), fontFamily.toStdString(), symbolFontPath, metadataPath);
        fdb->addFont(FontDataKey(fontNameStr), symbolFontPath);
        fdb->addFont(FontDataKey(fontNameStr + u" Text"), textFontPath);
        fdb->insertSubstitution(fontNameStr + u" Text", u"Leland Text");
    }
}

muse::io::path_t EngravingFontsController::findFontPathPrivate(const QString& metadataDir, const QString& fontName) const
{
    // Search in the folder where the metadata lives

    QStringList testPaths {
        metadataDir + "/" + fontName + ".ttf",
        metadataDir + "/" + fontName + ".otf"
    };
    for (const QString& testPath : testPaths) {
        if (QFile::exists(testPath)) {
            return testPath;
        }
    }
    return muse::io::path_t();
}

muse::io::path_t EngravingFontsController::findFontPathGlobal(const QString& fontName) const
{
    QStringList fontLocations = QStandardPaths::standardLocations(QStandardPaths::FontsLocation);
#if defined(Q_OS_WIN)
    // Qt does not include the local fonts folder in the standard locations
    fontLocations << qgetenv("LocalAppData").replace("\\", "/") + "/Microsoft/Windows/Fonts";
#endif
    for (const QString& dir : fontLocations) {
        QDirIterator it(dir, { "*.ttf", "*.otf" }, QDir::Files);
        while (it.hasNext()) {
            it.next();
            if (it.fileName() == fontName + ".ttf" || it.fileName() == fontName + ".otf") {
                return it.filePath();
            }
        }
    }
    return muse::io::path_t();
}
