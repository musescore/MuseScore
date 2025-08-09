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
#include "read302.h"

#include "global/defer.h"

#include "iengravingfont.h"

#include "rw/compat/compatutils.h"
#include "style/style.h"

#include "dom/audio.h"
#include "dom/excerpt.h"
#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/part.h"
#include "dom/score.h"
#include "dom/scoreorder.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/text.h"

#include "../read400/staffrw.h"
#include "../read400/tread.h"
#include "../compat/readstyle.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rw;
using namespace mu::engraving::read400;
using namespace mu::engraving::read302;
using namespace mu::engraving::compat;

bool Read302::readScore302(Score* score, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        ctx.setTrack(muse::nidx);
        const AsciiStringView tag(e.name());
        if (tag == "Staff") {
            read400::StaffRead::readStaff(score, e, ctx);
        } else if (tag == "Omr") {
            e.skipCurrentElement();
        } else if (tag == "Audio") {
            score->m_audio = new Audio;
            read400::TRead::read(score->m_audio, e, ctx);
        } else if (tag == "showOmr") {
            e.skipCurrentElement();
        } else if (tag == "playMode") {
            score->m_playMode = PlayMode(e.readInt());
        } else if (tag == "LayerTag") {
            e.skipCurrentElement();
        } else if (tag == "Layer") {
            e.skipCurrentElement();
        } else if (tag == "currentLayer") {
            e.skipCurrentElement();
        } else if (tag == "Synthesizer") {
            score->m_synthesizerState.read(e);
        } else if (tag == "page-offset") {
            score->m_pageNumberOffset = e.readInt();
        } else if (tag == "Division") {
            score->m_fileDivision = e.readInt();
        } else if (tag == "showInvisible") {
            score->m_showInvisible = e.readInt();
        } else if (tag == "showUnprintable") {
            score->m_showUnprintable = e.readInt();
        } else if (tag == "showFrames") {
            score->m_showFrames = e.readInt();
        } else if (tag == "showMargins") {
            score->m_showPageborders = e.readInt();
        } else if (tag == "markIrregularMeasures") {
            score->m_markIrregularMeasures = e.readInt();
        } else if (tag == "Style") {
            double sp = score->style().value(Sid::spatium).toReal();

            ReadStyleHook::readStyleTag(score, e);

            if (ctx.overrideSpatium()) {
                ctx.setOriginalSpatium(score->style().spatium());
                score->style().set(Sid::spatium, sp);
            }
            score->m_engravingFont = score->engravingFonts()->fontByName(score->style().styleSt(Sid::musicalSymbolFont).toStdString());
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
            read400::TRead::read(part, e, ctx);
            score->appendPart(part);
        } else if ((tag == "HairPin")
                   || (tag == "Ottava")
                   || (tag == "TextLine")
                   || (tag == "Volta")
                   || (tag == "Trill")
                   || (tag == "Slur")
                   || (tag == "Pedal")) {
            Spanner* s = toSpanner(Factory::createItemByName(tag, score->dummy()));
            read400::TRead::readItem(s, e, ctx);
            score->addSpanner(s);
        } else if (tag == "Excerpt") {
            if (MScore::noExcerpts) {
                e.skipCurrentElement();
            } else {
                if (score->isMaster()) {
                    MasterScore* mScore = static_cast<MasterScore*>(score);
                    Excerpt* ex = new Excerpt(mScore);
                    read400::TRead::read(ex, e, ctx);
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

                Score* curScore = ctx.score();
                ctx.setScore(s);

                readScore302(s, e, ctx);

                ctx.setScore(curScore);

                s->linkMeasures(m);
                ex->setTracksMapping(ctx.tracks());
                m->addExcerpt(ex);
            }
        } else if (tag == "name") {
            String n = e.readText();
            if (!score->isMaster()) {     //ignore the name if it's not a child score
                score->excerpt()->setName(n, /*saveAndNotify=*/ false);
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
    if (e.error() != muse::XmlStreamReader::NoError) {
        if (e.error() == muse::XmlStreamReader::CustomError) {
            LOGE() << e.errorString();
        } else {
            LOGE() << String(u"XML read error at line %1, column %2: %3").arg(e.lineNumber(), e.columnNumber())
                .arg(String::fromAscii(e.name().ascii()));
        }
        return false;
    }

    score->connectTies();

    score->m_fileDivision = Constants::DIVISION;

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

    for (Part* p : score->m_parts) {
        p->updateHarmonyChannels(false);
    }

    score->masterScore()->rebuildMidiMapping();
    score->masterScore()->updateChannel();

    for (Staff* staff : score->staves()) {
        staff->updateOttava();
    }

    if (score->isMaster()) {
        CompatUtils::assignInitialPartToExcerpts(score->masterScore()->excerpts());
    }

    return true;
}

muse::Ret Read302::readScore(Score* score, XmlReader& e, ReadInOutData* out)
{
    ReadContext ctx(score);
    if (out) {
        if (out->overriddenSpatium.has_value()) {
            ctx.setSpatium(out->overriddenSpatium.value());
            ctx.setOverrideSpatium(true);
        }

        ctx.setPropertiesToSkip(out->propertiesToSkip);
    }

    DEFER {
        if (out) {
            out->settingsCompat = std::move(ctx.settingCompat());
        }
    };

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "programVersion") {
            score->setMscoreVersion(e.readText());
        } else if (tag == "programRevision") {
            score->setMscoreRevision(e.readInt(nullptr, 16));
        } else if (tag == "Score") {
            if (!readScore302(score, e, ctx)) {
                if (e.error() == muse::XmlStreamReader::CustomError) {
                    return make_ret(Err::FileCriticallyCorrupted, e.errorString());
                }
                return make_ret(Err::FileBadFormat, e.errorString());
            }

            if (ctx.overrideSpatium() && out) {
                out->originalSpatium = ctx.originalSpatium();
            }
        } else if (tag == "Revision") {
            e.skipCurrentElement();
        }
    }

    return muse::make_ok();
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
    } else if (id.startsWith(u"mdl-")) {
        // See https://github.com/musescore/mdl/blob/master/resources/instruments/mdl_1_3_0.xml
        if (id == u"mdl-snareline" || id == u"mdl-snareline-a" || id == u"mdl-snaresolo" || id == u"mdl-snaresolo-a") {
            id = u"marching-snare";
        } else if (id == u"mdl-tenorline" || id == u"mdl-tenorsolo" || id == u"mdl-flubs") {
            id = u"marching-tenor-drums";
        } else if (id == u"mdl-bassline-10" || id == u"mdl-bassline-5") {
            id = u"marching-bass-drums";
        } else if (id == u"mdl-cymballine") {
            id = u"marching-cymbals";
        } else if (id == u"mdl-showtenorline" || id == u"mdl-rail" || id == u"mdl-drumset" || id == u"mdl-sampler") {
            id = u"drumset";
        }
    }

    instrument->setId(id);
}

bool Read302::pasteStaff(XmlReader&, Segment*, staff_idx_t, Fraction)
{
    UNREACHABLE;
    return false;
}

void Read302::pasteSymbols(XmlReader&, ChordRest*)
{
    UNREACHABLE;
}

void Read302::readTremoloCompat(compat::TremoloCompat*, XmlReader&)
{
    UNREACHABLE;
}

void Read302::doReadItem(EngravingItem*, XmlReader&)
{
    UNREACHABLE;
}
