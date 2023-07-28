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

#include <cmath>

#include "io/file.h"
#include "io/fileinfo.h"

#include "style/style.h"

#include "engravingitem.h"
#include "mscore.h"
#include "page.h"
#include "score.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;
using namespace mu::engraving::read400;

namespace mu::engraving {
//---------------------------------------------------------
// linkMeasures
//---------------------------------------------------------

void Score::linkMeasures(Score* score)
{
    MeasureBase* mbMaster = score->first();
    for (MeasureBase* mb = first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        while (mbMaster && !mbMaster->isMeasure()) {
            mbMaster = mbMaster->next();
        }
        if (!mbMaster) {
            LOGD("Measures in MasterScore and Score are not in sync.");
            break;
        }
        mb->linkTo(mbMaster);
        mbMaster = mbMaster->next();
    }
}

//---------------------------------------------------------
//   createThumbnail
//---------------------------------------------------------

std::shared_ptr<mu::draw::Pixmap> Score::createThumbnail()
{
    TRACEFUNC;

    LayoutMode mode = layoutMode();
    switchToPageMode();

    Page* page = pages().at(0);
    RectF fr = page->abbox();
    double mag = 256.0 / std::max(fr.width(), fr.height());
    int w = int(fr.width() * mag);
    int h = int(fr.height() * mag);

    int dpm = lrint(DPMM * 1000.0);

    auto pixmap = imageProvider()->createPixmap(w, h, dpm, configuration()->thumbnailBackgroundColor());

    double pr = MScore::pixelRatio;
    MScore::pixelRatio = 1.0;

    auto painterProvider = imageProvider()->painterForImage(pixmap);
    mu::draw::Painter p(painterProvider, "thumbnail");

    p.setAntialiasing(true);
    p.scale(mag, mag);
    print(&p, 0);
    p.endDraw();

    MScore::pixelRatio = pr;

    if (layoutMode() != mode) {
        setLayoutMode(mode);
        doLayout();
    }
    return pixmap;
}

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

bool Score::loadStyle(const String& fn, bool ign, const bool overlap)
{
    TRACEFUNC;

    File f(fn);
    if (f.open(IODevice::ReadOnly)) {
        MStyle st = style();
        if (st.read(&f, ign)) {
            undo(new ChangeStyle(this, st, overlap));
            return true;
        } else {
            LOGE() << "The style file is not compatible with this version of MuseScore.";
            return false;
        }
    }

    return false;
}

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

bool Score::saveStyle(const String& name)
{
    String ext(u".mss");
    FileInfo info(name);

    if (info.suffix().isEmpty()) {
        info = FileInfo(info.filePath() + ext);
    }
    File f(info.filePath());
    if (!f.open(IODevice::WriteOnly)) {
        LOGE() << "Failed open style file: " << info.filePath();
        return false;
    }

    bool ok = style().write(&f);
    if (!ok) {
        LOGE() << "Failed write style file: " << info.filePath();
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void Score::print(mu::draw::Painter* painter, int pageNo)
{
    m_printing  = true;
    MScore::pdfPrinting = true;
    Page* page = pages().at(pageNo);
    RectF fr  = page->abbox();

    std::vector<EngravingItem*> ell = page->items(fr);
    std::sort(ell.begin(), ell.end(), elementLessThan);
    for (const EngravingItem* e : ell) {
        if (!e->visible()) {
            continue;
        }
        painter->save();
        painter->translate(e->pagePos());
        EngravingItem::rendering()->drawItem(e, painter);
        painter->restore();
    }
    MScore::pdfPrinting = false;
    m_printing = false;
}
}
