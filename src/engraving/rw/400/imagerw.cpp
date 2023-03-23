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
#include "imagerw.h"

#include "../../libmscore/image.h"
#include "../../libmscore/score.h"

#include "../xmlreader.h"

#include "propertyrw.h"
#include "bsymbolrw.h"

using namespace mu::engraving::rw400;

void ImageRW::read(Image* img, XmlReader& e, ReadContext& ctx)
{
    //! TODO Should be replaced with `ctx.mscVersion()`
    //! But at the moment, `ctx` is not set everywhere
    int mscVersion = img->score()->mscVersion();
    if (mscVersion <= 114) {
        img->setSizeIsSpatium(false);
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "autoScale") {
            PropertyRW::readProperty(img, e, ctx, Pid::AUTOSCALE);
        } else if (tag == "size") {
            PropertyRW::readProperty(img, e, ctx, Pid::SIZE);
        } else if (tag == "lockAspectRatio") {
            PropertyRW::readProperty(img, e, ctx, Pid::LOCK_ASPECT_RATIO);
        } else if (tag == "sizeIsSpatium") {
            // setting this using the property Pid::SIZE_IS_SPATIUM breaks, because the
            // property setter attempts to maintain a constant size. If we're reading, we
            // don't want to do that, because the stored size will be in:
            //    mm if size isn't spatium
            //    sp if size is spatium
            img->setSizeIsSpatium(e.readBool());
        } else if (tag == "path") {
            img->setStorePath(e.readText());
        } else if (tag == "linkPath") {
            img->setLinkPath(e.readText());
        } else if (tag == "subtype") {    // obsolete
            e.skipCurrentElement();
        } else if (!BSymbolRW::readProperties(img, e, ctx)) {
            e.unknown();
        }
    }

    img->load();
}
