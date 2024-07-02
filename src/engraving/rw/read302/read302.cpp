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
#include "dom/chord.h"
#include "dom/drumset.h"
#include "dom/excerpt.h"
#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/note.h"
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
            score->m_engravingFont = score->engravingFonts()->fontByName(score->style().styleSt(Sid::MusicalSymbolFont).toStdString());
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
                ctx.setMasterCtx(&ctx);

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

    for (Part* part : score->parts()) {
        const track_idx_t startTrack = part->startTrack();
        const track_idx_t endTrack = part->endTrack();
        const InstrumentList& instruments = part->instruments();

        // First instrument in list is the "default instrument".
        // It's at tick -1 but we must start at tick 0.
        auto it = instruments.cbegin();
        for (Fraction startTick(0, 1), endTick; it != instruments.cend(); startTick = endTick) {
            Instrument* instr = it->second;
            ++it; // careful, iterator now points to next instrument in the list
            endTick = (it == instruments.cend())
                      ? score->endTick()
                      : Fraction::fromTicks(it->first);

            if (!instr->useDrumset() || !instr->musicXmlId().startsWith(u"mdl.")) {
                continue;
            }

            const Drumset* oldDrumset = instr->drumset();
            const InstrChannel* channel = instr->channel(0);

            if (!oldDrumset || !channel || channel->synti() != u"Zerberus") {
                continue;
            }

            const int program = channel->program();
            std::function<int(int)> repitchFunc;

            if (instr->musicXmlId() == u"mdl.drum.snare-drum" && (program == 5 || program == 6)) {
                // MDL Snare Line     (id="mdl-snareline")
                // MDL Snare Line A   (id="mdl-snareline-a")
                // MDL Snare          (id="mdl-snaresolo")
                // MDL Snare A        (id="mdl-snaresolo-a")
                repitchFunc = [](int pitch) {
                    switch (pitch) {
                    case 23: return 55;
                    case 27: return 25;
                    case 29: return 92;
                    case 30: return 91;
                    case 31: return 89;
                    case 32: return 58;
                    case 33: return 71;
                    case 36: return 76;
                    case 38: return 40;
                    case 39: return 38;
                    case 40: return 36;
                    case 60: return 50;
                    case 61: return 51;
                    case 62: return 56;
                    case 63: return 53;
                    case 64: return 49;
                    case 65: return 57;
                    case 67: return 60;
                    case 68: return 60;
                    case 72: return 65;
                    case 73: return 67;
                    case 74: return 59;
                    case 77: return 24;
                    default: return pitch;
                    }
                };
            } else if (instr->musicXmlId() == u"mdl.drum.tenor-drum" && (program == 7 || program == 8)) {
                // MDL Tenor Line   (id="mdl-tenorline")
                // MDL Tenors       (id="mdl-tenorsolo")
                repitchFunc = [](int pitch) {
                    switch (pitch) {
                    case 47: return 40;
                    case 60: return 36;
                    case 61: return 48;
                    case 62: return 60;
                    case 63: return 72;
                    case 64: return 84;
                    case 65: return 96;
                    case 72: return 41;
                    case 73: return 53;
                    case 74: return 65;
                    case 75: return 77;
                    case 76: return 89;
                    case 77: return 101;
                    case 78: return 37;
                    case 79: return 49;
                    case 80: return 61;
                    case 81: return 73;
                    case 82: return 85;
                    case 83: return 97;
                    case 96: return 37;
                    case 97: return 49;
                    case 98: return 61;
                    case 107: return 36;
                    default: return pitch;
                    }
                };
            } else if (instr->musicXmlId() == u"mdl.drum.bass-drum" && !oldDrumset->isValid(61)) {
                // MDL Bass Line (5)    (id="mdl-bassline-5")
                repitchFunc = [](int pitch) {
                    switch (pitch) {
                    case 60: return 90;
                    case 62: return 37;
                    case 64: return 49;
                    case 67: return 61;
                    case 68: return 73;
                    case 70: return 85;
                    case 72: return 92;
                    case 74: return 39;
                    case 76: return 51;
                    case 79: return 63;
                    case 80: return 75;
                    case 82: return 87;
                    default: return pitch;
                    }
                };
            } else {
                continue;
            }

            Drumset newDrumset;
            newDrumset.clear();
            for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
                if (oldDrumset->isValid(i)) {
                    newDrumset.setDrum(repitchFunc(i), oldDrumset->drum(i));
                }
            }
            // Note: newDrumset may have fewer drums than oldDrumset.
            // This happens if there were two pitches X and Y for which
            // repitchFunc(X) == repitchFunc(Y), and oldDrumset had drums
            // on both X and Y. In this situation, drum Y wins (if Y > X).
            instr->setDrumset(&newDrumset);

            auto repitchChord = [repitchFunc](Chord* ch) {
                for (Note* n : ch->notes()) {
                    n->setPitch(repitchFunc(n->pitch()));
                }
            };

            for (Segment* s = score->tick2segment(startTick, true, SegmentType::ChordRest);
                 s && s->tick() < endTick;
                 s = s->next1(SegmentType::ChordRest)) {
                for (track_idx_t track = startTrack; track < endTrack; ++track) {
                    EngravingItem* el = s->element(track);
                    if (!el || !el->isChord()) {
                        continue;
                    }
                    Chord* ch = toChord(el);
                    repitchChord(ch);
                    for (Chord* g : ch->graceNotes()) {
                        repitchChord(g);
                    }
                }
            }
        }
    }

    return true;
}

Err Read302::readScore(Score* score, XmlReader& e, ReadInOutData* out)
{
    ReadContext ctx(score);
    if (out && out->overriddenSpatium.has_value()) {
        ctx.setSpatium(out->overriddenSpatium.value());
        ctx.setOverrideSpatium(true);
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
                    return Err::FileCriticallyCorrupted;
                }
                return Err::FileBadFormat;
            }

            if (ctx.overrideSpatium() && out) {
                out->originalSpatium = ctx.originalSpatium();
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
