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
#include "infrastructure/smufl.h"
#include "log.h"

#include <QDirIterator>

using namespace mu::notation;

std::string EngravingFontsController::moduleName() const
{
    return "engraving_fonts";
}

void EngravingFontsController::init()
{
    mu::engraving::Smufl::init();

    auto musicFontsPath = configuration()->userMusicFontPathChanged();
    musicFontsPath.onReceive(this, [this](const muse::io::path_t& dir) {
        scanDirectory(dir);
    });

    scanDirectory(configuration()->userMusicFontPath());
}

void EngravingFontsController::scanDirectory(const muse::io::path_t& path) const
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

        if (symbolFontPath.empty() || !QFileInfo::exists(iterator.filePath() + "/metadata.json")) {
            continue;
        }
        if (textFontPath.empty()) {
            textFontPath = symbolFontPath;
        }

        engravingFonts()->addUserFont(fontName.toStdString(), fontName.toStdString(), symbolFontPath);
        fdb->addFont(FontDataKey(fontName), symbolFontPath);
        fdb->addFont(FontDataKey(fontName + u" Text"), textFontPath);
        fdb->insertSubstitution(fontName + u" Text", u"Leland Text");
    }

    engravingFonts()->loadAllFonts();
}
