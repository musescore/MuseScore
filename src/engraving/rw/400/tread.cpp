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
#include "tread.h"

#include "../../types/typesconv.h"
#include "../../types/symnames.h"

#include "../../libmscore/score.h"
#include "../../libmscore/factory.h"

#include "../../libmscore/tempotext.h"
#include "../../libmscore/stafftext.h"
#include "../../libmscore/stafftextbase.h"
#include "../../libmscore/dynamic.h"
#include "../../libmscore/expression.h"
#include "../../libmscore/harmony.h"
#include "../../libmscore/chordlist.h"
#include "../../libmscore/fret.h"
#include "../../libmscore/tremolobar.h"
#include "../../libmscore/sticking.h"
#include "../../libmscore/systemtext.h"
#include "../../libmscore/playtechannotation.h"
#include "../../libmscore/rehearsalmark.h"
#include "../../libmscore/instrchange.h"
#include "../../libmscore/staffstate.h"
#include "../../libmscore/figuredbass.h"
#include "../../libmscore/part.h"
#include "../../libmscore/fermata.h"
#include "../../libmscore/image.h"
#include "../../libmscore/tuplet.h"
#include "../../libmscore/text.h"
#include "../../libmscore/beam.h"
#include "../../libmscore/ambitus.h"
#include "../../libmscore/accidental.h"
#include "../../libmscore/marker.h"
#include "../../libmscore/jump.h"
#include "../../libmscore/measurenumber.h"
#include "../../libmscore/mmrestrange.h"
#include "../../libmscore/systemdivider.h"

#include "../xmlreader.h"
#include "../206/read206.h"

#include "textbaserw.h"
#include "propertyrw.h"
#include "engravingitemrw.h"
#include "bsymbolrw.h"
#include "symbolrw.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

template<typename T>
static bool try_read(EngravingItem* el, XmlReader& xml, ReadContext& ctx)
{
    T* t = dynamic_cast<T*>(el);
    if (!t) {
        return false;
    }
    TRead::read(t, xml, ctx);
    return true;
}

void TRead::read(EngravingItem* el, XmlReader& xml, ReadContext& ctx)
{
    if (try_read<Sticking>(el, xml, ctx)) {
    } else if (try_read<SystemText>(el, xml, ctx)) {
    } else if (try_read<Expression>(el, xml, ctx)) {
    } else if (try_read<HBox>(el, xml, ctx)) {
    } else if (try_read<VBox>(el, xml, ctx)) {
    } else if (try_read<TBox>(el, xml, ctx)) {
    } else if (try_read<FBox>(el, xml, ctx)) {
    } else if (try_read<Accidental>(el, xml, ctx)) {
    } else if (try_read<ActionIcon>(el, xml, ctx)) {
    } else if (try_read<Ambitus>(el, xml, ctx)) {
    } else if (try_read<Arpeggio>(el, xml, ctx)) {
    } else if (try_read<Articulation>(el, xml, ctx)) {
    } else if (try_read<BagpipeEmbellishment>(el, xml, ctx)) {
    } else if (try_read<BarLine>(el, xml, ctx)) {
    } else if (try_read<Bend>(el, xml, ctx)) {
    } else if (try_read<Bracket>(el, xml, ctx)) {
    } else if (try_read<Breath>(el, xml, ctx)) {
    } else if (try_read<Chord>(el, xml, ctx)) {
    } else if (try_read<ChordLine>(el, xml, ctx)) {
    } else if (try_read<Clef>(el, xml, ctx)) {
    } else if (try_read<Dynamic>(el, xml, ctx)) {
    } else if (try_read<Fermata>(el, xml, ctx)) {
    } else if (try_read<FiguredBass>(el, xml, ctx)) {
    } else if (try_read<Fingering>(el, xml, ctx)) {
    } else if (try_read<FretDiagram>(el, xml, ctx)) {
    } else if (try_read<Glissando>(el, xml, ctx)) {
    } else if (try_read<GradualTempoChange>(el, xml, ctx)) {
    } else if (try_read<Hairpin>(el, xml, ctx)) {
    } else if (try_read<Harmony>(el, xml, ctx)) {
    } else if (try_read<Image>(el, xml, ctx)) {
    } else if (try_read<InstrumentChange>(el, xml, ctx)) {
    } else if (try_read<Jump>(el, xml, ctx)) {
    } else if (try_read<KeySig>(el, xml, ctx)) {
    } else if (try_read<LayoutBreak>(el, xml, ctx)) {
    } else if (try_read<LetRing>(el, xml, ctx)) {
    } else if (try_read<Marker>(el, xml, ctx)) {
    } else if (try_read<MeasureNumber>(el, xml, ctx)) {
    } else if (try_read<MeasureRepeat>(el, xml, ctx)) {
    } else if (try_read<Note>(el, xml, ctx)) {
    } else if (try_read<Ottava>(el, xml, ctx)) {
    } else if (try_read<PalmMute>(el, xml, ctx)) {
    } else if (try_read<PlayTechAnnotation>(el, xml, ctx)) {
    } else if (try_read<RehearsalMark>(el, xml, ctx)) {
    } else if (try_read<InstrumentChange>(el, xml, ctx)) {
    } else if (try_read<StaffState>(el, xml, ctx)) {
    } else if (try_read<FiguredBass>(el, xml, ctx)) {
    } else if (try_read<Marker>(el, xml, ctx)) {
    } else if (try_read<Jump>(el, xml, ctx)) {
    } else {
        UNREACHABLE;
    }
}

void TRead::read(TextBase* t, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        if (!TextBaseRW::readProperties(t, xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(TempoText* t, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "tempo") {
            t->setTempo(TConv::fromXml(e.readAsciiText(), Constants::defaultTempo));
        } else if (tag == "followText") {
            t->setFollowText(e.readInt());
        } else if (!TextBaseRW::readProperties(t, e, ctx)) {
            e.unknown();
        }
    }
    // check sanity
    if (t->xmlText().isEmpty()) {
        t->setXmlText(String(u"<sym>metNoteQuarterUp</sym> = %1").arg(int(lrint(t->tempo().toBPM().val))));
        t->setVisible(false);
    }
}

void TRead::read(StaffText* t, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<StaffTextBase*>(t), xml, ctx);
}

void TRead::read(StaffTextBase* t, XmlReader& xml, ReadContext& ctx)
{
    t->clear();

    while (xml.readNextStartElement()) {
        if (!readProperties(t, xml, ctx)) {
            xml.unknown();
        }
    }
}

bool TRead::readProperties(StaffTextBase* t, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "MidiAction") {
        int channel = e.intAttribute("channel", 0);
        String name = e.attribute("name");
        bool found = false;
        size_t n = t->channelActions().size();
        for (size_t i = 0; i < n; ++i) {
            ChannelActions* a = &t->channelActions()[i];
            if (a->channel == channel) {
                a->midiActionNames.append(name);
                found = true;
                break;
            }
        }
        if (!found) {
            ChannelActions a;
            a.channel = channel;
            a.midiActionNames.append(name);
            t->channelActions().push_back(a);
        }
        e.readNext();
    } else if (tag == "channelSwitch" || tag == "articulationChange") {
        voice_idx_t voice = static_cast<voice_idx_t>(e.intAttribute("voice", -1));
        if (voice < VOICES) {
            t->setChannelName(voice, e.attribute("name"));
        } else if (voice == mu::nidx) {
            // no voice applies channel to all voices for
            // compatibility
            for (voice_idx_t i = 0; i < VOICES; ++i) {
                t->setChannelName(i, e.attribute("name"));
            }
        }
        e.readNext();
    } else if (tag == "aeolus") {
        int group = e.intAttribute("group", -1);
        if (group >= 0 && group < 4) {
            t->setAeolusStop(group, e.readInt());
        } else {
            e.readNext();
        }
        t->setSetAeolusStops(true);
    } else if (tag == "swing") {
        DurationType swingUnit = TConv::fromXml(e.asciiAttribute("unit"), DurationType::V_INVALID);
        int unit = 0;
        if (swingUnit == DurationType::V_EIGHTH) {
            unit = Constants::division / 2;
        } else if (swingUnit == DurationType::V_16TH) {
            unit = Constants::division / 4;
        } else if (swingUnit == DurationType::V_ZERO) {
            unit = 0;
        }
        int ratio = e.intAttribute("ratio", 60);
        t->setSwing(true);
        t->setSwingParameters(unit, ratio);
        e.readNext();
    } else if (tag == "capo") {
        int fretId = e.intAttribute("fretId", 0);
        t->setCapo(fretId);
        e.readNext();
    } else if (!TextBaseRW::readProperties(t, e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(Dynamic* d, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag = e.name();
        if (tag == "subtype") {
            d->setDynamicType(e.readText());
        } else if (tag == "velocity") {
            d->setVelocity(e.readInt());
        } else if (tag == "dynType") {
            d->setDynRange(TConv::fromXml(e.readAsciiText(), DynamicRange::STAFF));
        } else if (tag == "veloChange") {
            d->setChangeInVelocity(e.readInt());
        } else if (tag == "veloChangeSpeed") {
            d->setVelChangeSpeed(TConv::fromXml(e.readAsciiText(), DynamicSpeed::NORMAL));
        } else if (!TextBaseRW::readProperties(d, e, ctx)) {
        } else if (tag == "avoidBarLines") {
            readProperty(d, e, ctx, Pid::AVOID_BARLINES);
        } else if (tag == "dynamicsSize") {
            readProperty(d, e, ctx, Pid::DYNAMICS_SIZE);
        } else if (tag == "centerOnNotehead") {
            readProperty(d, e, ctx, Pid::CENTER_ON_NOTEHEAD);
        } else if (!readProperties(static_cast<TextBase*>(d), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Expression* expr, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        const AsciiStringView tag = xml.name();
        if (tag == "snapToDynamics") {
            readProperty(expr, xml, ctx, Pid::SNAP_TO_DYNAMICS);
        } else if (!readProperties(static_cast<TextBase*>(expr), xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(Harmony* h, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "base") {
            h->setBaseTpc(e.readInt());
        } else if (tag == "baseCase") {
            h->setBaseCase(static_cast<NoteCaseType>(e.readInt()));
        } else if (tag == "extension") {
            h->setId(e.readInt());
        } else if (tag == "name") {
            h->setTextName(e.readText());
        } else if (tag == "root") {
            h->setRootTpc(e.readInt());
        } else if (tag == "rootCase") {
            h->setRootCase(static_cast<NoteCaseType>(e.readInt()));
        } else if (tag == "function") {
            h->setFunction(e.readText());
        } else if (tag == "degree") {
            int degreeValue = 0;
            int degreeAlter = 0;
            String degreeType;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "degree-value") {
                    degreeValue = e.readInt();
                } else if (t == "degree-alter") {
                    degreeAlter = e.readInt();
                } else if (t == "degree-type") {
                    degreeType = e.readText();
                } else {
                    e.unknown();
                }
            }
            if (degreeValue <= 0 || degreeValue > 13
                || degreeAlter < -2 || degreeAlter > 2
                || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                LOGD("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s",
                     degreeValue, degreeAlter, muPrintable(degreeType));
            } else {
                if (degreeType == "add") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ADD));
                } else if (degreeType == "alter") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ALTER));
                } else if (degreeType == "subtract") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT));
                }
            }
        } else if (tag == "leftParen") {
            h->setLeftParen(true);
            e.readNext();
        } else if (tag == "rightParen") {
            h->setRightParen(true);
            e.readNext();
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::POS_ABOVE)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::HARMONY_TYPE)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::PLAY)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::HARMONY_VOICE_LITERAL)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::HARMONY_VOICING)) {
        } else if (PropertyRW::readProperty(h, tag, e, ctx, Pid::HARMONY_DURATION)) {
        } else if (!TextBaseRW::readProperties(h, e, ctx)) {
            e.unknown();
        }
    }

    h->afterRead();
}

void TRead::read(FretDiagram* d, XmlReader& e, ReadContext& ctx)
{
    // Read the old format first
    bool hasBarre = false;
    bool haveReadNew = false;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        // Check for new format fret diagram
        if (haveReadNew) {
            e.skipCurrentElement();
            continue;
        }
        if (tag == "fretDiagram") {
            // Read new
            while (e.readNextStartElement()) {
                const AsciiStringView tag(e.name());

                if (tag == "string") {
                    int no = e.intAttribute("no");
                    while (e.readNextStartElement()) {
                        const AsciiStringView t(e.name());
                        if (t == "dot") {
                            int fret = e.intAttribute("fret", 0);
                            FretDotType dtype = FretItem::nameToDotType(e.readText());
                            d->setDot(no, fret, true, dtype);
                        } else if (t == "marker") {
                            FretMarkerType mtype = FretItem::nameToMarkerType(e.readText());
                            d->setMarker(no, mtype);
                        } else if (t == "fingering") {
                            e.readText();
                            /*setFingering(no, e.readInt()); NOTE:JT todo */
                        } else {
                            e.unknown();
                        }
                    }
                } else if (tag == "barre") {
                    int start = e.intAttribute("start", -1);
                    int end = e.intAttribute("end", -1);
                    int fret = e.readInt();

                    d->setBarre(start, end, fret);
                } else if (!EngravingItemRW::readProperties(d, e, ctx)) {
                    e.unknown();
                }
            }
            haveReadNew = true;
        }
        // Check for new properties
        else if (tag == "showNut") {
            PropertyRW::readProperty(d, e, ctx, Pid::FRET_NUT);
        } else if (tag == "orientation") {
            PropertyRW::readProperty(d, e, ctx, Pid::ORIENTATION);
        }
        // Then read the rest if there is no new format diagram (compatibility read)
        else if (tag == "strings") {
            PropertyRW::readProperty(d, e, ctx, Pid::FRET_STRINGS);
        } else if (tag == "frets") {
            PropertyRW::readProperty(d, e, ctx, Pid::FRET_FRETS);
        } else if (tag == "fretOffset") {
            PropertyRW::readProperty(d, e, ctx, Pid::FRET_OFFSET);
        } else if (tag == "string") {
            int no = e.intAttribute("no");
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "dot") {
                    d->setDot(no, e.readInt());
                } else if (t == "marker") {
                    d->setMarker(no, Char(e.readInt()) == u'X' ? FretMarkerType::CROSS : FretMarkerType::CIRCLE);
                }
                /*else if (t == "fingering")
                      setFingering(no, e.readInt());*/
                else {
                    e.unknown();
                }
            }
        } else if (tag == "barre") {
            hasBarre = e.readBool();
        } else if (tag == "mag") {
            PropertyRW::readProperty(d, e, ctx, Pid::MAG);
        } else if (tag == "Harmony") {
            Harmony* h = new Harmony(d->score()->dummy()->segment());
            read(h, e, ctx);
            d->add(h);
        } else if (!EngravingItemRW::readProperties(d, e, ctx)) {
            e.unknown();
        }
    }

    // Old handling of barres
    if (hasBarre) {
        for (int s = 0; s < d->strings(); ++s) {
            for (auto& dot : d->dot(s)) {
                if (dot.exists()) {
                    d->setBarre(s, -1, dot.fret);
                    return;
                }
            }
        }
    }
}

void TRead::read(TremoloBar* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        auto tag = e.name();
        if (tag == "point") {
            PitchValue pv;
            pv.time    = e.intAttribute("time");
            pv.pitch   = e.intAttribute("pitch");
            pv.vibrato = e.intAttribute("vibrato");
            b->points().push_back(pv);
            e.readNext();
        } else if (tag == "mag") {
            b->setUserMag(e.readDouble(0.1, 10.0));
        } else if (PropertyRW::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "play") {
            b->setPlay(e.readInt());
        } else if (PropertyRW::readProperty(b, tag, e, ctx, Pid::LINE_WIDTH)) {
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Sticking* s, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<TextBase*>(s), xml, ctx);
}

void TRead::read(SystemText* t, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<StaffTextBase*>(t), xml, ctx);
}

void TRead::read(PlayTechAnnotation* a, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        const AsciiStringView tag(xml.name());

        if (PropertyRW::readProperty(a, tag, xml, ctx, Pid::PLAY_TECH_TYPE)) {
            continue;
        }

        if (!readProperties(static_cast<StaffTextBase*>(a), xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(RehearsalMark* m, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<TextBase*>(m), xml, ctx);
}

void TRead::read(InstrumentChange* c, XmlReader& e, ReadContext& ctx)
{
    Instrument inst;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Instrument") {
            inst.read(e, c->part());
        } else if (tag == "init") {
            c->setInit(e.readBool());
        } else if (!TextBaseRW::readProperties(c, e, ctx)) {
            e.unknown();
        }
    }

    if (c->score()->mscVersion() < 206) {
        // previous versions did not honor transposition of instrument change
        // except in ways that it should not have
        // notes entered before the instrument change was added would not be altered,
        // so original transposition remained in effect
        // notes added afterwards would be transposed by both intervals, resulting in tpc corruption
        // here we set the instrument change to inherit the staff transposition to emulate previous versions
        // in Note::read(), we attempt to fix the tpc corruption
        // There is also code in read206 to try to deal with this, but it is out of date and therefore disabled
        // What this means is, scores created in 2.1 or later should be fine, scores created in 2.0 maybe not so much

        Interval v = c->staff() ? c->staff()->part()->instrument(c->tick())->transpose() : 0;
        inst.setTranspose(v);
    }

    c->setInstrument(inst);
}

void TRead::read(StaffState* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            s->setStaffStateType(StaffStateType(e.readInt()));
        } else if (tag == "Instrument") {
            Instrument i;
            i.read(e, nullptr);
            s->setInstrument(std::move(i));
        } else if (!EngravingItemRW::readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(FiguredBass* b, XmlReader& e, ReadContext& ctx)
{
    String normalizedText;
    int idx = 0;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "ticks") {
            b->setTicks(e.readFraction());
        } else if (tag == "onNote") {
            b->setOnNote(e.readInt() != 0l);
        } else if (tag == "FiguredBassItem") {
            FiguredBassItem* pItem = b->createItem(idx++);
            pItem->setTrack(b->track());
            pItem->setParent(b);
            pItem->read(e);
            b->appendItem(pItem);
            // add item normalized text
            if (!normalizedText.isEmpty()) {
                normalizedText.append('\n');
            }
            normalizedText.append(pItem->normalizedText());
        }
//            else if (tag == "style")
//                  setStyledPropertyListIdx(e.readElementText());
        else if (!TextBaseRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
    // if items could be parsed set normalized text
    if (b->items().size() > 0) {
        b->setXmlText(normalizedText);          // this is the text to show while editing
    }
}

void TRead::read(Fermata* f, XmlReader& e, ReadContext& ctx)
{
    auto readProperties = [](Fermata* f, XmlReader& e, ReadContext& ctx)
    {
        const AsciiStringView tag(e.name());

        if (tag == "subtype") {
            AsciiStringView s = e.readAsciiText();
            SymId id = SymNames::symIdByName(s);
            f->setSymId(id);
        } else if (tag == "play") {
            f->setPlay(e.readBool());
        } else if (tag == "timeStretch") {
            f->setTimeStretch(e.readDouble());
        } else if (tag == "offset") {
            if (f->score()->mscVersion() > 114) {
                EngravingItemRW::readProperties(f, e, ctx);
            } else {
                e.skipCurrentElement();       // ignore manual layout in older scores
            }
        } else if (EngravingItemRW::readProperties(f, e, ctx)) {
        } else {
            return false;
        }
        return true;
    };

    while (e.readNextStartElement()) {
        if (!readProperties(f, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Image* img, XmlReader& e, ReadContext& ctx)
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

void TRead::read(Tuplet* t, XmlReader& e, ReadContext& ctx)
{
    t->setId(e.intAttribute("id", 0));

    Text* number = nullptr;
    Fraction ratio;
    TDuration baseLen;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (PropertyRW::readStyledProperty(t, tag, e, ctx)) {
        } else if (tag == "bold") { //important that these properties are read after number is created
            bool val = e.readInt();
            if (number) {
                number->setBold(val);
            }
            if (t->isStyled(Pid::FONT_STYLE)) {
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "italic") {
            bool val = e.readInt();
            if (number) {
                number->setItalic(val);
            }
            if (t->isStyled(Pid::FONT_STYLE)) {
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "underline") {
            bool val = e.readInt();
            if (number) {
                number->setUnderline(val);
            }
            if (t->isStyled(Pid::FONT_STYLE)) {
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "strike") {
            bool val = e.readInt();
            if (number) {
                number->setStrike(val);
            }
            if (t->isStyled(Pid::FONT_STYLE)) {
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "normalNotes") {
            ratio.setDenominator(e.readInt());
        } else if (tag == "actualNotes") {
            ratio.setNumerator(e.readInt());
        } else if (tag == "p1") {
            t->setUserPoint1(e.readPoint() * t->score()->spatium());
        } else if (tag == "p2") {
            t->setUserPoint2(e.readPoint() * t->score()->spatium());
        } else if (tag == "baseNote") {
            baseLen = TDuration(TConv::fromXml(e.readAsciiText(), DurationType::V_INVALID));
        } else if (tag == "baseDots") {
            baseLen.setDots(e.readInt());
        } else if (tag == "Number") {
            number = Factory::createText(t, TextStyleType::TUPLET);
            number->setComposition(true);
            number->setParent(t);
            Tuplet::resetNumberProperty(number);
            number->read(e);
            number->setVisible(t->visible());         //?? override saved property
            number->setTrack(t->track());
            // move property flags from _number back to tuplet
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN }) {
                t->setPropertyFlags(p, number->propertyFlags(p));
            }
        } else if (!EngravingItemRW::readProperties(t, e, ctx)) {
            e.unknown();
        }
    }

    t->setNumber(number);
    t->setBaseLen(baseLen);
    t->setRatio(ratio);

    Fraction f =  t->baseLen().fraction() * t->ratio().denominator();
    t->setTicks(f.reduced());
}

void TRead::read(Beam* b, XmlReader& e, ReadContext& ctx)
{
    if (b->score()->mscVersion() < 301) {
        b->setId(e.intAttribute("id"));
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "StemDirection") {
            PropertyRW::readProperty(b, e, ctx, Pid::STEM_DIRECTION);
        } else if (tag == "distribute") {
            e.skipCurrentElement(); // obsolete
        } else if (PropertyRW::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "growLeft") {
            b->setGrowLeft(e.readDouble());
        } else if (tag == "growRight") {
            b->setGrowRight(e.readDouble());
        } else if (tag == "Fragment") {
            BeamFragment* f = new BeamFragment;
            int idx = (b->beamDirection() == DirectionV::AUTO || b->beamDirection() == DirectionV::DOWN) ? 0 : 1;
            b->setUserModified(true);
            double _spatium = b->spatium();
            while (e.readNextStartElement()) {
                const AsciiStringView tag1(e.name());
                if (tag1 == "y1") {
                    f->py1[idx] = e.readDouble() * _spatium;
                } else if (tag1 == "y2") {
                    f->py2[idx] = e.readDouble() * _spatium;
                } else {
                    e.unknown();
                }
            }
            b->addBeamFragment(f);
        } else if (tag == "l1" || tag == "l2") {      // ignore
            e.skipCurrentElement();
        } else if (tag == "subtype") {          // obsolete
            e.skipCurrentElement();
        } else if (tag == "y1" || tag == "y2") { // obsolete
            e.skipCurrentElement();
        } else if (!EngravingItemRW::readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Ambitus* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "head") {
            PropertyRW::readProperty(a, e, ctx, Pid::HEAD_GROUP);
        } else if (tag == "headType") {
            PropertyRW::readProperty(a, e, ctx, Pid::HEAD_TYPE);
        } else if (tag == "mirror") {
            PropertyRW::readProperty(a, e, ctx, Pid::MIRROR_HEAD);
        } else if (tag == "hasLine") {
            a->setHasLine(e.readInt());
        } else if (tag == "lineWidth") {
            PropertyRW::readProperty(a, e, ctx, Pid::LINE_WIDTH_SPATIUM);
        } else if (tag == "topPitch") {
            a->setTopPitch(e.readInt(), false);
        } else if (tag == "bottomPitch") {
            a->setBottomPitch(e.readInt(), false);
        } else if (tag == "topTpc") {
            a->setTopTpc(e.readInt(), false);
        } else if (tag == "bottomTpc") {
            a->setBottomTpc(e.readInt(), false);
        } else if (tag == "topAccidental") {
            while (e.readNextStartElement()) {
                if (e.name() == "Accidental") {
                    if (a->score()->mscVersion() < 301) {
                        compat::Read206::readAccidental206(a->topAccidental(), e);
                    } else {
                        TRead::read(a->topAccidental(), e, ctx);
                    }
                } else {
                    e.skipCurrentElement();
                }
            }
        } else if (tag == "bottomAccidental") {
            while (e.readNextStartElement()) {
                if (e.name() == "Accidental") {
                    if (a->score()->mscVersion() < 301) {
                        compat::Read206::readAccidental206(a->bottomAccidental(), e);
                    } else {
                        TRead::read(a->bottomAccidental(), e, ctx);
                    }
                } else {
                    e.skipCurrentElement();
                }
            }
        } else if (EngravingItemRW::readProperties(a, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Accidental* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "bracket") {
            int i = e.readInt();
            if (i == 0 || i == 1 || i == 2) {
                a->setBracket(AccidentalBracket(i));
            }
        } else if (tag == "subtype") {
            a->setSubtype(e.readAsciiText());
        } else if (tag == "role") {
            a->setRole(TConv::fromXml(e.readAsciiText(), AccidentalRole::AUTO));
        } else if (tag == "small") {
            a->setSmall(e.readInt());
        } else if (EngravingItemRW::readProperties(a, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Marker* m, XmlReader& e, ReadContext& ctx)
{
    MarkerType mt = MarkerType::SEGNO;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "label") {
            AsciiStringView s(e.readAsciiText());
            m->setLabel(String::fromAscii(s.ascii()));
            mt = TConv::fromXml(s, MarkerType::USER);
        } else if (!TextBaseRW::readProperties(m, e, ctx)) {
            e.unknown();
        }
    }
    m->setMarkerType(mt);
}

void TRead::read(Jump* j, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "jumpTo") {
            j->setJumpTo(e.readText());
        } else if (tag == "playUntil") {
            j->setPlayUntil(e.readText());
        } else if (tag == "continueAt") {
            j->setContinueAt(e.readText());
        } else if (tag == "playRepeats") {
            j->setPlayRepeats(e.readBool());
        } else if (!TextBaseRW::readProperties(j, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(MeasureNumber* n, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<MeasureNumberBase*>(n), xml, ctx);
}

void TRead::read(MeasureNumberBase* b, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<TextBase*>(b), xml, ctx);
}

void TRead::read(MMRestRange* r, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<MeasureNumberBase*>(r), xml, ctx);
}

void TRead::read(SystemDivider* d, XmlReader& e, ReadContext& ctx)
{
    if (e.attribute("type") == "left") {
        d->setDividerType(SystemDivider::Type::LEFT);

        SymId sym = SymNames::symIdByName(d->score()->styleSt(Sid::dividerLeftSym));
        d->setSym(sym, d->score()->engravingFont());
    } else {
        d->setDividerType(SystemDivider::Type::RIGHT);

        SymId sym = SymNames::symIdByName(d->score()->styleSt(Sid::dividerRightSym));
        d->setSym(sym, d->score()->engravingFont());
    }
    SymbolRW::read(d, e, ctx);
}
