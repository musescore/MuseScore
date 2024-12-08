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

#include "engraving/infrastructure/smufl.h"
#include "engraving/internal/engravingfont.h"
#include "draw/internal/ifontsdatabase.h"
#include "log.h"

#include <QDirIterator>
#include <QStandardPaths>

using namespace mu::notation;

std::string EngravingFontsController::moduleName() const
{
    return "engraving_fonts";
}

void EngravingFontsController::init()
{
    // Standard locations as described in https://w3c.github.io/smufl/latest/specification/font-metadata-locations.html

    // These standard location roughly match up with what the following returns, but some adjustments are needed.
    QStringList systemFontsPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first(2);

#if defined(Q_OS_WIN)
    // On Windows, the second standard location returned by Qt is %ProgramData%, but we want %CommonProgramFiles%
    systemFontsPaths[1] = qgetenv("CommonProgramFiles").replace("\\", "/");
#elif defined(Q_OS_LINUX)
    // On Unix systems, we want $XDG_DATA_DIRS and $XDG_DATA_HOME
    QStringList xdgDataDirs = QString::fromLocal8Bit(qgetenv("XDG_DATA_DIRS")).split(':');
    systemFontsPaths = xdgDataDirs << qgetenv("XDG_DATA_HOME");
#endif

    // The first location is the system-wide location, so we should iterate in reverse order so that
    // user fonts take precedence over system fonts
    for (QString path : systemFontsPaths) {
        scanDirectory(path + "/SMuFL/Fonts", false);
    }

    // Additionally, also support a MuseScore-specific location
    auto musicFontsPath = configuration()->userMusicFontPathChanged();
    musicFontsPath.onReceive(this, [this](const muse::io::path_t& dir) {
        scanDirectory(dir, true);
    });

    scanDirectory(configuration()->userMusicFontPath(), true);
}

void EngravingFontsController::scanDirectory(const muse::io::path_t& path, bool isPrivate) const
{
    using namespace muse::draw;

    std::shared_ptr<IFontsDatabase> fdb = ioc()->resolve<IFontsDatabase>(moduleName());
    engravingFonts()->clearUserFonts();

    QDirIterator iterator(path.toQString(), QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

    while (iterator.hasNext()) {
        QString fontDir = iterator.next();
        muse::String fontName;

        muse::io::path_t symbolFontPath;
        muse::io::path_t textFontPath;
        QDirIterator fontFiles(iterator.filePath(), { "*.otf", "*.ttf" }, QDir::Files);

        while (fontFiles.hasNext()) {
            fontFiles.next();
            QString name = fontFiles.fileInfo().baseName();
            if (name.endsWith("Text")) {
                textFontPath = fontFiles.filePath();
            } else {
                symbolFontPath = fontFiles.filePath();
                fontName = name;
            }
        }

        if (symbolFontPath.empty()) {
            continue;
        }
        if (textFontPath.empty()) {
            textFontPath = symbolFontPath;
        }

        muse::io::path_t metadataPath;
        QStringList metadataFilenameOptions = {
            "/metadata.json",
            QString("/%1.json").arg(fontName),
            QString("/%1_metadata.json").arg(fontName)
        };
        for (const auto& option : metadataFilenameOptions) {
            if (QFile::exists(iterator.filePath() + option)) {
                metadataPath = iterator.filePath() + option;
                break;
            }
        }
        if (metadataPath.empty()) {
            LOGE() << "No metadata file found for font " << fontName;
            continue;
        }

        engravingFonts()->addExternalFont(fontName.toStdString(), fontName.toStdString(), symbolFontPath, metadataPath, isPrivate);
        fdb->addFont(FontDataKey(fontName), symbolFontPath);
        fdb->addFont(FontDataKey(fontName + u" Text"), textFontPath);
        fdb->insertSubstitution(fontName + u" Text", u"Leland Text");
    }

    engravingFonts()->loadAllFonts();
}
