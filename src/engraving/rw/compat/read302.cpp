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
#include "read302.h"

#include "infrastructure/symbolfonts.h"
#include "rw/xml.h"
#include "style/style.h"

#include "libmscore/audio.h"
#include "libmscore/excerpt.h"
#include "libmscore/factory.h"
#include "libmscore/masterscore.h"
#include "libmscore/measurebase.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/scoreorder.h"
#include "libmscore/spanner.h"
#include "libmscore/staff.h"
#include "libmscore/text.h"

#include "../staffrw.h"
#include "readstyle.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rw;
using namespace mu::engraving::compat;

bool Read302::readScore302(Score* score, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        ctx.setTrack(mu::nidx);
        const AsciiStringView tag(e.name());
        if (tag == "Staff") {
            StaffRW::readStaff(score, e, ctx);
        } else if (tag == "Omr") {
            e.skipCurrentElement();
        } else if (tag == "Audio") {
            score->_audio = new Audio;
            score->_audio->read(e);
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
            double sp = score->style().value(Sid::spatium).toReal();

            ReadStyleHook::readStyleTag(score, e);

            // if (_layoutMode == LayoutMode::FLOAT || _layoutMode == LayoutMode::SYSTEM) {
            if (score->layoutOptions().isMode(LayoutMode::FLOAT)) {
                // style should not change spatium in
                // float mode
                score->style().set(Sid::spatium, sp);
            }
            score->m_symbolFont = SymbolFonts::fontByName(score->style().styleSt(Sid::MusicalSymbolFont));
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
        } else if (tag == "Part") {
            Part* part = new Part(score);
            part->read(e);
            score->appendPart(part);
        } else if ((tag == "HairPin")
                   || (tag == "Ottava")
                   || (tag == "TextLine")
                   || (tag == "Volta")
                   || (tag == "Trill")
                   || (tag == "Slur")
                   || (tag == "Pedal")) {
            Spanner* s = toSpanner(Factory::createItemByName(tag, score->dummy()));
            s->read(e);
            score->addSpanner(s);
        } else if (tag == "Excerpt") {
            if (MScore::noExcerpts) {
                e.skipCurrentElement();
            } else {
                if (score->isMaster()) {
                    MasterScore* mScore = static_cast<MasterScore*>(score);
                    Excerpt* ex = new Excerpt(mScore);
                    ex->read(e);
                    mScore->excerpts().push_back(ex);
                } else {
                    LOGD("Score::read(): part cannot have parts");
                    e.skipCurrentElement();
                }
            }
        } else if (e.name() == "Tracklist") {
            int strack = e.intAttribute("sTrack",   -1);
            int dtrack = e.intAttribute("dstTrack", -1);
            if (strack != -1 && dtrack != -1) {
                ctx.tracks().insert({ strack, dtrack });
            }
            e.skipCurrentElement();
        } else if (tag == "Score") {            // recursion
            if (MScore::noExcerpts) {
                e.skipCurrentElement();
            } else {
                ctx.tracks().clear();             // ???
                MasterScore* m = score->masterScore();
                Score* s = m->createScore();

                ReadStyleHook::setupDefaultStyle(s);

                Excerpt* ex = new Excerpt(m);
                ex->setExcerptScore(s);
                ctx.setLastMeasure(nullptr);

                Score* curScore = e.context()->score();
                e.context()->setScore(s);

                readScore302(s, e, *e.context());

                e.context()->setScore(curScore);

                s->linkMeasures(m);
                ex->setTracksMapping(ctx.tracks());
                m->addExcerpt(ex);
            }
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
    e.context()->reconnectBrokenConnectors();
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
    score->relayoutForStyles(); // force relayout if certain style settings are enabled

    score->_fileDivision = Constants::division;

    if (score->mscVersion() == 302) {
        // MuseScore 3.6.x scores had some wrong instrument IDs
        for (Part* part : score->parts()) {
            for (const auto& pair : part->instruments()) {
                fixInstrumentId(pair.second);
            }
        }
    } else {
        // Older scores had no IDs at all
        for (Part* part : score->parts()) {
            for (const auto& pair : part->instruments()) {
                pair.second->updateInstrumentId();
            }
        }
    }

    score->setUpTempoMap();

    for (Part* p : score->_parts) {
        p->updateHarmonyChannels(false);
        if (!p->show()) {
            // convert hidden instruments into hidden staves, to preserve playback
            p->setShow(true);
            for (Staff* s : p->staves()) {
                s->setVisible(false);
            }
        }
    }

    score->masterScore()->rebuildMidiMapping();
    score->masterScore()->updateChannel();

    for (Staff* staff : score->staves()) {
        staff->updateOttava();
    }

//      createPlayEvents();
    return true;
}

Err Read302::read302(MasterScore* masterScore, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "programVersion") {
            masterScore->setMscoreVersion(e.readText());
        } else if (tag == "programRevision") {
            masterScore->setMscoreRevision(e.readInt(nullptr, 16));
        } else if (tag == "Score") {
            if (!readScore302(masterScore, e, ctx)) {
                if (e.error() == XmlStreamReader::CustomError) {
                    return Err::FileCriticallyCorrupted;
                }
                return Err::FileBadFormat;
            }
        } else if (tag == "Revision") {
            e.skipCurrentElement();
        }
    }

    return Err::NoError;
}

void Read302::fixInstrumentId(Instrument* instrument)
{
    String id = instrument->id();
    String trackName = instrument->trackName().toLower();

    // incorrect instrument IDs in 3.6.x
    if (id == u"Winds") {
        id = u"winds";
    } else if (id == u"harmonica-d12high-g") {
        id = u"harmonica-d10high-g";
    } else if (id == u"harmonica-d12f") {
        id = u"harmonica-d10f";
    } else if (id == u"harmonica-d12d") {
        id = u"harmonica-d10d";
    } else if (id == u"harmonica-d12c") {
        id = u"harmonica-d10c";
    } else if (id == u"harmonica-d12a") {
        id = u"harmonica-d10a";
    } else if (id == u"harmonica-d12-g") {
        id = u"harmonica-d10g";
    } else if (id == u"drumset" && trackName == u"percussion") {
        id = u"percussion";
    } else if (id == u"cymbal" && trackName == u"cymbals") {
        id = u"marching-cymbals";
    } else if (id == u"bass-drum" && trackName == u"bass drums") {
        id = u"marching-bass-drums";
    }

    instrument->setId(id);
}
