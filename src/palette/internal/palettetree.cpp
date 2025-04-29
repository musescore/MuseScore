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

#include "palettetree.h"

#include <QBuffer>

#include "engraving/rw/xmlreader.h"
#include "engraving/rw/xmlwriter.h"

using namespace mu::palette;

void PaletteTree::insert(size_t idx, PalettePtr palette)
{
    palettes.emplace(palettes.begin() + idx, palette);
}

void PaletteTree::append(PalettePtr palette)
{
    palettes.emplace_back(palette);
}

bool PaletteTree::read(mu::engraving::XmlReader& e, bool pasteMode)
{
    while (e.readNextStartElement()) {
        const muse::AsciiStringView tag(e.name());
        if (tag == "Palette") {
            PalettePtr p = std::make_shared<Palette>();
            p->read(e, pasteMode);
            palettes.push_back(p);
        } else {
            e.unknown();
        }
    }

    return true;
}

void PaletteTree::write(mu::engraving::XmlWriter& xml, bool pasteMode) const
{
    xml.startElement("PaletteBox"); // for compatibility with old palettes file format

    for (const PalettePtr& palette : palettes) {
        palette->write(xml, pasteMode);
    }

    xml.endElement();
}

void PaletteTree::retranslate()
{
    for (const PalettePtr& palette : palettes) {
        palette->retranslate();
    }
}
