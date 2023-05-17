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
#include "read400.h"

#include "libmscore/audio.h"
#include "libmscore/excerpt.h"
#include "libmscore/factory.h"
#include "libmscore/masterscore.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/spanner.h"
#include "libmscore/staff.h"
#include "libmscore/text.h"

#include "staffrw.h"
#include "tread.h"

#include "log.h"
#include "rw/compat/compatutils.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

Err Read400::read(Score* score, XmlReader& e, ReadInOutData* data)
{
    ReadContext ctx(score);
    e.setContext(&ctx);

    if (!score->isMaster() && data) {
        ctx.initLinks(data->links);
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "programVersion") {
            String ver = e.readText();
            if (score->isMaster()) {
                score->setMscoreVersion(ver);
            }
        } else if (tag == "programRevision") {
            int rev = e.readInt(nullptr, 16);
            if (score->isMaster()) {
                score->setMscoreRevision(rev);
            }
        } else if (tag == "Revision") {
            e.skipCurrentElement();
        } else if (tag == "Score") {
            if (!readScore400(score, e, ctx)) {
                if (e.error() == XmlStreamReader::CustomError) {
                    return Err::FileCriticallyCorrupted;
                }
                return Err::FileBadFormat;
            }
        } else if (tag == "museScore") {
            // pass
        } else {
            e.skipCurrentElement();
        }
    }

    if (!score->isMaster()) {
        Excerpt* ex = score->excerpt();
        ex->setTracksMapping(ctx.tracks());
    }

    if (data) {
        data->links = ctx.readLinks();
        data->settingsCompat = ctx.settingCompat();
    }

    return Err::NoError;
}

bool Read400::readScore400(Score* score, XmlReader& e, ReadContext& ctx)
{
    std::vector<int> sysStaves;
    while (e.readNextStartElement()) {
        e.context()->setTrack(mu::nidx);
        const AsciiStringView tag(e.name());
        if (tag == "Staff") {
            StaffRW::readStaff(score, e, ctx);
        } else if (tag == "Omr") {
            e.skipCurrentElement();
        } else if (tag == "Audio") {
            score->_audio = new Audio;
            rw400::TRead::read(score->_audio, e, ctx);
        } else if (tag == "showOmr") {
            e.skipCurrentElement();
        } else if (tag == "playMode") {
            score->_playMode = PlayMode(e.readInt());
        } else if (tag == "LayerTag") {
            int id = e.intAttribute("id");
            const String& t = e.attribute("tag");
            String val(e.readText());
            if (id >= 0 && id < 32) {
                score->_layerTags[id] = t;
                score->_layerTagComments[id] = val;
            }
        } else if (tag == "Layer") {
            Layer layer;
            layer.name = e.attribute("name");
            layer.tags = static_cast<unsigned int>(e.intAttribute("mask"));
            score->_layer.push_back(layer);
            e.readNext();
        } else if (tag == "currentLayer") {
            score->_currentLayer = e.readInt();
        } else if (tag == "Synthesizer") {
            score->_synthesizerState.read(e);
        } else if (tag == "page-offset") {
            score->_pageNumberOffset = e.readInt();
        } else if (tag == "Division") {
            score->_fileDivision = e.readInt();
        } else if (tag == "open") {
            score->_isOpen = e.readBool();
        } else if (tag == "showInvisible") {
            score->_showInvisible = e.readInt();
        } else if (tag == "showUnprintable") {
            score->_showUnprintable = e.readInt();
        } else if (tag == "showFrames") {
            score->_showFrames = e.readInt();
        } else if (tag == "showMargins") {
            score->_showPageborders = e.readInt();
        } else if (tag == "markIrregularMeasures") {
            score->_markIrregularMeasures = e.readInt();
        } else if (tag == "Style") {
            // Since version 400, the style is stored in a separate file
            e.skipCurrentElement();
        } else if (tag == "copyright" || tag == "rights") {
            score->setMetaTag(u"copyright", Text::readXmlText(e, score));
        } else if (tag == "movement-number") {
            score->setMetaTag(u"movementNumber", e.readText());
        } else if (tag == "movement-title") {
            score->setMetaTag(u"movementTitle", e.readText());
        } else if (tag == "work-number") {
            score->setMetaTag(u"workNumber", e.readText());
        } else if (tag == "work-title") {
            score->setMetaTag(u"workTitle", e.readText());
        } else if (tag == "source") {
            score->setMetaTag(u"source", e.readText());
        } else if (tag == "metaTag") {
            String name = e.attribute("name");
            score->setMetaTag(name, e.readText());
        } else if (tag == "Order") {
            ScoreOrder order;
            order.read(e);
            if (order.isValid()) {
                score->setScoreOrder(order);
            }
        } else if (tag == "SystemObjects") {
            // the staves to show system objects
            score->clearSystemObjectStaves();
            while (e.readNextStartElement()) {
                if (e.name() == "Instance") {
                    int staffIdx = e.attribute("staffId").toInt() - 1;
                    // TODO: read the other attributes from this element when we begin treating different classes
                    // of system objects differently. ex:
                    // bool showBarNumbers = !(e.hasAttribute("barNumbers") && e.attribute("barNumbers") == "false");
                    if (staffIdx > 0) {
                        sysStaves.push_back(staffIdx);
                    }
                    e.skipCurrentElement();
                } else {
                    e.skipCurrentElement();
                }
            }
        } else if (tag == "Part") {
            Part* part = new Part(score);
            TRead::read(part, e, ctx);
            score->appendPart(part);
        } else if ((tag == "HairPin")
                   || (tag == "Ottava")
                   || (tag == "TextLine")
                   || (tag == "Volta")
                   || (tag == "Trill")
                   || (tag == "Slur")
                   || (tag == "Pedal")) {
            Spanner* s = toSpanner(Factory::createItemByName(tag, score->dummy()));
            rw400::TRead::readItem(s, e, ctx);
            score->addSpanner(s);
        } else if (tag == "Excerpt") {
            // Since version 400, the Excerpts are stored in a separate file
            e.skipCurrentElement();
        } else if (e.name() == "initialPartId") {
            if (score->excerpt()) {
                score->excerpt()->setInitialPartId(ID(e.readInt()));
            }
        } else if (e.name() == "Tracklist") {
            int strack = e.intAttribute("sTrack",   -1);
            int dtrack = e.intAttribute("dstTrack", -1);
            if (strack != -1 && dtrack != -1) {
                ctx.tracks().insert({ strack, dtrack });
            }
            e.skipCurrentElement();
        } else if (tag == "Score") {
            // Since version 400, the Excerpts is stored in a separate file
            e.skipCurrentElement();
        } else if (tag == "name") {
            String n = e.readText();
            if (!score->isMaster()) {     //ignore the name if it's not a child score
                score->excerpt()->setName(n);
            }
        } else if (tag == "layoutMode") {
            String s = e.readText();
            if (s == "line") {
                score->setLayoutMode(LayoutMode::LINE);
            } else if (s == "system") {
                score->setLayoutMode(LayoutMode::SYSTEM);
            } else {
                LOGD("layoutMode: %s", muPrintable(s));
            }
        } else {
            e.unknown();
        }
    }
    ctx.reconnectBrokenConnectors();
    if (e.error() != XmlStreamReader::NoError) {
        if (e.error() == XmlStreamReader::CustomError) {
            LOGE() << e.errorString();
        } else {
            LOGE() << String(u"XML read error at line %1, column %2: %3").arg(e.lineNumber(), e.columnNumber())
                .arg(String::fromAscii(e.name().ascii()));
        }
        return false;
    }

    score->connectTies();

    score->_fileDivision = Constants::division;

    // Make sure every instrument has an instrumentId set.
    for (Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            pair.second->updateInstrumentId();
        }
    }

    score->setUpTempoMap();

    for (Part* p : score->_parts) {
        p->updateHarmonyChannels(false);
    }

    score->masterScore()->rebuildMidiMapping();
    score->masterScore()->updateChannel();

    for (Staff* staff : score->staves()) {
        staff->updateOttava();
    }
    for (int idx : sysStaves) {
        score->addSystemObjectStaff(score->staff(idx));
    }

//      createPlayEvents();

    return true;
}
