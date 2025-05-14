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
#include "tread.h"

#include "../../types/typesconv.h"
#include "../../types/symnames.h"
#include "../../infrastructure/rtti.h"
#include "../../infrastructure/htmlparser.h"

#include "../../dom/score.h"
#include "../../dom/masterscore.h"
#include "../../dom/factory.h"
#include "../../dom/linkedobjects.h"
#include "../../dom/mscore.h"

#include "../../dom/tempotext.h"
#include "../../dom/stafftext.h"
#include "../../dom/stafftextbase.h"

#include "../../dom/drumset.h"
#include "../../dom/dynamic.h"
#include "../../dom/expression.h"
#include "../../dom/harmony.h"
#include "../../dom/harmonicmark.h"
#include "../../dom/chordlist.h"

#include "../../dom/excerpt.h"

#include "../../dom/fret.h"
#include "../../dom/tremolobar.h"
#include "../../dom/sticking.h"
#include "../../dom/systemtext.h"
#include "../../dom/playtechannotation.h"
#include "../../dom/rehearsalmark.h"

#include "../../dom/instrument.h"
#include "../../dom/instrchange.h"

#include "../../dom/staffstate.h"
#include "../../dom/figuredbass.h"
#include "../../dom/part.h"
#include "../../dom/fermata.h"
#include "../../dom/image.h"
#include "../../dom/tuplet.h"
#include "../../dom/text.h"
#include "../../dom/beam.h"
#include "../../dom/ambitus.h"
#include "../../dom/accidental.h"
#include "../../dom/marker.h"
#include "../../dom/jump.h"
#include "../../dom/measurenumber.h"
#include "../../dom/mmrestrange.h"
#include "../../dom/systemdivider.h"
#include "../../dom/actionicon.h"
#include "../../dom/arpeggio.h"
#include "../../dom/articulation.h"
#include "../../dom/ornament.h"
#include "../../dom/audio.h"
#include "../../dom/bagpembell.h"
#include "../../dom/barline.h"
#include "../../dom/chord.h"
#include "../../dom/bend.h"
#include "../../dom/box.h"
#include "../../dom/layoutbreak.h"
#include "../../dom/stafftypechange.h"
#include "../../dom/bracket.h"
#include "../../dom/breath.h"
#include "../../dom/note.h"
#include "../../dom/noteline.h"
#include "../../dom/spanner.h"
#include "../../dom/fingering.h"
#include "../../dom/notedot.h"
#include "../../dom/chordline.h"
#include "../../dom/timesig.h"
#include "../../dom/lyrics.h"
#include "../../dom/stem.h"
#include "../../dom/stemslash.h"
#include "../../dom/hook.h"
#include "../../dom/page.h"
#include "../../dom/tremolotwochord.h"
#include "../../dom/tremolosinglechord.h"
#include "../../dom/clef.h"
#include "../../dom/glissando.h"
#include "../../dom/gradualtempochange.h"
#include "../../dom/line.h"
#include "../../dom/textlinebase.h"
#include "../../dom/groups.h"
#include "../../dom/harppedaldiagram.h"
#include "../../dom/hairpin.h"
#include "../../dom/keysig.h"
#include "../../dom/layoutbreak.h"
#include "../../dom/ledgerline.h"
#include "../../dom/letring.h"
#include "../../dom/measurerepeat.h"
#include "../../dom/mmrest.h"
#include "../../dom/rest.h"
#include "../../dom/rasgueado.h"
#include "../../dom/slur.h"
#include "../../dom/slurtie.h"
#include "../../dom/spacer.h"
#include "../../dom/stafftype.h"
#include "../../dom/stafftypechange.h"
#include "../../dom/system.h"
#include "../../dom/textline.h"
#include "../../dom/trill.h"
#include "../../dom/vibrato.h"
#include "../../dom/volta.h"
#include "../../dom/tie.h"
#include "../../dom/ottava.h"
#include "../../dom/pedal.h"
#include "../../dom/palmmute.h"
#include "../../dom/segment.h"
#include "../../dom/part.h"
#include "../../dom/whammybar.h"

#include "../xmlreader.h"
#include "../read206/read206.h"
#include "../compat/compatutils.h"
#include "readcontext.h"
#include "connectorinforeader.h"

#include "log.h"

using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::read400;

using ReadTypes = rtti::TypeList<Accidental, ActionIcon, Ambitus, Arpeggio, Articulation,
                                 BagpipeEmbellishment, BarLine, Beam, Bend,  HBox, VBox, FBox, TBox, Bracket, Breath,
                                 Chord, ChordLine, Clef,
                                 Dynamic, Expression,
                                 Fermata, FiguredBass, Fingering, FretDiagram,
                                 Glissando, GradualTempoChange,
                                 Hairpin, Harmony, HarmonicMark, HarpPedalDiagram, Hook,
                                 Image, InstrumentChange,
                                 Jump,
                                 KeySig,
                                 LayoutBreak, LedgerLine, LetRing, Lyrics,
                                 Marker, MeasureNumber, MeasureRepeat, MMRest, MMRestRange,
                                 Note, NoteDot, NoteHead, NoteLine,
                                 Page, PalmMute, Pedal, PlayTechAnnotation,
                                 Rasgueado, RehearsalMark, Rest,
                                 Ornament, Ottava,
                                 Segment, Slur, Spacer, StaffState, StaffText, StaffTypeChange, Stem, StemSlash, Sticking,
                                 Symbol, FSymbol, System, SystemDivider, SystemText,
                                 TempoText, Text, TextLine, Tie, TimeSig, TremoloBar, Trill, Tuplet,
                                 Vibrato, Volta,
                                 WhammyBar>;

class ReadVisitor : public rtti::Visitor<ReadVisitor>
{
public:
    template<typename T>
    static bool doVisit(EngravingItem* item, XmlReader& xml, ReadContext& ctx)
    {
        if (T::classof(item)) {
            TRead::read(static_cast<T*>(item), xml, ctx);
            return true;
        }
        return false;
    }
};

void TRead::readItem(EngravingItem* item, XmlReader& xml, ReadContext& ctx)
{
    bool found = ReadVisitor::visit(ReadTypes {}, item, xml, ctx);
    DO_ASSERT(found);
}

PropertyValue TRead::readPropertyValue(Pid id, XmlReader& e, ReadContext& ctx)
{
    switch (propertyType(id)) {
    case P_TYPE::BOOL:
        return PropertyValue(bool(e.readInt()));
    case P_TYPE::INT:
        return PropertyValue(e.readInt());
    case P_TYPE::REAL:
        return PropertyValue(e.readDouble());
    case P_TYPE::SPATIUM: return PropertyValue(Spatium(e.readDouble()));
    case P_TYPE::MILLIMETRE: return PropertyValue(Spatium(e.readDouble())); //! NOTE type mm, but stored in xml as spatium
    case P_TYPE::TEMPO:
        return PropertyValue(e.readDouble());
    case P_TYPE::FRACTION:
        return PropertyValue::fromValue(e.readFraction());

    case P_TYPE::SYMID:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), SymId::noSym));
    case P_TYPE::COLOR:
        return PropertyValue::fromValue(e.readColor());
    case P_TYPE::ORNAMENT_STYLE:
        return PropertyValue::fromValue(TConv::fromXml(e.readAsciiText(), OrnamentStyle::DEFAULT));
    case P_TYPE::ORNAMENT_INTERVAL:
        return PropertyValue(TConv::fromXml(e.readText(), DEFAULT_ORNAMENT_INTERVAL));
    case P_TYPE::POINT:
        return PropertyValue::fromValue(e.readPoint());
    case P_TYPE::SCALE:
        return PropertyValue::fromValue(e.readScale());
    case P_TYPE::SIZE:
        return PropertyValue::fromValue(e.readSize());
    case P_TYPE::STRING:
        return PropertyValue(e.readText());

    case P_TYPE::ALIGN:
        return PropertyValue(TConv::fromXml(e.readText(), Align()));
    case P_TYPE::PLACEMENT_V:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), PlacementV::ABOVE));
    case P_TYPE::PLACEMENT_H:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), PlacementH::LEFT));
    case P_TYPE::TEXT_PLACE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), TextPlace::AUTO));
    case P_TYPE::DIRECTION_V:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), DirectionV::AUTO));
    case P_TYPE::DIRECTION_H:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), DirectionH::AUTO));
    case P_TYPE::ORIENTATION:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), Orientation::VERTICAL));

    case P_TYPE::LAYOUTBREAK_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), LayoutBreakType::NOBREAK));
    case P_TYPE::VELO_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), VeloType::OFFSET_VAL));
    case P_TYPE::GLISS_STYLE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), GlissandoStyle::CHROMATIC));
    case P_TYPE::BARLINE_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), BarLineType::NORMAL));

    case P_TYPE::NOTEHEAD_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), NoteHeadType::HEAD_AUTO));
    case P_TYPE::NOTEHEAD_SCHEME:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), NoteHeadScheme::HEAD_AUTO));
    case P_TYPE::NOTEHEAD_GROUP:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), NoteHeadGroup::HEAD_NORMAL));

    case P_TYPE::CLEF_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), ClefType::G));

    case P_TYPE::DYNAMIC_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), DynamicType::OTHER));

    case P_TYPE::LINE_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), LineType::SOLID));
    case P_TYPE::HOOK_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), HookType::NONE));

    case P_TYPE::KEY_MODE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), KeyMode::NONE));

    case P_TYPE::TEXT_STYLE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), TextStyleType::DEFAULT));

    case P_TYPE::CHANGE_METHOD:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), ChangeMethod::NORMAL));

    case P_TYPE::BEAM_MODE:
        return PropertyValue(int(0));
    case P_TYPE::GROUPS: {
        Groups g;
        read400::TRead::read(&g, e, ctx);
        return PropertyValue::fromValue(g.nodes());
    }
    case P_TYPE::DURATION_TYPE_WITH_DOTS:
    case P_TYPE::INT_VEC:
        return PropertyValue();

    case P_TYPE::PLAYTECH_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), PlayingTechniqueType::Natural));
    case P_TYPE::TEMPOCHANGE_TYPE:
        return PropertyValue(TConv::fromXml(e.readAsciiText(), GradualTempoChangeType::Undefined));
    default:
        ASSERT_X("unhandled PID type");
        break;
    }
    return PropertyValue();
}

bool TRead::readProperty(EngravingItem* item, const AsciiStringView& tag, XmlReader& xml, ReadContext& ctx, Pid pid)
{
    if (tag == propertyName(pid)) {
        readProperty(item, xml, ctx, pid);
        return true;
    }
    return false;
}

void TRead::readProperty(EngravingItem* item, XmlReader& xml, ReadContext& ctx, Pid pid)
{
    double spatium = ctx.score() ? ctx.spatium() : item->spatium();
    PropertyValue v = readPropertyValue(pid, xml, ctx);
    switch (propertyType(pid)) {
    case P_TYPE::MILLIMETRE: //! NOTE type mm, but stored in xml as spatium
        v = v.value<Spatium>().toMM(spatium);
        break;
    case P_TYPE::POINT:
        if (item->offsetIsSpatiumDependent()) {
            v = v.value<PointF>() * spatium;
        } else {
            v = v.value<PointF>() * DPMM;
        }
        break;
    default:
        break;
    }

    // Pre-4.4 compatibility: these items now use DIRECTION property
    if (pid == Pid::PLACEMENT && item->hasVoiceAssignmentProperties()) {
        pid = Pid::DIRECTION;
        v = v.value<PlacementV>() == PlacementV::ABOVE ? PropertyValue(DirectionV::UP) : PropertyValue(DirectionV::DOWN);
    }

    if (!ctx.shouldSkipProperty(pid)) {
        item->setProperty(pid, v);
        if (item->isStyled(pid)) {
            item->setPropertyFlags(pid, PropertyFlags::UNSTYLED);
        }
    }
}

bool TRead::readStyledProperty(EngravingItem* item, const AsciiStringView& tag, XmlReader& xml, ReadContext& ctx)
{
    for (const StyledProperty& spp : *item->styledProperties()) {
        if (readProperty(item, tag, xml, ctx, spp.pid)) {
            return true;
        }
    }
    return false;
}

bool TRead::readItemProperties(EngravingItem* item, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (TRead::readProperty(item, tag, e, ctx, Pid::SIZE_SPATIUM_DEPENDENT)) {
    } else if (TRead::readProperty(item, tag, e, ctx, Pid::OFFSET)) {
    } else if (TRead::readProperty(item, tag, e, ctx, Pid::MIN_DISTANCE)) {
    } else if (TRead::readProperty(item, tag, e, ctx, Pid::AUTOPLACE)) {
    } else if (tag == "track") {
        item->setTrack(e.readInt() + ctx.trackOffset());
    } else if (tag == "color") {
        item->setColor(e.readColor());
    } else if (tag == "visible") {
        item->setVisible(e.readInt());
    } else if (tag == "selected") { // obsolete
        e.readInt();
    } else if ((tag == "linked") || (tag == "linkedMain")) {
        Staff* s = item->staff();
        if (!s) {
            s = ctx.score()->staff(ctx.track() / VOICES);
            if (!s) {
                LOGW("EngravingItem::readProperties: linked element's staff not found (%s)", item->typeName());
                e.skipCurrentElement();
                return true;
            }
        }
        if (tag == "linkedMain") {
            item->setLinks(new LinkedObjects());
            item->links()->push_back(item);

            ctx.addLink(s, item->links(), ctx.location(true));

            e.readNext();
        } else {
            Staff* ls = s->links() ? toStaff(s->links()->mainElement()) : nullptr;
            bool linkedIsMaster = ls ? ls->score()->isMaster() : false;
            Location loc = ctx.location(true);
            if (ls) {
                loc.setStaff(static_cast<int>(ls->idx()));
            }
            Location mainLoc = Location::relative();
            bool locationRead = false;
            int localIndexDiff = 0;
            while (e.readNextStartElement()) {
                const AsciiStringView ntag(e.name());

                if (ntag == "score") {
                    String val(e.readText());
                    if (val == "same") {
                        linkedIsMaster = item->score()->isMaster();
                    }
                } else if (ntag == "location") {
                    TRead::read(&mainLoc, e, ctx);
                    mainLoc.toAbsolute(loc);
                    locationRead = true;
                } else if (ntag == "indexDiff") {
                    localIndexDiff = e.readInt();
                } else {
                    e.unknown();
                }
            }
            if (!locationRead) {
                mainLoc = loc;
            }
            LinkedObjects* link = ctx.getLink(linkedIsMaster, mainLoc, localIndexDiff);
            if (link) {
                EngravingObject* linked = link->mainElement();
                if (linked->type() == item->type()) {
                    item->linkTo(linked);
                } else {
                    LOGW("EngravingItem::readProperties: linked elements have different types: %s, %s. Input file corrupted?",
                         item->typeName(), linked->typeName());
                }
            }
            if (!item->links()) {
                LOGW("EngravingItem::readProperties: could not link %s at staff %d", item->typeName(), mainLoc.staff() + 1);
            }
        }
    } else if (tag == "tick") {
        int val = e.readInt();
        if (val >= 0) {
            ctx.setTick(Fraction::fromTicks(ctx.fileDivision(val)));             // obsolete
        }
    } else if (tag == "pos") {           // obsolete
        TRead::readProperty(item, e, ctx, Pid::OFFSET);
    } else if (tag == "voice") {
        item->setVoice(e.readInt());
    } else if (tag == "tag") {
        e.skipCurrentElement();
    } else if (TRead::readProperty(item, tag, e, ctx, Pid::PLACEMENT)) {
    } else if (tag == "z") {
        item->setZ(e.readInt());
    } else {
        return false;
    }
    return true;
}

void TRead::read(TextBase* t, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        if (!readProperties(static_cast<TextBase*>(t), xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(TempoText* t, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (readProperty(t, tag, e, ctx, Pid::PLAY)) {
        } else if (tag == "tempo") {
            t->setTempo(TConv::fromXml(e.readAsciiText(), Constants::DEFAULT_TEMPO));
        } else if (tag == "followText") {
            t->setFollowText(e.readInt());
        } else if (!readProperties(static_cast<TextBase*>(t), e, ctx)) {
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
        } else if (voice == muse::nidx) {
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
            unit = Constants::DIVISION / 2;
        } else if (swingUnit == DurationType::V_16TH) {
            unit = Constants::DIVISION / 4;
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
    } else if (!readProperties(static_cast<TextBase*>(t), e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(Dynamic* d, XmlReader& e, ReadContext& ctx)
{
    int mscVersion = d->score()->mscVersion();
    while (e.readNextStartElement()) {
        const AsciiStringView tag = e.name();
        if (tag == "subtype") {
            d->setDynamicType(e.readText());
        } else if (tag == "velocity") {
            d->setVelocity(e.readInt());
        } else if (tag == "dynType") {
            d->setDynRange(TConv::fromXml(e.readAsciiText(), DynamicRange::PART));
        } else if (tag == "veloChange") {
            d->setChangeInVelocity(e.readInt());
        } else if (tag == "veloChangeSpeed") {
            d->setVelChangeSpeed(TConv::fromXml(e.readAsciiText(), DynamicSpeed::NORMAL));
        } else if (tag == "size" && mscVersion < 400) {
            e.skipCurrentElement();
        } else if (readProperty(d, tag, e, ctx, Pid::AVOID_BARLINES)) {
        } else if (readProperty(d, tag, e, ctx, Pid::DYNAMICS_SIZE)) {
        } else if (readProperty(d, tag, e, ctx, Pid::CENTER_ON_NOTEHEAD)) {
        } else if (!readProperties(static_cast<TextBase*>(d), e, ctx)) {
            e.unknown();
        }
    }
    if (mscVersion < 400) {
        d->setSize(10.0);
        if (d->xmlText().contains(u"<sym>") && !d->xmlText().contains(u"<font")) {
            d->setAlign(Align(AlignH::HCENTER, AlignV::BASELINE));
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
                const AsciiStringView tag2(e.name());

                if (tag2 == "string") {
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
                } else if (tag2 == "barre") {
                    int start = e.intAttribute("start", -1);
                    int end = e.intAttribute("end", -1);
                    int fret = e.readInt();

                    d->setBarre(start, end, fret);
                } else if (!readItemProperties(d, e, ctx)) {
                    e.unknown();
                }
            }
            haveReadNew = true;
        }
        // Check for new properties
        else if (tag == "showNut") {
            TRead::readProperty(d, e, ctx, Pid::FRET_NUT);
        } else if (tag == "orientation") {
            TRead::readProperty(d, e, ctx, Pid::ORIENTATION);
        }
        // Then read the rest if there is no new format diagram (compatibility read)
        else if (tag == "strings") {
            TRead::readProperty(d, e, ctx, Pid::FRET_STRINGS);
        } else if (tag == "frets") {
            TRead::readProperty(d, e, ctx, Pid::FRET_FRETS);
        } else if (tag == "fretOffset") {
            TRead::readProperty(d, e, ctx, Pid::FRET_OFFSET);
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
            TRead::readProperty(d, e, ctx, Pid::MAG);
        } else if (tag == "Harmony") {
            Harmony* h = new Harmony(d->score()->dummy()->segment());
            read(h, e, ctx);
            d->add(h);
        } else if (!readItemProperties(d, e, ctx)) {
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
        } else if (TRead::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "play") {
            b->setPlay(e.readInt());
        } else if (TRead::readProperty(b, tag, e, ctx, Pid::LINE_WIDTH)) {
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

        if (TRead::readProperty(a, tag, xml, ctx, Pid::PLAY_TECH_TYPE)) {
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

void TRead::read(Instrument* item, XmlReader& e, ReadContext& ctx, Part* part)
{
    bool customDrumset = false;
    bool readSingleNoteDynamics = false;

    item->clearChannels();         // remove default channel
    item->setId(e.attribute("id"));
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "singleNoteDynamics") {
            item->setSingleNoteDynamics(e.readBool());
            readSingleNoteDynamics = true;
        } else if (!readProperties(item, e, ctx, part, &customDrumset)) {
            e.unknown();
        }
    }

    item->updateInstrumentId();

    if (item->channel(0) && item->channel(0)->program() == -1) {
        item->channel(0)->setProgram(item->recognizeMidiProgram());
    }

    if (!readSingleNoteDynamics) {
        item->setSingleNoteDynamicsFromTemplate();
    }
}

bool TRead::readProperties(Instrument* item, XmlReader& e, ReadContext& ctx, Part* part, bool* customDrumset)
{
    PartAudioSettingsCompat partAudioSetting;
    InstrumentTrackId trackId;
    if (part && part->score()) {
        trackId = { part->score()->parts().size() + 1, item->id() };//part is not assigned to score, _id field is not correct
    }
    partAudioSetting.instrumentId = trackId;

    const AsciiStringView tag(e.name());
    if (tag == "longName") {
        StaffName name;
        TRead::read(&name, e);
        item->appendLongName(name);
    } else if (tag == "shortName") {
        StaffName name;
        TRead::read(&name, e);
        item->appendShortName(name);
    } else if (tag == "trackName") {
        item->setTrackName(e.readText());
    } else if (tag == "minPitch") {      // obsolete
        int pitch = e.readInt();
        item->setMinPitchP(pitch);
        item->setMinPitchA(pitch);
    } else if (tag == "maxPitch") {       // obsolete
        int pitch = e.readInt();
        item->setMaxPitchP(pitch);
        item->setMaxPitchA(pitch);
    } else if (tag == "minPitchA") {
        item->setMinPitchA(e.readInt());
    } else if (tag == "minPitchP") {
        item->setMinPitchP(e.readInt());
    } else if (tag == "maxPitchA") {
        item->setMaxPitchA(e.readInt());
    } else if (tag == "maxPitchP") {
        item->setMaxPitchP(e.readInt());
    } else if (tag == "transposition") {    // obsolete
        Interval transpose;
        transpose.chromatic = e.readInt();
        transpose.diatonic = chromatic2diatonic(transpose.chromatic);
        item->setTranspose(transpose);
    } else if (tag == "transposeChromatic") {
        Interval transpose = item->transpose();
        transpose.chromatic = e.readInt();
        item->setTranspose(transpose);
    } else if (tag == "transposeDiatonic") {
        Interval transpose = item->transpose();
        transpose.diatonic = e.readInt();
        item->setTranspose(transpose);
    } else if (tag == "instrumentId") {
        item->setMusicXmlId(e.readText());
    } else if (tag == "useDrumset") {
        item->setUseDrumset(e.readInt());
        if (item->useDrumset()) {
            item->setDrumset(new Drumset(*smDrumset));
        }
    } else if (tag == "Drum") {
        // if we see on of this tags, a custom drumset will
        // be created
        if (!item->drumset()) {
            item->setDrumset(new Drumset(*smDrumset));
        }
        if (!(*customDrumset)) {
            const_cast<Drumset*>(item->drumset())->clear();
            *customDrumset = true;
        }
        const_cast<Drumset*>(item->drumset())->load(e);
    }
    // support tag "Tablature" for a while for compatibility with existent 2.0 scores
    else if (tag == "Tablature" || tag == "StringData") {
        StringData sd;
        read(&sd, e);
        item->setStringData(sd);
    } else if (tag == "MidiAction") {
        NamedEventList a;
        read(&a, e);
        item->addMidiAction(a);
    } else if (tag == "Articulation") {
        MidiArticulation a;
        read(&a, e);
        item->addMidiArticulation(a);
    } else if (tag == "Channel" || tag == "channel") {
        InstrChannel* a = new InstrChannel;
        read(a, e, ctx, part, trackId);
        item->appendChannel(a);
    } else if (tag == "clef") {           // sets both transposing and concert clef
        int idx = e.intAttribute("staff", 1) - 1;
        ClefType ct = TConv::fromXml(e.readAsciiText(), ClefType::G);
        item->setClefType(idx, ClefTypeList(ct, ct));
    } else if (tag == "concertClef") {
        int idx = e.intAttribute("staff", 1) - 1;
        item->setClefType(idx, ClefTypeList(TConv::fromXml(e.readAsciiText(), ClefType::G), item->clefType(idx).transposingClef));
    } else if (tag == "transposingClef") {
        int idx = e.intAttribute("staff", 1) - 1;
        item->setClefType(idx, ClefTypeList(item->clefType(idx).concertClef, TConv::fromXml(e.readAsciiText(), ClefType::G)));
    } else {
        return false;
    }

    return true;
}

void TRead::read(InstrChannel* item, XmlReader& e, ReadContext& ctx, Part* part, const InstrumentTrackId& instrId)
{
    item->setNotifyAboutChangedEnabled(false);

    // synti = 0;
    item->setName(e.attribute("name"));
    if (item->name() == "") {
        item->setName(String::fromUtf8(InstrChannel::DEFAULT_NAME));
    }

    int midiPort = -1;
    int midiChannel = -1;
    PartAudioSettingsCompat partAudioSetting;
    partAudioSetting.instrumentId = instrId;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "program") {
            item->setProgram(e.intAttribute("value", -1));
            if (item->program() == -1) {
                item->setProgram(e.readInt());
            } else {
                e.readNext();
            }
        } else if (tag == "controller") {
            int value = e.intAttribute("value", 0);
            int ctrl  = e.intAttribute("ctrl", 0);
            switch (ctrl) {
            case CTRL_HBANK:
                item->setBank((value << 7) + (item->bank() & 0x7f));
                item->setUserBankController(true);
                break;
            case CTRL_LBANK:
                item->setBank((item->bank() & ~0x7f) + (value & 0x7f));
                item->setUserBankController(true);
                break;
            case CTRL_VOLUME:
                item->setVolume(value);
                break;
            case CTRL_PANPOT:
                item->setPan(value);
                break;
            case CTRL_CHORUS_SEND:
                item->setChorus(value);
                break;
            case CTRL_REVERB_SEND:
                item->setReverb(value);
                break;
            default:
            {
                Event ev(ME_CONTROLLER);
                ev.setOntime(-1);
                ev.setChannel(0);
                ev.setDataA(ctrl);
                ev.setDataB(value);
                item->addToInit(ev);
            }
            break;
            }
            e.readNext();
        } else if (tag == "Articulation") {
            MidiArticulation a;
            read(&a, e);
            item->articulation.push_back(a);
        } else if (tag == "MidiAction") {
            NamedEventList a;
            read(&a, e);
            item->midiActions.push_back(a);
        } else if (tag == "synti") {
            item->setSynti(e.readText());
        } else if (tag == "color") {
            item->setColor(e.readInt());
        } else if (tag == "mute") {
            partAudioSetting.mute = e.readInt();
        } else if (tag == "solo") {
            partAudioSetting.solo = e.readInt();
        } else if (tag == "midiPort") {
            midiPort = e.readInt();
        } else if (tag == "midiChannel") {
            midiChannel = e.readInt();
        } else {
            e.unknown();
        }
    }

    item->setMustUpdateInit(true);

    ctx.addPartAudioSettingCompat(partAudioSetting);

    item->setNotifyAboutChangedEnabled(true);

    if ((midiPort != -1 || midiChannel != -1) && part && part->score()->isMaster()) {
        part->masterScore()->addMidiMapping(item, part, midiPort, midiChannel);
    }
}

void TRead::read(MidiArticulation* item, XmlReader& e)
{
    item->name = e.attribute("name");
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "velocity") {
            String text(e.readText());
            if (text.endsWith(u"%")) {
                text = text.left(text.size() - 1);
            }
            item->velocity = text.toInt();
        } else if (tag == "gateTime") {
            String text(e.readText());
            if (text.endsWith(u"%")) {
                text = text.left(text.size() - 1);
            }
            item->gateTime = text.toInt();
        } else if (tag == "descr") {
            item->descr = e.readText();
        } else {
            e.unknown();
        }
    }
}

void TRead::read(NamedEventList* item, XmlReader& e)
{
    item->name = e.attribute("name");
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "program") {
            MidiCoreEvent ev(ME_CONTROLLER, 0, CTRL_PROGRAM, e.intAttribute("value", 0));
            item->events.push_back(ev);
            e.skipCurrentElement();
        } else if (tag == "controller") {
            MidiCoreEvent ev;
            ev.setType(ME_CONTROLLER);
            ev.setDataA(e.intAttribute("ctrl", 0));
            ev.setDataB(e.intAttribute("value", 0));
            item->events.push_back(ev);
            e.skipCurrentElement();
        } else if (tag == "descr") {
            item->descr = e.readText();
        } else {
            e.unknown();
        }
    }
}

void TRead::read(InstrumentChange* c, XmlReader& e, ReadContext& ctx)
{
    Instrument inst;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Instrument") {
            read(&inst, e, ctx, c->part());
        } else if (tag == "init") {
            c->setInit(e.readBool());
        } else if (!readProperties(static_cast<TextBase*>(c), e, ctx)) {
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

//---------------------------------------------------------
//    for import of 1.3 scores
//---------------------------------------------------------

static SymId convertFromOldId(int val)
{
    SymId symId = SymId::noSym;
    switch (val) {
    case 32: symId = SymId::accidentalSharp;
        break;
    case 33: symId = SymId::accidentalThreeQuarterTonesSharpArrowUp;
        break;
    case 34: symId = SymId::accidentalQuarterToneSharpArrowDown;
        break;
    // case 35: // "sharp arrow both" missing in SMuFL
    case 36: symId = SymId::accidentalQuarterToneSharpStein;
        break;
    case 37: symId = SymId::accidentalBuyukMucennebSharp;
        break;
    case 38: symId = SymId::accidentalKomaSharp;
        break;
    case 39: symId = SymId::accidentalThreeQuarterTonesSharpStein;
        break;
    case 40: symId = SymId::accidentalNatural;
        break;
    case 41: symId = SymId::accidentalQuarterToneSharpNaturalArrowUp;
        break;
    case 42: symId = SymId::accidentalQuarterToneFlatNaturalArrowDown;
        break;
    // case 43: // "natural arrow both" missing in SMuFL
    case 44: symId = SymId::accidentalFlat;
        break;
    case 45: symId = SymId::accidentalQuarterToneFlatArrowUp;
        break;
    case 46: symId = SymId::accidentalThreeQuarterTonesFlatArrowDown;
        break;
    // case 47: // "flat arrow both" missing in SMuFL
    case 48: symId = SymId::accidentalBakiyeFlat;
        break;
    case 49: symId = SymId::accidentalBuyukMucennebFlat;
        break;
    case 50: symId = SymId::accidentalThreeQuarterTonesFlatZimmermann;
        break;
    case 51: symId = SymId::accidentalQuarterToneFlatStein;
        break;
    // case 52: // "mirrored flat slash" missing in SMuFL
    case 53: symId = SymId::accidentalDoubleFlat;
        break;
    // case 54: // "flat flat slash" missing in SMuFL
    case 55: symId = SymId::accidentalDoubleSharp;
        break;
    case 56: symId = SymId::accidentalSori;
        break;
    case 57: symId = SymId::accidentalKoron;
        break;
    default:
        LOGD("MuseScore 1.3 symbol id corresponding to <%d> not found", val);
        symId = SymId::noSym;
        break;
    }
    return symId;
}

void TRead::read(KeyList* item, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (e.name() == "key") {
            Key k;
            int tick = e.intAttribute("tick", 0);
            if (e.hasAttribute("custom")) {
                k = Key::C;              // ke.setCustomType(e.intAttribute("custom"));
            } else {
                k = Key(e.intAttribute("idx"));
            }
            KeySigEvent ke;
            ke.setConcertKey(k);
            (*item)[ctx.fileDivision(tick)] = ke;
            e.readNext();
        } else {
            e.unknown();
        }
    }
}

void TRead::read(KeySig* s, XmlReader& e, ReadContext& ctx)
{
    KeySigEvent sig;
    int subtype = 0;
    Part* p = s->part();

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "CustDef" || tag == "KeySym") {
            CustDef cd;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "sym") {
                    AsciiStringView val(e.readAsciiText());
                    bool valid;
                    SymId id = SymId(val.toInt(&valid));
                    if (!valid) {
                        id = SymNames::symIdByName(val);
                    }
                    if (s->score()->mscVersion() <= 114) {
                        if (valid) {
                            id = convertFromOldId(val.toInt(&valid));
                        }
                        if (!valid) {
                            id = SymNames::symIdByOldName(val);
                        }
                    }
                    cd.sym = id;
                } else if (t == "def") {
                    cd.degree = e.intAttribute("degree", 0);
                    cd.octAlt = e.intAttribute("octAlt", 0);
                    cd.xAlt = e.doubleAttribute("xAlt", 0.0);
                    e.readNext();
                } else if (t == "pos") { // for older files
                    double prevx = 0;
                    double accidentalGap = ctx.score()->style().styleS(Sid::keysigAccidentalDistance).val();
                    double _spatium = s->spatium();
                    // count default x position
                    for (CustDef& cd2 : sig.customKeyDefs()) {
                        prevx += s->symWidth(cd2.sym) / _spatium + accidentalGap + cd2.xAlt;
                    }
                    bool flat = std::string(SymNames::nameForSymId(cd.sym).ascii()).find("Flat") != std::string::npos;
                    // if x not there, use default step
                    cd.xAlt = e.doubleAttribute("x", prevx) - prevx;
                    // if y not there, use middle line
                    int line = static_cast<int>(e.doubleAttribute("y", 2) * 2);
                    cd.degree = (3 - line) % 7;
                    cd.degree += (cd.degree < 0) ? 7 : 0;
                    line += flat ? -1 : 1; // top point in treble for # is -1 (gis), and for b is 1 (es)
                    cd.octAlt = static_cast<int>((line - (line >= 0 ? 0 : 6)) / 7);
                    e.readNext();
                } else {
                    e.unknown();
                }
            }
            sig.customKeyDefs().push_back(cd);
        } else if (tag == "showCourtesySig") {
            s->setShowCourtesy(e.readInt());
        } else if (tag == "showNaturals") {           // obsolete
            e.readInt();
        } else if (tag == "accidental") {             // we need to guess proper concert key
            Key key = Key(e.readInt());
            Key cKey = key;
            if (p && !s->concertPitch()) {
                Interval v = p->instrument(s->tick())->transpose();
                if (!v.isZero()) {
                    cKey = transposeKey(key, v);
                }
            }
            sig.setConcertKey(cKey);
            sig.setKey(key);
        } else if (tag == "natural") {                // obsolete
            e.readInt();
        } else if (tag == "custom") {
            e.readInt();
            sig.setCustom(true);
        } else if (tag == "mode") {
            String m(e.readText());
            if (m == "none") {
                sig.setMode(KeyMode::NONE);
            } else if (m == "major") {
                sig.setMode(KeyMode::MAJOR);
            } else if (m == "minor") {
                sig.setMode(KeyMode::MINOR);
            } else if (m == "dorian") {
                sig.setMode(KeyMode::DORIAN);
            } else if (m == "phrygian") {
                sig.setMode(KeyMode::PHRYGIAN);
            } else if (m == "lydian") {
                sig.setMode(KeyMode::LYDIAN);
            } else if (m == "mixolydian") {
                sig.setMode(KeyMode::MIXOLYDIAN);
            } else if (m == "aeolian") {
                sig.setMode(KeyMode::AEOLIAN);
            } else if (m == "ionian") {
                sig.setMode(KeyMode::IONIAN);
            } else if (m == "locrian") {
                sig.setMode(KeyMode::LOCRIAN);
            } else {
                sig.setMode(KeyMode::UNKNOWN);
            }
        } else if (tag == "subtype") {
            subtype = e.readInt();
        } else if (tag == "forInstrumentChange") {
            sig.setForInstrumentChange(e.readBool());
        } else if (!readItemProperties(s, e, ctx)) {
            e.unknown();
        }
    }
    // for backward compatibility
    if (!sig.isValid()) {
        sig.initFromSubtype(subtype);
    }
    if (sig.custom() && sig.customKeyDefs().empty()) {
        sig.setMode(KeyMode::NONE);
    }
    // if there are more than 6 accidentals in transposing key, it cannot be PreferSharpFlat::AUTO
    if (p && !s->concertPitch() && (sig.key() > 6 || sig.key() < -6)
        && p->preferSharpFlat() == PreferSharpFlat::AUTO && !p->instrument(s->tick())->transpose().isZero()) {
        p->setPreferSharpFlat(PreferSharpFlat::NONE);
    }

    s->setKeySigEvent(sig);
}

void TRead::read(StaffState* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            s->setStaffStateType(StaffStateType(e.readInt()));
        } else if (tag == "Instrument") {
            Instrument i;
            read(&i, e, ctx, nullptr);
            s->setInstrument(std::move(i));
        } else if (!readItemProperties(s, e, ctx)) {
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
            TRead::read(pItem, e, ctx);
            b->appendItem(pItem);
            // add item normalized text
            if (!normalizedText.isEmpty()) {
                normalizedText.append('\n');
            }
            normalizedText.append(pItem->normalizedText());
        }
//            else if (tag == "style")
//                  setStyledPropertyListIdx(e.readElementText());
        else if (!readProperties(static_cast<TextBase*>(b), e, ctx)) {
            e.unknown();
        }
    }
    // if items could be parsed set normalized text
    if (b->items().size() > 0) {
        b->setXmlText(normalizedText);          // this is the text to show while editing
    }
}

void TRead::read(FiguredBassItem* i, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "brackets") {
            i->setParenth1((FiguredBassItem::Parenthesis)e.intAttribute("b0"));
            i->setParenth2((FiguredBassItem::Parenthesis)e.intAttribute("b1"));
            i->setParenth3((FiguredBassItem::Parenthesis)e.intAttribute("b2"));
            i->setParenth4((FiguredBassItem::Parenthesis)e.intAttribute("b3"));
            i->setParenth5((FiguredBassItem::Parenthesis)e.intAttribute("b4"));
            e.readNext();
        } else if (tag == "prefix") {
            i->setPrefix((FiguredBassItem::Modifier)(e.readInt()));
        } else if (tag == "digit") {
            i->setDigit(e.readInt());
        } else if (tag == "suffix") {
            i->setSuffix((FiguredBassItem::Modifier)(e.readInt()));
        } else if (tag == "continuationLine") {
            i->setContLine((FiguredBassItem::ContLine)(e.readInt()));
        } else if (!readItemProperties(i, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Excerpt* item, XmlReader& e, ReadContext&)
{
    const std::vector<Part*>& pl = item->masterScore()->parts();
    std::vector<Part*> parts;
    while (e.readNextStartElement()) {
        const AsciiStringView tag = e.name();
        if (tag == "name" || tag == "title") {
            item->setName(e.readText().trimmed());
        } else if (tag == "part") {
            size_t partIdx = static_cast<size_t>(e.readInt());
            if (partIdx >= pl.size()) {
                LOGD("Excerpt::read: bad part index");
            } else {
                parts.push_back(pl.at(partIdx));
            }
        }
    }
    item->setParts(parts);
}

void TRead::read(Fermata* f, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(f, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Fermata* f, XmlReader& xml, ReadContext& ctx)
{
    const AsciiStringView tag(xml.name());

    if (tag == "subtype") {
        AsciiStringView s = xml.readAsciiText();
        SymId id = SymNames::symIdByName(s);
        f->setSymIdAndTimeStretch(id);
    } else if (tag == "play") {
        f->setPlay(xml.readBool());
    } else if (tag == "timeStretch") {
        f->setTimeStretch(xml.readDouble());
    } else if (tag == "offset") {
        if (f->score()->mscVersion() > 114) {
            readItemProperties(f, xml, ctx);
        } else {
            xml.skipCurrentElement();       // ignore manual layout in older scores
        }
    } else if (readItemProperties(f, xml, ctx)) {
    } else {
        return false;
    }
    return true;
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
            TRead::readProperty(img, e, ctx, Pid::AUTOSCALE);
        } else if (tag == "size") {
            TRead::readProperty(img, e, ctx, Pid::SIZE);
        } else if (tag == "lockAspectRatio") {
            TRead::readProperty(img, e, ctx, Pid::LOCK_ASPECT_RATIO);
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
        } else if (!TRead::readProperties(img, e, ctx)) {
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

        if (TRead::readStyledProperty(t, tag, e, ctx)) {
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
            t->setUserPoint1(e.readPoint() * t->style().spatium());
        } else if (tag == "p2") {
            t->setUserPoint2(e.readPoint() * t->style().spatium());
        } else if (tag == "baseNote") {
            baseLen = TDuration(TConv::fromXml(e.readAsciiText(), DurationType::V_INVALID));
        } else if (tag == "baseDots") {
            baseLen.setDots(e.readInt());
        } else if (tag == "Number") {
            number = Factory::createText(t, TextStyleType::TUPLET);
            number->setComposition(true);
            number->setParent(t);
            Tuplet::resetNumberProperty(number);
            TRead::read(number, e, ctx);
            number->setVisible(t->visible());         //?? override saved property
            number->setColor(t->color());
            number->setTrack(t->track());
            // move property flags from _number back to tuplet
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN }) {
                t->setPropertyFlags(p, number->propertyFlags(p));
            }
        } else if (!readItemProperties(t, e, ctx)) {
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
            TRead::readProperty(b, e, ctx, Pid::STEM_DIRECTION);
        } else if (tag == "distribute") {
            e.skipCurrentElement(); // obsolete
        } else if (TRead::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "growLeft") {
            b->setGrowLeft(e.readDouble());
        } else if (tag == "growRight") {
            b->setGrowRight(e.readDouble());
        } else if (tag == "Fragment") {
            BeamFragment* f = new BeamFragment;
            int idx = (b->direction() == DirectionV::AUTO || b->direction() == DirectionV::DOWN) ? 0 : 1;
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
        } else if (!readItemProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Ambitus* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(a, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Ambitus* a, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "head") {
        TRead::readProperty(a, e, ctx, Pid::HEAD_GROUP);
    } else if (tag == "headType") {
        TRead::readProperty(a, e, ctx, Pid::HEAD_TYPE);
    } else if (tag == "mirror") {
        TRead::readProperty(a, e, ctx, Pid::MIRROR_HEAD);
    } else if (tag == "hasLine") {
        a->setHasLine(e.readInt());
    } else if (tag == "lineWidth") {
        TRead::readProperty(a, e, ctx, Pid::LINE_WIDTH);
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
                    read206::Read206::readAccidental206(a->topAccidental(), e, ctx);
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
                    read206::Read206::readAccidental206(a->bottomAccidental(), e, ctx);
                } else {
                    TRead::read(a->bottomAccidental(), e, ctx);
                }
            } else {
                e.skipCurrentElement();
            }
        }
    } else if (readItemProperties(a, e, ctx)) {
    } else {
        return false;
    }
    return true;
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
        } else if (readItemProperties(a, e, ctx)) {
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
        } else if (!readProperties(static_cast<TextBase*>(m), e, ctx)) {
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
        } else if (!readProperties(static_cast<TextBase*>(j), e, ctx)) {
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
    while (xml.readNextStartElement()) {
        if (!readProperties(b, xml, ctx)) {
            xml.unknown();
        }
    }
}

bool TRead::readProperties(MeasureNumberBase* n, XmlReader& xml, ReadContext& ctx)
{
    if (TRead::readProperty(n, xml.name(), xml, ctx, Pid::HPLACEMENT)) {
        return true;
    } else {
        return readProperties(static_cast<TextBase*>(n), xml, ctx);
    }
}

void TRead::read(MMRestRange* r, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<MeasureNumberBase*>(r), xml, ctx);
}

void TRead::read(SystemDivider* d, XmlReader& e, ReadContext& ctx)
{
    if (e.attribute("type") == "left") {
        d->setDividerType(SystemDivider::Type::LEFT);
    } else {
        d->setDividerType(SystemDivider::Type::RIGHT);
    }
    TRead::read(static_cast<Symbol*>(d), e, ctx);
}

void TRead::read(ActionIcon* i, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "action") {
            i->setAction(e.readText().toStdString(), 0);
        } else if (tag == "subtype") {
            i->setActionType(static_cast<ActionIconType>(e.readInt()));
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Arpeggio* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            a->setArpeggioType(TConv::fromXml(e.readAsciiText(), ArpeggioType::NORMAL));
        } else if (tag == "userLen1") {
            a->setUserLen1(e.readDouble() * a->spatium());
        } else if (tag == "userLen2") {
            a->setUserLen2(e.readDouble() * a->spatium());
        } else if (tag == "span") {
            // Span now refers to number of voices spanned, not staves
            a->setSpan((e.readInt() - 1) * VOICES + 1);
        } else if (tag == "play") {
            a->setPlayArpeggio(e.readBool());
        } else if (tag == "timeStretch") {
            a->setStretch(e.readDouble());
        } else if (!readItemProperties(a, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Ornament* o, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        if (!readProperties(o, xml, ctx)) {
            xml.unknown();
        }
    }
}

bool TRead::readProperties(Ornament* o, XmlReader& xml, ReadContext& ctx)
{
    const AsciiStringView tag(xml.name());
    if (readProperty(o, tag, xml, ctx, Pid::INTERVAL_ABOVE)) {
    } else if (readProperty(o, tag, xml, ctx, Pid::INTERVAL_BELOW)) {
    } else if (readProperty(o, tag, xml, ctx, Pid::ORNAMENT_SHOW_ACCIDENTAL)) {
    } else if (readProperty(o, tag, xml, ctx, Pid::START_ON_UPPER_NOTE)) {
    } else if (readProperties(static_cast<Articulation*>(o), xml, ctx)) {
    } else if (tag == "Accidental") {
        Accidental* accidental = Factory::createAccidental(o);
        TRead::read(accidental, xml, ctx);
        accidental->setParent(o);
        accidental->placement() == PlacementV::ABOVE ? o->setAccidentalAbove(accidental) : o->setAccidentalBelow(accidental);
    } else if (tag == "Chord") {
        Chord* chord = Factory::createChord(ctx.score()->dummy()->segment());
        TRead::read(chord, xml, ctx);
        chord->setTrack(ctx.track());
        o->setCueNoteChord(chord);
        o->setNoteAbove(chord->notes().front());
    } else {
        return false;
    }
    return true;
}

void TRead::read(Articulation* a, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        if (!readProperties(a, xml, ctx)) {
            xml.unknown();
        }
    }
}

bool TRead::readProperties(Articulation* a, XmlReader& xml, ReadContext& ctx)
{
    const AsciiStringView tag(xml.name());

    if (tag == "subtype") {
        AsciiStringView s = xml.readAsciiText();
        ArticulationTextType textType = TConv::fromXml(s, ArticulationTextType::NO_TEXT);
        if (textType != ArticulationTextType::NO_TEXT) {
            a->setTextType(textType);
        } else {
            SymId id = SymNames::symIdByName(s);
            if (id == SymId::noSym) {
                id = read206::Read206::articulationNames2SymId206(s); // compatibility hack for "old" 3.0 scores
            }
            if (id == SymId::noSym || s == "ornamentMordentInverted") {   // SMuFL < 1.30
                id = SymId::ornamentMordent;
            }

            //! TODO Should be replaced with `ctx.mscoreVersion()`
            //! But at the moment, `ctx` is not set everywhere
            String programVersion = a->score()->mscoreVersion();
            if (!programVersion.isEmpty() && programVersion < u"3.6") {
                if (id == SymId::noSym || s == "ornamentMordent") {   // SMuFL < 1.30 and MuseScore < 3.6
                    id = SymId::ornamentShortTrill;
                }
            }
            a->setSymId(id);
        }
    } else if (tag == "channel") {
        a->setChannelName(xml.attribute("name"));
        xml.readNext();
    } else if (tag == "anchor") {
        if (ctx.mscVersion() <= 400) {
            int v = xml.readInt();
            ArticulationAnchor aa = compat::CompatUtils::translateToNewArticulationAnchor(v);
            a->setAnchor(aa);
            if (a->isStyled(Pid::ARTICULATION_ANCHOR)) {
                a->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
            }
        } else {
            TRead::readProperty(a, tag, xml, ctx, Pid::ARTICULATION_ANCHOR);
        }
    } else if (tag == "direction") {
        TRead::readProperty(a, xml, ctx, Pid::DIRECTION);
    } else if (tag == "ornamentStyle") {
        TRead::readProperty(a, xml, ctx, Pid::ORNAMENT_STYLE);
    } else if (tag == "play") {
        a->setPlayArticulation(xml.readBool());
    } else if (tag == "offset") {
        if (a->score()->mscVersion() >= 400) {
            readItemProperties(a, xml, ctx);
        } else {
            xml.skipCurrentElement();       // ignore manual layout in older scores
        }
    } else if (readItemProperties(a, xml, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(Audio* a, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        if (e.name() == "path") {
            a->setPath(e.readText());
        } else {
            e.unknown();
        }
    }
}

void TRead::read(BagpipeEmbellishment* b, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            b->setEmbelType(TConv::fromXml(e.readAsciiText(), EmbellishmentType(0)));
        } else {
            e.unknown();
        }
    }
}

void TRead::read(BarLine* b, XmlReader& e, ReadContext& ctx)
{
    b->resetProperty(Pid::BARLINE_SPAN);
    b->resetProperty(Pid::BARLINE_SPAN_FROM);
    b->resetProperty(Pid::BARLINE_SPAN_TO);

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            b->setBarLineType(TConv::fromXml(e.readAsciiText(), BarLineType::NORMAL));
        } else if (tag == "span") {
            b->setSpanStaff(e.readBool());
        } else if (tag == "spanFromOffset") {
            b->setSpanFrom(e.readInt());
        } else if (tag == "spanToOffset") {
            b->setSpanTo(e.readInt());
        } else if (tag == "Articulation") {
            Articulation* a = Factory::createArticulation(b->score()->dummy()->chord());
            TRead::read(a, e, ctx);
            b->add(a);
        } else if (tag == "Symbol") {
            Symbol* s = Factory::createSymbol(b);
            s->setTrack(b->track());
            TRead::read(s, e, ctx);
            b->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = Factory::createImage(b);
                image->setTrack(b->track());
                TRead::read(image, e, ctx);
                b->add(image);
            }
        } else if (!readItemProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Bend* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (TRead::readStyledProperty(b, tag, e, ctx)) {
        } else if (tag == "point") {
            PitchValue pv;
            pv.time    = e.intAttribute("time");
            pv.pitch   = e.intAttribute("pitch");
            pv.vibrato = e.intAttribute("vibrato");
            b->addPoint(pv);
            e.readNext();
        } else if (tag == "play") {
            b->setPlayBend(e.readBool());
        } else if (!readItemProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Box* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(b, e, ctx)) {
            e.unknown();
        }
    }

    if (b->score()->mscVersion() < 302) {
        b->setAutoSizeEnabled(false);    // disable auto-size for older scores by default.
    }
}

void TRead::read(HBox* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
    if (b->score()->mscVersion() < 302) {
        b->setAutoSizeEnabled(false);    // disable auto-size for older scores by default.
    }
}

bool TRead::readProperties(HBox* b, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (readProperty(b, tag, e, ctx, Pid::CREATE_SYSTEM_HEADER)) {
    } else if (readProperties(static_cast<Box*>(b), e, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(VBox* b, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<Box*>(b), xml, ctx);
}

void TRead::read(FBox* b, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<Box*>(b), xml, ctx);
}

void TRead::read(TBox* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Text") {
            TRead::read(b->text(), e, ctx);
        } else if (TRead::readProperties(static_cast<Box*>(b), e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

bool TRead::readBoxProperties(Box* b, XmlReader& e, ReadContext& ctx)
{
    if (b->isHBox()) {
        return readProperties(dynamic_cast<HBox*>(b), e, ctx);
    }
    return readProperties(b, e, ctx);
}

bool TRead::readProperties(Box* b, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "height") {
        b->setBoxHeight(Spatium(e.readDouble()));
    } else if (tag == "width") {
        b->setBoxWidth(Spatium(e.readDouble()));
    } else if (tag == "topGap") {
        double gap = e.readDouble();
        b->setTopGap(Spatium(gap));
        if (b->score()->mscVersion() >= 206) {
            b->setTopGap(Spatium(gap));
        }
        b->setPropertyFlags(Pid::TOP_GAP, PropertyFlags::UNSTYLED);
    } else if (tag == "bottomGap") {
        double gap = e.readDouble();
        b->setBottomGap(Spatium(gap));
        if (b->score()->mscVersion() >= 206) {
            b->setBottomGap(Spatium(gap));
        }
        b->setPropertyFlags(Pid::BOTTOM_GAP, PropertyFlags::UNSTYLED);
    } else if (tag == "leftMargin") {
        b->setLeftMargin(e.readDouble());
    } else if (tag == "rightMargin") {
        b->setRightMargin(e.readDouble());
    } else if (tag == "topMargin") {
        b->setTopMargin(e.readDouble());
    } else if (tag == "bottomMargin") {
        b->setBottomMargin(e.readDouble());
    } else if (tag == "boxAutoSize") {
        b->setAutoSizeEnabled(e.readBool());
    } else if (tag == "Text") {
        Text* t;
        if (b->isTBox()) {
            t = toTBox(b)->text();
            TRead::read(t, e, ctx);
        } else {
            t = Factory::createText(b);
            TRead::read(t, e, ctx);
            if (t->empty()) {
                LOGD("read empty text");
            } else {
                b->add(t);
            }
        }
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(b);
        TRead::read(s, e, ctx);
        b->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(b);
            image->setTrack(ctx.track());
            TRead::read(image, e, ctx);
            b->add(image);
        }
    } else if (tag == "FretDiagram") {
        FretDiagram* f = Factory::createFretDiagram(b->score()->dummy()->segment());
        TRead::read(f, e, ctx);
        //! TODO Looks like a bug.
        //! The FretDiagram parent must be Segment
        //! there is a method: `Segment* segment() const { return toSegment(parent()); }`,
        //! but when we add it to Box, the parent will be rewritten.
        b->add(f);
    } else if (tag == "HBox") {
        // m_parent is used here (rather than system()) because explicit parent isn't set for this object
        // until it is fully read. m_parent is nonetheless valid and using it here matches MU3 behavior.
        // If we do not set the parent of this new box correctly, it will cause a crash on read()
        // because it needs access to the system it is being added to. (c.r. issue #14643)
        if (b->parent() && b->parent()->isSystem()) {
            HBox* hb = new HBox(toSystem(b->parent()));
            TRead::read(hb, e, ctx);
            //! TODO Looks like a bug.
            //! The HBox parent must be System
            //! there is a method: `System* system() const { return (System*)parent(); }`,
            //! but when we add it to Box, the parent will be rewritten.
            b->add(hb);
        }
    } else if (tag == "VBox") {
        if (b->parent() && b->parent()->isSystem()) {
            VBox* vb = new VBox(toSystem(b->parent()));
            TRead::read(vb, e, ctx);
            //! TODO Looks like a bug.
            //! The VBox parent must be System
            //! there is a method: `System* system() const { return (System*)parent(); }`,
            //! but when we add it to Box, the parent will be rewritten.
            b->add(vb);
        }
    } else if (TRead::readProperties(static_cast<MeasureBase*>(b), e, ctx)) {
    } else {
        return false;
    }
    return true;
}

bool TRead::readProperties(MeasureBase* b, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "LayoutBreak") {
        LayoutBreak* lb = Factory::createLayoutBreak(b);
        TRead::read(lb, e, ctx);
        bool doAdd = true;
        switch (lb->layoutBreakType()) {
        case LayoutBreakType::LINE:
            if (b->lineBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::PAGE:
            if (b->pageBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::SECTION:
            if (b->sectionBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::NOBREAK:
            if (b->noBreak()) {
                doAdd = false;
            }
            break;
        }
        if (doAdd) {
            b->add(lb);
            b->cleanupLayoutBreaks(false);
        }
    } else if (tag == "StaffTypeChange") {
        StaffTypeChange* stc = Factory::createStaffTypeChange(b);
        stc->setTrack(ctx.track());
        stc->setParent(b);
        TRead::read(stc, e, ctx);
        b->add(stc);
    } else if (readItemProperties(b, e, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(Bracket* b, XmlReader& e, ReadContext& ctx)
{
    BracketItem* bi = Factory::createBracketItem(b->score()->dummy());
    bi->setBracketType(TConv::fromXml(e.asciiAttribute("type"), BracketType::NORMAL));

    while (e.readNextStartElement()) {
        if (e.name() == "level") {
            bi->setColumn(e.readInt());
        } else if (!readItemProperties(b, e, ctx)) {
            e.unknown();
        }
    }

    b->setBracketItem(bi);
}

void TRead::read(Breath* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {                 // obsolete
            SymId symId = SymId::noSym;
            switch (e.readInt()) {
            case 0:
            case 1:
                symId = SymId::breathMarkComma;
                break;
            case 2:
                symId = SymId::caesuraCurved;
                break;
            case 3:
                symId = SymId::caesura;
                break;
            }
            b->setSymId(symId);
        } else if (tag == "symbol") {
            b->setSymId(SymNames::symIdByName(e.readAsciiText()));
        } else if (tag == "pause") {
            b->setPause(e.readDouble());
        } else if (!readItemProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(BSymbol* sym, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag = e.name();

    if (readItemProperties(sym, e, ctx)) {
        return true;
    } else if (tag == "systemFlag") {
        sym->setSystemFlag(e.readInt());
    } else if (tag == "Symbol") {
        Symbol* s = Factory::createSymbol(sym);
        TRead::read(s, e, ctx);
        sym->add(s);
    } else if (tag == "FSymbol") {
        FSymbol* s = Factory::createFSymbol(sym);
        TRead::read(s, e, ctx);
        sym->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(sym);
            TRead::read(image, e, ctx);
            sym->add(image);
        }
    } else {
        return false;
    }
    return true;
}

void TRead::read(Symbol* sym, XmlReader& e, ReadContext& ctx)
{
    SymId symId = SymId::noSym;
    String fontName;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "name") {
            String val(e.readText());
            symId = SymNames::symIdByName(val);
            if (val != "noSym" && symId == SymId::noSym) {
                // if symbol name not found, fall back to user names
                // TODO: does it make sense? user names are probably localized
                symId = SymNames::symIdByUserName(val);
                if (symId == SymId::noSym) {
                    LOGD("unknown symbol <%s>, falling back to no symbol", muPrintable(val));
                    // set a default symbol, or layout() will crash
                    symId = SymId::noSym;
                }
            }
            sym->setSym(symId);
        } else if (tag == "font") {
            fontName = e.readText();
        } else if (tag == "Symbol") {
            Symbol* s = new Symbol(sym);
            TRead::read(s, e, ctx);
            sym->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(sym);
                TRead::read(image, e, ctx);
                sym->add(image);
            }
        } else if (tag == "small" || tag == "subtype") {    // obsolete
            e.skipCurrentElement();
        } else if (tag == "offset" && sym->score()->mscVersion() < 400 && sym->onTabStaff()
                   && (symId == SymId::noteheadParenthesisLeft || symId == SymId::noteheadParenthesisRight)) {
            // Reset mu3 TAB parentheses offset
            e.skipCurrentElement();
        } else if (!TRead::readProperties(static_cast<BSymbol*>(sym), e, ctx)) {
            e.unknown();
        }
    }

    std::shared_ptr<IEngravingFont> scoreFont = nullptr;
    if (!fontName.empty()) {
        scoreFont = ctx.engravingFonts()->fontByName(fontName.toStdString());
    }

    sym->setPos(PointF());
    sym->setSym(symId, scoreFont);
}

void TRead::read(FSymbol* sym, XmlReader& e, ReadContext& ctx)
{
    Font font = sym->font();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "font") {
            font.setFamily(e.readText(), Font::Type::Unknown);
        } else if (tag == "fontsize") {
            font.setPointSizeF(e.readDouble());
        } else if (tag == "code") {
            sym->setCode(e.readInt());
        } else if (!TRead::readProperties(static_cast<BSymbol*>(sym), e, ctx)) {
            e.unknown();
        }
    }

    sym->setPos(PointF());
    sym->setFont(font);
}

void TRead::read(Chord* ch, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (TRead::readProperties(ch, e, ctx)) {
        } else {
            e.unknown();
        }
    }

    //! TODO Should be replaced with `ctx.mscVersion()`
    //! But at the moment, `ctx` is not set everywhere
    int mscVersion = ch->score()->mscVersion();

    // Reset horizontal offset of grace notes when migrating from before 4.0
    if (ch->isGrace() && mscVersion < 400) {
        ch->rxoffset() = 0;
    }
}

bool TRead::readProperties(Chord* ch, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "Note") {
        Note* note = Factory::createNote(ch);
        // the note needs to know the properties of the track it belongs to
        note->setTrack(ch->track());
        note->setParent(ch);
        TRead::read(note, e, ctx);
        ch->add(note);
    } else if (TRead::readProperties(static_cast<ChordRest*>(ch), e, ctx)) {
    } else if (tag == "Stem") {
        Stem* s = Factory::createStem(ch);
        TRead::read(s, e, ctx);
        ch->add(s);
    } else if (tag == "Hook") {
        Hook* hook = new Hook(ch);
        TRead::read(hook, e, ctx);
        ch->setHook(hook);
        ch->add(hook);
    } else if (tag == "appoggiatura") {
        ch->setNoteType(NoteType::APPOGGIATURA);
        e.readNext();
    } else if (tag == "acciaccatura") {
        ch->setNoteType(NoteType::ACCIACCATURA);
        e.readNext();
    } else if (tag == "grace4") {
        ch->setNoteType(NoteType::GRACE4);
        e.readNext();
    } else if (tag == "grace16") {
        ch->setNoteType(NoteType::GRACE16);
        e.readNext();
    } else if (tag == "grace32") {
        ch->setNoteType(NoteType::GRACE32);
        e.readNext();
    } else if (tag == "grace8after") {
        ch->setNoteType(NoteType::GRACE8_AFTER);
        e.readNext();
    } else if (tag == "grace16after") {
        ch->setNoteType(NoteType::GRACE16_AFTER);
        e.readNext();
    } else if (tag == "grace32after") {
        ch->setNoteType(NoteType::GRACE32_AFTER);
        e.readNext();
    } else if (tag == "StemSlash") {
        StemSlash* ss = Factory::createStemSlash(ch);
        TRead::read(ss, e, ctx);
        ch->add(ss);
    } else if (TRead::readProperty(ch, tag, e, ctx, Pid::STEM_DIRECTION)) {
    } else if (tag == "noStem") {
        ch->setNoStem(e.readInt());
    } else if (tag == "Arpeggio") {
        Arpeggio* arpeggio = Factory::createArpeggio(ch);
        arpeggio->setTrack(ch->track());
        TRead::read(arpeggio, e, ctx);
        arpeggio->setParent(ch);
        ch->setArpeggio(arpeggio);
    } else if (tag == "Tremolo") {
        TremoloCompat tcompat;
        tcompat.parent = ch;
        TRead::read(tcompat, e, ctx);
        if (tcompat.two) {
            tcompat.two->setParent(ch);
            tcompat.two->setDurationType(ch->durationType());
            ch->setTremoloTwoChord(tcompat.two, false);
        } else if (tcompat.single) {
            tcompat.single->setParent(ch);
            tcompat.single->setDurationType(ch->durationType());
            ch->setTremoloSingleChord(tcompat.single);
        } else {
            UNREACHABLE;
        }
    } else if (tag == "tickOffset") {      // obsolete
    } else if (tag == "ChordLine") {
        ChordLine* cl = Factory::createChordLine(ch);
        TRead::read(cl, e, ctx);
        ch->add(cl);
    } else {
        return false;
    }
    return true;
}

bool TRead::readProperties(ChordRest* ch, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    //! TODO Should be replaced with `ctx.mscVersion()`
    //! But at the moment, `ctx` is not set everywhere
    int mscVersion = ch->score()->mscVersion();

    if (tag == "durationType") {
        ch->setDurationType(TConv::fromXml(e.readAsciiText(), DurationType::V_QUARTER));
        if (ch->actualDurationType().type() != DurationType::V_MEASURE) {
            if (mscVersion < 112 && (ch->type() == ElementType::REST)
                &&            // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                ch->ticks().numerator() != 0
                &&            // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                (ch->actualDurationType() == DurationType::V_WHOLE && ch->ticks() <= Fraction(4, 4))) {
                // old pre 2.0 scores: convert
                ch->setDurationType(DurationType::V_MEASURE);
            } else {    // not from old score: set duration fraction from duration type
                ch->setTicks(ch->actualDurationType().fraction());
            }
        } else {
            if (mscVersion <= 114) {
                SigEvent event = ctx.compatTimeSigMap()->timesig(ctx.tick());
                ch->setTicks(event.timesig());
            }
        }
    } else if (tag == "BeamMode") {
        // 400 and previous used begin32/begin64 for beam mode
        // 410 uses begin16/begin32
        auto txt = e.readAsciiText();
        if (txt == "begin64") {
            txt = "begin32";
        } else if (txt == "begin32") {
            txt = "begin16";
        }
        ch->setBeamMode(TConv::fromXml(txt, BeamMode::AUTO));
    } else if (tag == "Articulation") {
        Articulation* atr = Factory::createArticulation(ch);
        atr->setTrack(ch->track());
        TRead::read(atr, e, ctx);
        ch->add(atr);
    } else if (tag == "Ornament") {
        Ornament* ornament = Factory::createOrnament(ch);
        ornament->setTrack(ch->track());
        TRead::read(ornament, e, ctx);
        ch->add(ornament);
    } else if (tag == "leadingSpace" || tag == "trailingSpace") {
        LOGD("ChordRest: %s obsolete", tag.ascii());
        e.skipCurrentElement();
    } else if (tag == "small") {
        ch->setSmall(e.readInt());
    } else if (tag == "duration") {
        ch->setTicks(e.readFraction());
    } else if (tag == "ticklen") {      // obsolete (version < 1.12)
        int mticks = ctx.compatTimeSigMap()->timesig(ctx.tick()).timesig().ticks();
        int i = e.readInt();
        if (i == 0) {
            i = mticks;
        }
        if ((ch->type() == ElementType::REST) && (mticks == i)) {
            ch->setDurationType(DurationType::V_MEASURE);
            ch->setTicks(Fraction::fromTicks(i));
        } else {
            Fraction f = Fraction::fromTicks(i);
            ch->setTicks(f);
            ch->setDurationType(TDuration(f));
        }
    } else if (tag == "dots") {
        ch->setDots(e.readInt());
    } else if (tag == "staffMove") {
        ch->setStaffMove(e.readInt());
        if (ch->vStaffIdx() < ch->part()->staves().front()->idx() || ch->vStaffIdx() > ch->part()->staves().back()->idx()) {
            ch->setStaffMove(0);
        }
    } else if (tag == "Spanner") {
        readSpanner(e, ctx, ch, ch->track());
    } else if (tag == "Lyrics") {
        Lyrics* lyr = Factory::createLyrics(ch);
        lyr->setTrack(ctx.track());
        TRead::read(lyr, e, ctx);
        ch->add(lyr);
    } else if (tag == "pos") {
        PointF pt = e.readPoint();
        ch->setOffset(pt * ch->spatium());
    }
//      else if (tag == "offset")
//            DurationElement::readProperties(e);
    else if (!readItemProperties(ch, e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(ChordLine* l, XmlReader& e, ReadContext& ctx)
{
    //! NOTE Need separated "given" data and layout data
    ChordLine::LayoutData* ldata = l->mutldata();
    ldata->path = PainterPath();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Path") {
            PainterPath path;
            PointF curveTo;
            PointF p1;
            int state = 0;
            while (e.readNextStartElement()) {
                const AsciiStringView nextTag(e.name());
                if (nextTag == "Element") {
                    int type = e.intAttribute("type");
                    double x  = e.doubleAttribute("x");
                    double y  = e.doubleAttribute("y");
                    double spatium = ctx.spatium();
                    x *= spatium;
                    y *= spatium;
                    switch (PainterPath::ElementType(type)) {
                    case PainterPath::ElementType::MoveToElement:
                        path.moveTo(x, y);
                        break;
                    case PainterPath::ElementType::LineToElement:
                        path.lineTo(x, y);
                        break;
                    case PainterPath::ElementType::CurveToElement:
                        curveTo.rx() = x;
                        curveTo.ry() = y;
                        state = 1;
                        break;
                    case PainterPath::ElementType::CurveToDataElement:
                        if (state == 1) {
                            p1.rx() = x;
                            p1.ry() = y;
                            state = 2;
                        } else if (state == 2) {
                            path.cubicTo(curveTo, p1, PointF(x, y));
                            state = 0;
                        }
                        break;
                    }
                    e.skipCurrentElement();           //needed to go to next EngravingItem in Path
                } else {
                    e.unknown();
                }
            }
            ldata->path = path;
            l->setModified(true);
        } else if (tag == "subtype") {
            l->setChordLineType(TConv::fromXml(e.readAsciiText(), ChordLineType::NOTYPE));
        } else if (tag == "straight") {
            l->setStraight(e.readInt());
        } else if (tag == "wavy") {
            l->setWavy(e.readInt());
        } else if (tag == "lengthX") {
            l->setLengthX(e.readInt());
        } else if (tag == "lengthY") {
            l->setLengthY(e.readInt());
        } else if (tag == "offset" && l->score()->mscVersion() < 400) { // default positions has changed in 4.0 so ignore previous offset
            e.skipCurrentElement();
        } else if (!readItemProperties(l, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Clef* c, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "concertClefType") {
            c->setConcertClef(TConv::fromXml(e.readAsciiText(), ClefType::G));
        } else if (tag == "transposingClefType") {
            c->setTransposingClef(TConv::fromXml(e.readAsciiText(), ClefType::G));
        } else if (tag == "showCourtesyClef") {
            c->setShowCourtesy(e.readInt());
        } else if (tag == "forInstrumentChange") {
            c->setForInstrumentChange(e.readBool());
        } else if (tag == "clefToBarlinePos") {
            c->setClefToBarlinePosition(ClefToBarlinePosition(e.readInt()));
        } else if (tag == "isHeader") {
            c->setIsHeader(e.readBool());
        } else if (!readItemProperties(c, e, ctx)) {
            e.unknown();
        }
    }

    if (c->clefType() == ClefType::INVALID) {
        c->setClefType(ClefType::G);
    }
}

bool TRead::readProperties(Clef* c, XmlReader& xml, ReadContext& ctx)
{
    return readItemProperties(c, xml, ctx);
}

void TRead::read(Fingering* f, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(static_cast<TextBase*>(f), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Glissando* g, XmlReader& e, ReadContext& ctx)
{
    g->eraseSpannerSegments();

    if (g->score()->mscVersion() < 301) {
        ctx.addSpanner(e.intAttribute("id", -1), g);
    }

    staff_idx_t staffIdx = track2staff(ctx.track());
    Staff* staff = ctx.score()->staff(staffIdx);
    if (staff) {
        String instrId = staff->part()->instrumentId(g->tick());
        g->setIsHarpGliss(instrId == "harp");
    }
    g->resetProperty(Pid::GLISS_STYLE);
    g->setShowText(false);
    while (e.readNextStartElement()) {
        const AsciiStringView tag = e.name();
        if (tag == "text") {
            g->setShowText(true);
            TRead::readProperty(g, e, ctx, Pid::GLISS_TEXT);
        } else if (tag == "subtype") {
            g->setGlissandoType(TConv::fromXml(e.readAsciiText(), GlissandoType::STRAIGHT));
        } else if (tag == "glissandoStyle") {
            TRead::readProperty(g, e, ctx, Pid::GLISS_STYLE);
        } else if (tag == "easeInSpin") {
            g->setEaseIn(e.readInt());
        } else if (tag == "easeOutSpin") {
            g->setEaseOut(e.readInt());
        } else if (TRead::readStyledProperty(g, tag, e, ctx)) {
        } else if (!TRead::readProperties(static_cast<SLine*>(g), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(GradualTempoChange* c, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        const AsciiStringView tag(xml.name());

        if (TRead::readProperty(c, tag, xml, ctx, Pid::LINE_WIDTH)) {
            c->setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
            continue;
        }

        if (TRead::readProperty(c, tag, xml, ctx, Pid::TEMPO_CHANGE_TYPE)) {
            continue;
        }

        if (TRead::readProperty(c, tag, xml, ctx, Pid::TEMPO_EASING_METHOD)) {
            continue;
        }

        if (TRead::readProperty(c, tag, xml, ctx, Pid::TEMPO_CHANGE_FACTOR)) {
            continue;
        }

        if (!readProperties(static_cast<TextLineBase*>(c), xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(Groups* g, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Node") {
            GroupNode n;
            n.pos    = e.intAttribute("pos");
            n.action = e.intAttribute("action");
            g->addNode(n);
            e.skipCurrentElement();
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Hairpin* h, XmlReader& e, ReadContext& ctx)
{
    h->eraseSpannerSegments();

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            h->setHairpinType(HairpinType(e.readInt()));
        } else if (TRead::readStyledProperty(h, tag, e, ctx)) {
        } else if (tag == "hairpinCircledTip") {
            h->setHairpinCircledTip(e.readInt());
        } else if (tag == "veloChange") {
            h->setVeloChange(e.readInt());
        } else if (tag == "dynType") {
            h->setDynRange(TConv::fromXml(e.readAsciiText(), DynamicRange::PART));
        } else if (tag == "useTextLine") {        // obsolete
            e.readInt();
            if (h->hairpinType() == HairpinType::CRESC_HAIRPIN) {
                h->setHairpinType(HairpinType::CRESC_LINE);
            } else if (h->hairpinType() == HairpinType::DECRESC_HAIRPIN) {
                h->setHairpinType(HairpinType::DECRESC_LINE);
            }
        } else if (tag == "singleNoteDynamics") {
            h->setSingleNoteDynamics(e.readBool());
        } else if (tag == "veloChangeMethod") {
            h->setVeloChangeMethod(TConv::fromXml(e.readAsciiText(), ChangeMethod::NORMAL));
        } else if (!readProperties(static_cast<TextLineBase*>(h), e, ctx)) {
            e.unknown();
        }
    }

    h->styleChanged();
}

void TRead::read(Harmony* h, XmlReader& e, ReadContext& ctx)
{
    HarmonyInfo* info = new HarmonyInfo(ctx.score());
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "base") {
            info->m_bassTpc = e.readInt();
        } else if (tag == "baseCase") {
            h->setBassCase(static_cast<NoteCaseType>(e.readInt()));
        } else if (tag == "extension") {
            info->m_id = e.readInt();
        } else if (tag == "name") {
            info->m_textName = e.readText();
        } else if (tag == "root") {
            info->m_rootTpc = e.readInt();
        } else if (tag == "rootCase") {
            h->setRootCase(static_cast<NoteCaseType>(e.readInt()));
        } else if (tag == "function") {
            h->setTpcFromFunction(e.readText());
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
        } else if (TRead::readProperty(h, tag, e, ctx, Pid::POS_ABOVE)) {
        } else if (TRead::readProperty(h, tag, e, ctx, Pid::HARMONY_TYPE)) {
        } else if (TRead::readProperty(h, tag, e, ctx, Pid::PLAY)) {
        } else if (TRead::readProperty(h, tag, e, ctx, Pid::HARMONY_VOICE_LITERAL)) {
        } else if (TRead::readProperty(h, tag, e, ctx, Pid::HARMONY_VOICING)) {
        } else if (TRead::readProperty(h, tag, e, ctx, Pid::HARMONY_DURATION)) {
        } else if (!readProperties(static_cast<TextBase*>(h), e, ctx)) {
            e.unknown();
        }
    }

    h->addChord(info);

    h->afterRead();
}

void TRead::read(HarmonicMark* h, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<TextLineBase*>(h), xml, ctx);
}

void TRead::read(HarpPedalDiagram* h, XmlReader& xml, ReadContext& ctx)
{
    while (xml.readNextStartElement()) {
        const AsciiStringView tag = xml.name();
        if (tag == "isDiagram") {
            h->setIsDiagram(xml.readBool());
        } else if (tag == "pedalState") {
            while (xml.readNextStartElement()) {
                const AsciiStringView stringTag = xml.name();
                if (stringTag == "string") {
                    HarpStringType str = HarpStringType(xml.intAttribute("name"));
                    PedalPosition pos = PedalPosition(xml.readInt());
                    h->setPedal(str, pos);
                } else {
                    xml.unknown();
                }
            }
            h->setPlayableTpcs();
        } else if (!readProperties(h, xml, ctx)) {
            xml.unknown();
        }
    }
}

void TRead::read(Hook* h, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<Symbol*>(h), xml, ctx);
}

void TRead::read(LayoutBreak* b, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            TRead::readProperty(b, e, ctx, Pid::LAYOUT_BREAK);
        } else if (tag == "pause") {
            TRead::readProperty(b, e, ctx, Pid::PAUSE);
        } else if (tag == "startWithLongNames") {
            TRead::readProperty(b, e, ctx, Pid::START_WITH_LONG_NAMES);
        } else if (tag == "startWithMeasureOne") {
            TRead::readProperty(b, e, ctx, Pid::START_WITH_MEASURE_ONE);
        } else if (tag == "firstSystemIndentation") {
            TRead::readProperty(b, e, ctx, Pid::FIRST_SYSTEM_INDENTATION);
        } else if (!readItemProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(LedgerLine* l, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(l, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(LedgerLine* l, XmlReader& e, ReadContext&)
{
    const AsciiStringView tag(e.name());

    if (tag == "lineWidth") {
        //! NOTE Probably need to be removed, because it is calculated in Layout
        LedgerLine::LayoutData* ldata = l->mutldata();
        ldata->lineWidth = (e.readDouble() * l->spatium());
    } else if (tag == "lineLen") {
        l->setLen(e.readDouble() * l->spatium());
    } else if (tag == "vertical") {
        l->setVertical(e.readInt());
    } else {
        return false;
    }
    return true;
}

void TRead::read(LetRing* r, XmlReader& e, ReadContext& ctx)
{
    if (r->score()->mscVersion() < 301) {
        ctx.addSpanner(e.intAttribute("id", -1), r);
    }
    while (e.readNextStartElement()) {
        if (TRead::readProperty(r, e.name(), e, ctx, Pid::LINE_WIDTH)) {
            r->setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
        } else if (!readProperties(static_cast<TextLineBase*>(r), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Location* l, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "staves") {
            l->setStaff(e.readInt());
        } else if (tag == "voices") {
            l->setVoice(e.readInt());
        } else if (tag == "measures") {
            l->setMeasure(e.readInt());
        } else if (tag == "fractions") {
            l->setFrac(e.readFraction());
        } else if (tag == "grace") {
            l->setGraceIndex(e.readInt());
        } else if (tag == "notes") {
            l->setNote(e.readInt());
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Lyrics* l, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!TRead::readProperties(l, e, ctx)) {
            e.unknown();
        }
    }
    if (!l->isStyled(Pid::OFFSET) && !ctx.pasteMode()) {
        // fix offset for pre-3.1 scores
        // 3.0: y offset was meaningless if autoplace is set
        String version = ctx.mscoreVersion();
        if (l->autoplace() && !version.isEmpty() && version < u"3.1") {
            PointF off = l->propertyDefault(Pid::OFFSET).value<PointF>();
            l->ryoffset() = off.y();
        }
    }
}

void TRead::read(LineSegment* l, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(l, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(LineSegment* l, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "subtype") {
        l->setSpannerSegmentType(SpannerSegmentType(e.readInt()));
    } else if (tag == "off2") {
        l->setUserOff2(e.readPoint() * l->style().spatium());
    } else if (!readItemProperties(l, e, ctx)) {
        return false;
    }
    return true;
}

bool TRead::readProperties(Lyrics* l, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "no") {
        l->setNo(e.readInt());
        if (l->isEven()) {
            l->initTextStyleType(TextStyleType::LYRICS_EVEN);
        }
    } else if (tag == "syllabic") {
        l->setSyllabic(TConv::fromXml(e.readAsciiText(), LyricsSyllabic::SINGLE));
    } else if (tag == "ticks") {          // obsolete
        l->setTicks(e.readFraction());     // will fall back to reading integer ticks on older scores
    } else if (tag == "ticks_f") {
        l->setTicks(e.readFraction());
    } else if (TRead::readProperty(l, tag, e, ctx, Pid::PLACEMENT)) {
    } else if (!readProperties(static_cast<TextBase*>(l), e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(MeasureRepeat* r, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            r->setNumMeasures(e.readInt());
        } else if (!readProperties(static_cast<Rest*>(r), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(MMRest* r, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Symbol") {
            Symbol* s = new Symbol(r);
            s->setTrack(r->track());
            TRead::read(s, e, ctx);
            r->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(r);
                image->setTrack(r->track());
                TRead::read(image, e, ctx);
                r->add(image);
            }
        } else if (tag == "NoteDot") {
            NoteDot* dot = Factory::createNoteDot(r);
            TRead::read(dot, e, ctx);
            r->add(dot);
        } else if (tag == "mmRestNumberPos") {
            // Old property, deprecated in 4.5
            r->setNumberOffset(e.readDouble() - ctx.style().styleS(Sid::mmRestNumberPos).val());
        } else if (TRead::readStyledProperty(r, tag, e, ctx)) {
        } else if (readProperties(r, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

bool TRead::readProperties(MMRest* r, XmlReader& xml, ReadContext& ctx)
{
    const AsciiStringView tag(xml.name());
    if (tag == "mmRestNumberVisible") {
        r->setProperty(Pid::MMREST_NUMBER_VISIBLE, xml.readBool());
    } else if (TRead::readProperties(static_cast<Rest*>(r), xml, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(Note* n, XmlReader& e, ReadContext& ctx)
{
    n->setTpc1(Tpc::TPC_INVALID);
    n->setTpc2(Tpc::TPC_INVALID);

    while (e.readNextStartElement()) {
        if (TRead::readProperties(n, e, ctx)) {
        } else {
            e.unknown();
        }
    }

    n->setupAfterRead(ctx.tick(), ctx.pasteMode());
}

bool TRead::readProperties(Note* n, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "pitch") {
        n->setPitch(std::clamp(e.readInt(), 0, 127), false);
    } else if (tag == "tpc") {
        int tcp = e.readInt();
        n->setTpc1(tcp);
        n->setTpc2(tcp);
    } else if (tag == "track") {          // for performance
        n->setTrack(e.readInt());
    } else if (tag == "Accidental") {
        Accidental* a = Factory::createAccidental(n);
        a->setTrack(n->track());
        TRead::read(a, e, ctx);
        n->add(a);
    } else if (tag == "Spanner") {
        readSpanner(e, ctx, n, n->track());
    } else if (tag == "tpc2") {
        n->setTpc2(e.readInt());
    } else if (tag == "small") {
        n->setSmall(e.readInt());
    } else if (tag == "mirror") {
        TRead::readProperty(n, e, ctx, Pid::MIRROR_HEAD);
    } else if (tag == "dotPosition") {
        TRead::readProperty(n, e, ctx, Pid::DOT_POSITION);
    } else if (tag == "fixed") {
        n->setFixed(e.readBool());
    } else if (tag == "fixedLine") {
        n->setFixedLine(e.readInt());
    } else if (tag == "headScheme") {
        TRead::readProperty(n, e, ctx, Pid::HEAD_SCHEME);
    } else if (tag == "head") {
        TRead::readProperty(n, e, ctx, Pid::HEAD_GROUP);
    } else if (tag == "velocity") {
        if (n->score()->mscVersion() >= 400) {
            n->setUserVelocity(e.readInt());
        } else {
            // TODO: convert pre-MU4 velocity values to MU4
            // but doing so is non-trivial,
            // since MU3 "offset" is based on a percentage of the current dynamic (which is not known at this time)
            // and MU4 "user" is not raw MIDI velocity like MU3 is
            // so until a new velocity system is designed for MU4 (or another solution is found),
            // the best we can do is ignore pre-MU4 velocity values
            e.skipCurrentElement();
        }
    } else if (tag == "play") {
        n->setPlay(e.readInt());
    } else if (tag == "tuning") {
        n->setTuning(e.readDouble());
    } else if (tag == "fret") {
        n->setFret(e.readInt());
    } else if (tag == "string") {
        n->setString(e.readInt());
    } else if (tag == "ghost") {
        if (n->score()->mscVersion() < 400) {
            n->setDeadNote(e.readInt());
        } else {
            n->setGhost(e.readInt());
        }
    } else if (tag == "dead") {
        n->setDeadNote(e.readInt());
    } else if (tag == "headType") {
        TRead::readProperty(n, e, ctx, Pid::HEAD_TYPE);
    } else if (tag == "veloType") {
        TRead::readProperty(n, e, ctx, Pid::VELO_TYPE);
    } else if (tag == "line") {
        n->setLine(e.readInt());
    } else if (tag == "Fingering") {
        Fingering* f = Factory::createFingering(n);
        f->setTrack(n->track());
        TRead::read(f, e, ctx);
        n->add(f);
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(n);
        s->setTrack(n->track());
        TRead::read(s, e, ctx);
        n->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(n);
            image->setTrack(n->track());
            TRead::read(image, e, ctx);
            n->add(image);
        }
    } else if (tag == "Bend") {
        Bend* b = Factory::createBend(n);
        b->setTrack(n->track());
        TRead::read(b, e, ctx);
        n->add(b);
    } else if (tag == "NoteDot") {
        NoteDot* dot = Factory::createNoteDot(n);
        TRead::read(dot, e, ctx);
        n->add(dot);
    } else if (tag == "Events") {
        NoteEventList playEvents;
        while (e.readNextStartElement()) {
            const AsciiStringView t(e.name());
            if (t == "Event") {
                NoteEvent ne;
                read400::TRead::read(&ne, e, ctx);
                playEvents.push_back(ne);
            } else {
                e.unknown();
            }
        }
        n->setPlayEvents(playEvents);
        if (n->chord()) {
            n->chord()->setPlayEventType(PlayEventType::User);
        }
    } else if (tag == "offset") {
        readItemProperties(n, e, ctx);
    } else if (tag == "ChordLine" && n->chord()) {
        ChordLine* cl = Factory::createChordLine(n->chord());
        TRead::read(cl, e, ctx);
        cl->setNote(n);
        n->chord()->add(cl);
    } else if (readItemProperties(n, e, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(NoteEvent* item, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "pitch") {
            item->setPitch(e.readInt());
        } else if (tag == "ontime") {
            item->setOntime(e.readInt());
        } else if (tag == "len") {
            item->setLen(e.readInt());
        } else {
            e.unknown();
        }
    }
}

void TRead::read(NoteDot* d, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (e.name() == "name") {      // obsolete
            e.readText();
        } else if (e.name() == "subtype") {     // obsolete
            e.readText();
        } else if (!readItemProperties(d, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(NoteHead* h, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<Symbol*>(h), xml, ctx);
}

void TRead::read(Ottava* o, XmlReader& e, ReadContext& ctx)
{
    o->eraseSpannerSegments();
    if (o->score()->mscVersion() < 301) {
        ctx.addSpanner(e.intAttribute("id", -1), o);
    }
    while (e.readNextStartElement()) {
        readProperties(o, e, ctx);
    }
    if (o->ottavaType() != OttavaType::OTTAVA_8VA || o->numbersOnly() != o->propertyDefault(Pid::NUMBERS_ONLY).toBool()) {
        o->styleChanged();
    }
}

bool TRead::readProperties(Ottava* o, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "subtype") {
        String s = e.readText();
        bool ok;
        int idx = s.toInt(&ok);
        if (!ok) {
            o->setOttavaType(OttavaType::OTTAVA_8VA);
            for (OttavaDefault d : ottavaDefault) {
                if (s == d.name) {
                    o->setOttavaType(d.type);
                    break;
                }
            }
        } else if (o->score()->mscVersion() <= 114) {
            //subtype are now in a different order...
            if (idx == 1) {
                idx = 2;
            } else if (idx == 2) {
                idx = 1;
            }
            o->setOttavaType(OttavaType(idx));
        } else {
            o->setOttavaType(OttavaType(idx));
        }
    } else if (readStyledProperty(o, tag, e, ctx)) {
        return true;
    } else if (!readProperties(static_cast<TextLineBase*>(o), e, ctx)) {
        e.unknown();
        return false;
    }
    return true;
}

void TRead::read(Page* p, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (e.name() == "System") {
            System* system = Factory::createSystem(p->score()->dummy()->page());
            p->score()->systems().push_back(system);
            read(system, e, ctx);
        } else {
            e.unknown();
        }
    }
}

void TRead::read(PalmMute* p, XmlReader& e, ReadContext& ctx)
{
    if (p->score()->mscVersion() < 301) {
        ctx.addSpanner(e.intAttribute("id", -1), p);
    }
    while (e.readNextStartElement()) {
        if (readProperty(p, e.name(), e, ctx, Pid::LINE_WIDTH)) {
            p->setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
        } else if (!readProperties(static_cast<TextLineBase*>(p), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Part* p, XmlReader& e, ReadContext& ctx)
{
    p->setId(e.intAttribute("id", 0));

    while (e.readNextStartElement()) {
        if (!readProperties(p, e, ctx)) {
            e.unknown();
        }
    }

    if (p->partName().isEmpty()) {
        p->setPartName(p->instrument()->trackName());
    }
}

bool TRead::readProperties(Part* p, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "id") {
        p->setId(e.readInt());
    } else if (tag == "Staff") {
        Staff* staff = Factory::createStaff(p);
        p->score()->appendStaff(staff);
        TRead::read(staff, e, ctx);
    } else if (tag == "Instrument") {
        Instrument* instr = new Instrument;
        read(instr, e, ctx, p);
        p->setInstrument(instr, Fraction(-1, 1));
    } else if (tag == "name") {
        p->instrument()->setLongName(e.readText());
    } else if (tag == "color") {
        p->setColor(e.readInt());
    } else if (tag == "shortName") {
        p->instrument()->setShortName(e.readText());
    } else if (tag == "trackName") {
        p->setPartName(e.readText());
    } else if (tag == "show") {
        p->setShow(e.readInt());
    } else if (tag == "soloist") {
        p->setSoloist(e.readInt());
    } else if (tag == "preferSharpFlat") {
        String val = e.readText();
        if (val == "sharps") {
            p->setPreferSharpFlat(PreferSharpFlat::SHARPS);
        } else if (val == "flats") {
            p->setPreferSharpFlat(PreferSharpFlat::FLATS);
        } else {
            p->setPreferSharpFlat(PreferSharpFlat::AUTO);
        }
    } else {
        return false;
    }
    return true;
}

void TRead::read(Pedal* p, XmlReader& e, ReadContext& ctx)
{
    if (p->score()->mscVersion() < 301) {
        ctx.addSpanner(e.intAttribute("id", -1), p);
    }

    bool beginTextTag = false;
    bool continueTextTag = false;
    bool endTextTag = false;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        beginTextTag = beginTextTag || tag == "beginText";
        continueTextTag = continueTextTag || tag == "continueText";
        endTextTag = endTextTag || tag == "endText";
        if (readStyledProperty(p, tag, e, ctx)) {
        } else if (!readProperties(static_cast<TextLineBase*>(p), e, ctx)) {
            e.unknown();
        }
    }

    // Set to the 400 defaults if no value was specified;
    // or follow the new style setting if the specified value matches it
    if (!beginTextTag) {
        p->setBeginText(String());
        p->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
    } else if (p->beginText() == p->propertyDefault(Pid::BEGIN_TEXT).value<String>()) {
        p->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::STYLED);
    }
    if (!continueTextTag) {
        p->setContinueText(String());
        p->setPropertyFlags(Pid::CONTINUE_TEXT, PropertyFlags::UNSTYLED);
    } else if (p->continueText() == p->propertyDefault(Pid::CONTINUE_TEXT).value<String>()) {
        p->setPropertyFlags(Pid::CONTINUE_TEXT, PropertyFlags::STYLED);
    }
    if (!endTextTag) {
        p->setEndText(String());
        p->setPropertyFlags(Pid::END_TEXT, PropertyFlags::UNSTYLED);
    } else if (p->endText() == p->propertyDefault(Pid::END_TEXT).value<String>()) {
        p->setPropertyFlags(Pid::END_TEXT, PropertyFlags::STYLED);
    }
}

void TRead::read(Rasgueado* r, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<TextLineBase*>(r), xml, ctx);
}

void TRead::read(Rest* r, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Symbol") {
            Segment* seg = toSegment(r->parent());
            Symbol* s = new Symbol(seg);
            s->setTrack(r->track());
            TRead::read(s, e, ctx);
            seg->add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Segment* seg = toSegment(r->parent());
                Image* image = new Image(seg);
                image->setTrack(r->track());
                TRead::read(image, e, ctx);
                seg->add(image);
            }
        } else if (tag == "NoteDot") {
            NoteDot* dot = Factory::createNoteDot(r);
            TRead::read(dot, e, ctx);
            r->add(dot);
        } else if (TRead::readStyledProperty(r, tag, e, ctx)) {
        } else if (TRead::readProperties(r, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Segment* s, XmlReader& e, ReadContext& ctx)
{
    UNUSED(ctx);
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "subtype") {
            e.skipCurrentElement();
        } else if (tag == "leadingSpace") {
            s->setExtraLeadingSpace(Spatium(e.readDouble()));
        } else if (tag == "trailingSpace") {          // obsolete
            e.readDouble();
        } else {
            e.unknown();
        }
    }
}

void TRead::read(SLine* l, XmlReader& e, ReadContext& ctx)
{
    l->eraseSpannerSegments();

    if (l->score()->mscVersion() < 301) {
        ctx.addSpanner(e.intAttribute("id", -1), l);
    }

    while (e.readNextStartElement()) {
        if (!readProperties(l, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(SLine* l, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "tick2") {                  // obsolete
        if (l->tick() == Fraction(-1, 1)) {   // not necessarily set (for first note of score?) #30151
            l->setTick(ctx.tick());
        }
        l->setTick2(Fraction::fromTicks(e.readInt()));
    } else if (tag == "tick") {           // obsolete
        l->setTick(Fraction::fromTicks(e.readInt()));
    } else if (tag == "ticks") {
        l->setTicks(Fraction::fromTicks(e.readInt()));
    } else if (tag == "Segment") {
        LineSegment* ls = l->createLineSegment(l->score()->dummy()->system());
        ls->setTrack(l->track());     // needed in read to get the right staff mag
        TRead::read(ls, e, ctx);
        l->add(ls);
        ls->setVisible(l->visible());
    } else if (tag == "length") {
        l->setLen(e.readDouble());
    } else if (tag == "diagonal") {
        l->setDiagonal(e.readInt());
    } else if (tag == "anchor") {
        l->setAnchor(SLine::Anchor(e.readInt()));
    } else if (tag == "lineWidth") {
        l->setLineWidth(Spatium(e.readDouble()));
    } else if (TRead::readProperty(l, tag, e, ctx, Pid::LINE_STYLE)) {
    } else if (tag == "dashLineLength") {
        l->setDashLineLen(e.readDouble());
    } else if (tag == "dashGapLength") {
        l->setDashGapLen(e.readDouble());
    } else if (tag == "lineColor") {
        l->setLineColor(e.readColor());
    } else if (tag == "color") {
        l->setLineColor(e.readColor());
    } else if (!readProperties(static_cast<Spanner*>(l), e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(Slur* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Slur* s, XmlReader& e, ReadContext& ctx)
{
    return readProperties(static_cast<SlurTie*>(s), e, ctx);
}

void TRead::read(SlurTie* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(SlurTie* s, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (TRead::readProperty(s, tag, e, ctx, Pid::SLUR_DIRECTION)) {
    } else if (tag == "lineType") {
        s->setStyleType(static_cast<SlurStyleType>(e.readInt()));
    } else if (tag == "SlurSegment" || tag == "TieSegment") {
        const int idx = e.intAttribute("no", 0);
        const int n = int(s->spannerSegments().size());
        for (int i = n; i < idx; ++i) {
            s->add(s->newSlurTieSegment(s->score()->dummy()->system()));
        }
        SlurTieSegment* sts = s->newSlurTieSegment(s->score()->dummy()->system());
        TRead::read(sts, e, ctx);
        s->add(sts);
    } else if (!readProperties(static_cast<Spanner*>(s), e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(SlurTieSegment* s, XmlReader& e, ReadContext& ctx)
{
    double _spatium = s->style().spatium();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (s->score()->mscVersion() < 400 && (tag == "o1" || tag == "o2" || tag == "o3" || tag == "o4")) {
            e.skipCurrentElement(); // Ignore slur user offsets from pre-4.0
        } else if (tag == "o1") {
            s->ups(Grip::START).off = e.readPoint() * _spatium;
        } else if (tag == "o2") {
            s->ups(Grip::BEZIER1).off = e.readPoint() * _spatium;
        } else if (tag == "o3") {
            s->ups(Grip::BEZIER2).off = e.readPoint() * _spatium;
        } else if (tag == "o4") {
            s->ups(Grip::END).off = e.readPoint() * _spatium;
        } else if (!readItemProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Spanner* s, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (ctx.pasteMode() && (tag == "ticks_f")) {
        s->setTicks(e.readFraction());
        return true;
    } else if (tag == "play") {
        s->setPlaySpanner(e.readBool());
        return true;
    } else if (!readItemProperties(s, e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(Spacer* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            s->setSpacerType(SpacerType(e.readInt()));
        } else if (tag == "space") {
            s->setGap(Spatium(e.readDouble()));
        } else if (!readItemProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(StaffType* t, XmlReader& e, ReadContext&)
{
    t->setGroup(TConv::fromXml(e.asciiAttribute("group"), StaffGroup::STANDARD));

    if (t->group() == StaffGroup::TAB) {
        t->setGenKeysig(false);
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "name") {
            t->setXmlName(e.readText());
        } else if (tag == "lines") {
            t->setLines(e.readInt());
        } else if (tag == "lineDistance") {
            t->setLineDistance(Spatium(e.readDouble()));
        } else if (tag == "yoffset") {
            t->setYoffset(Spatium(e.readDouble()));
        } else if (tag == "mag") {
            t->setUserMag(e.readDouble());
        } else if (tag == "small") {
            t->setSmall(e.readBool());
        } else if (tag == "stepOffset") {
            t->setStepOffset(e.readInt());
        } else if (tag == "clef") {
            t->setGenClef(e.readInt());
        } else if ((tag == "slashStyle") || (tag == "stemless")) {
            bool val = e.readInt() != 0;
            t->setStemless(val);
            t->setShowBackTied(!val);        // for compatibility with 2.0.2 scores where this prop
        }                                 // was lacking and controlled by "slashStyle" instead
        else if (tag == "barlines") {
            t->setShowBarlines(e.readInt());
        } else if (tag == "timesig") {
            t->setGenTimesig(e.readInt());
        } else if (tag == "noteheadScheme") {
            t->setNoteHeadScheme(TConv::fromXml(e.readAsciiText(), NoteHeadScheme::HEAD_NORMAL));
        } else if (tag == "keysig") {
            t->setGenKeysig(e.readInt());
        } else if (tag == "ledgerlines") {
            t->setShowLedgerLines(e.readInt());
        } else if (tag == "invisible") {
            t->setInvisible(e.readInt());
        } else if (tag == "color") {
            t->setColor(e.readColor());
        } else if (tag == "durations") {
            t->setGenDurations(e.readBool());
        } else if (tag == "durationFontName") {
            t->setDurationFontName(e.readText());
        } else if (tag == "durationFontSize") {
            t->setDurationFontSize(e.readDouble());
        } else if (tag == "durationFontY") {
            t->setDurationFontUserY(e.readDouble());
        } else if (tag == "fretFontName") {
            t->setFretFontName(e.readText());
        } else if (tag == "fretFontSize") {
            t->setFretFontSize(e.readDouble());
        } else if (tag == "fretFontY") {
            t->setFretFontUserY(e.readDouble());
        } else if (tag == "symbolRepeat") {
            t->setSymbolRepeat((TablatureSymbolRepeat)e.readInt());
        } else if (tag == "linesThrough") {
            t->setLinesThrough(e.readBool());
        } else if (tag == "minimStyle") {
            t->setMinimStyle((TablatureMinimStyle)e.readInt());
        } else if (tag == "onLines") {
            t->setOnLines(e.readBool());
        } else if (tag == "showRests") {
            t->setShowRests(e.readBool());
        } else if (tag == "stemsDown") {
            t->setStemsDown(e.readBool());
        } else if (tag == "stemsThrough") {
            t->setStemsThrough(e.readBool());
        } else if (tag == "upsideDown") {
            t->setUpsideDown(e.readBool());
        } else if (tag == "showTabFingering") {
            t->setShowTabFingering(e.readBool());
        } else if (tag == "useNumbers") {
            t->setUseNumbers(e.readBool());
        } else if (tag == "showBackTied") {           // must be after reading "slashStyle"/"stemless" prop, as in older
            t->setShowBackTied(e.readBool());            // scores, this prop was lacking and controlled by "slashStyle"
        } else {
            e.unknown();
        }
    }
}

void TRead::read(StaffTypeChange* c, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "StaffType") {
            StaffType* st = new StaffType();
            TRead::read(st, e, ctx);
            // Measure::add() will replace this with a pointer to a copy in the staff
            c->setStaffType(st, true);
        } else if (!readItemProperties(c, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Staff* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Staff* s, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    if (tag == "StaffType") {
        StaffType st;
        TRead::read(&st, e, ctx);
        s->setStaffType(Fraction(0, 1), st);
    } else if (tag == "defaultClef") {           // sets both default transposing and concert clef
        ClefType ct = TConv::fromXml(e.readAsciiText(), ClefType::G);
        s->setDefaultClefType(ClefTypeList(ct, ct));
    } else if (tag == "defaultConcertClef") {
        s->setDefaultClefType(ClefTypeList(TConv::fromXml(e.readAsciiText(), ClefType::G), s->defaultClefType().transposingClef));
    } else if (tag == "defaultTransposingClef") {
        s->setDefaultClefType(ClefTypeList(s->defaultClefType().concertClef, TConv::fromXml(e.readAsciiText(), ClefType::G)));
    } else if (tag == "small") {                // obsolete
        s->staffType(Fraction(0, 1))->setSmall(e.readInt());
    } else if (tag == "invisible") {
        s->staffType(Fraction(0, 1))->setInvisible(e.readInt());              // same as: setInvisible(Fraction(0,1)), e.readInt())
    } else if (tag == "hideWhenEmpty") {
        s->setHideWhenEmpty(Staff::HideMode(e.readInt()));
    } else if (tag == "cutaway") {
        s->setCutaway(e.readInt());
    } else if (tag == "showIfSystemEmpty") {
        s->setShowIfEmpty(e.readInt());
    } else if (tag == "hideSystemBarLine") {
        s->setHideSystemBarLine(e.readInt());
    } else if (tag == "mergeMatchingRests") {
        s->setMergeMatchingRests(e.readInt() ? AutoOnOff::ON : AutoOnOff::AUTO);
    } else if (tag == "isStaffVisible") {
        s->setVisible(e.readBool());
    } else if (tag == "keylist") {
        read400::TRead::read(s->keyList(), e, ctx);
    } else if (tag == "bracket") {
        int col = e.intAttribute("col", -1);
        if (col == -1) {
            col = static_cast<int>(s->brackets().size());
        }
        s->setBracketType(col, BracketType(e.intAttribute("type", -1)));
        s->setBracketSpan(col, e.intAttribute("span", 0));
        s->setBracketVisible(col, static_cast<bool>(e.intAttribute("visible", 1)));
        e.readNext();
    } else if (tag == "barLineSpan") {
        s->setBarLineSpan(e.readInt());
    } else if (tag == "barLineSpanFrom") {
        s->setBarLineFrom(e.readInt());
    } else if (tag == "barLineSpanTo") {
        s->setBarLineTo(e.readInt());
    } else if (tag == "distOffset") {
        s->setUserDist(Spatium(e.readDouble()));
    } else if (tag == "mag") {
        /*_userMag =*/
        e.readDouble(0.1, 10.0);
    } else if (tag == "linkedTo") {
        int v = e.readInt() - 1;
        Staff* st = s->score()->masterScore()->staff(v);
        if (s->links()) {
            LOGD("Staff::readProperties: multiple <linkedTo> tags");
            if (!st || s->isLinked(st)) {     // maybe we don't need actually to relink...
                return true;
            }
            // not using unlink() here as it may delete _links
            // a pointer to which is stored also in XmlReader.
            s->links()->remove(s);
            s->setLinks(nullptr);
        }
        if (st && st != s) {
            s->linkTo(st);
        } else if (!s->score()->isMaster() && !st) {
            // if it is a master score it is OK not to find
            // a staff which is going after the current one.
            LOGD("staff %d not found in parent", v);
        }
    } else if (tag == "color") {
        s->staffType(Fraction(0, 1))->setColor(e.readColor());
    } else if (tag == "transposeDiatonic") {
        ctx.setTransposeDiatonic(static_cast<int8_t>(e.readInt()));
    } else if (tag == "transposeChromatic") {
        ctx.setTransposeChromatic(static_cast<int8_t>(e.readInt()));
    } else if (tag == "playbackVoice1") {
        s->setPlaybackVoice(0, e.readInt());
    } else if (tag == "playbackVoice2") {
        s->setPlaybackVoice(1, e.readInt());
    } else if (tag == "playbackVoice3") {
        s->setPlaybackVoice(2, e.readInt());
    } else if (tag == "playbackVoice4") {
        s->setPlaybackVoice(3, e.readInt());
    } else {
        return false;
    }
    return true;
}

void TRead::read(StaffName* item, XmlReader& xml)
{
    item->setPos(xml.intAttribute("pos", 0));
    String name = xml.readXml();
    if (name.startsWith(u"<html>")) {
        // compatibility to old html implementation:
        name = HtmlParser::parse(name);
    }
    item->setName(name);
}

void TRead::read(Stem* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!TRead::readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Stem* s, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "userLen" && s->score()->mscVersion() < 400) {
        // Ignore stem length pre-4.0
        e.skipCurrentElement();
        s->setUserLength(Spatium(0));
    } else if (TRead::readProperty(s, tag, e, ctx, Pid::USER_LEN)) {
    } else if (TRead::readStyledProperty(s, tag, e, ctx)) {
    } else if (readItemProperties(s, e, ctx)) {
    } else {
        return false;
    }
    return true;
}

void TRead::read(StemSlash* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readItemProperties(s, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(StringData* item, XmlReader& e)
{
    item->stringList().clear();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "frets") {
            item->setFrets(e.readInt());
        } else if (tag == "string") {
            instrString strg;
            strg.open  = e.intAttribute("open", 0);
            strg.pitch = e.readInt();
            item->stringList().push_back(strg);
        } else {
            e.unknown();
        }
    }
    if (item->isFiveStringBanjo()) {
        item->configBanjo5thString();
    }
}

void TRead::read(System* s, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "SystemDivider") {
            SystemDivider* sd = new SystemDivider(s);
            TRead::read(sd, e, ctx);
            s->add(sd);
        } else {
            e.unknown();
        }
    }
}

void TRead::read(Text* t, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "style") {
            TextStyleType s = TConv::fromXml(e.readAsciiText(), TextStyleType::DEFAULT);
            if (TextStyleType::TUPLET == s) {  // ugly hack for compatibility
                continue;
            }
            t->initTextStyleType(s);
        } else if (!readProperties(static_cast<TextBase*>(t), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(TextLine* l, XmlReader& e, ReadContext& ctx)
{
    bool system =  e.intAttribute("system", 0) == 1;
    l->setSystemFlag(system);
    l->initStyle();
    TRead::read(static_cast<TextLineBase*>(l), e, ctx);
}

void TRead::read(TextLineBase* b, XmlReader& e, ReadContext& ctx)
{
    b->eraseSpannerSegments();

    if (b->score()->mscVersion() < 301) {
        ctx.addSpanner(e.intAttribute("id", -1), b);
    }

    while (e.readNextStartElement()) {
        if (!readProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Tie* t, XmlReader& xml, ReadContext& ctx)
{
    TRead::read(static_cast<SlurTie*>(t), xml, ctx);
}

void TRead::read(TimeSig* s, XmlReader& e, ReadContext& ctx)
{
    int n=0, z1=0, z2=0, z3=0, z4=0;
    bool old = false;

    TimeSigType timeSigType = TimeSigType::NORMAL;
    Fraction sig;
    Fraction stretch(1, 1);
    String numeratorString;
    String denominatorString;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "den") {
            old = true;
            n = e.readInt();
        } else if (tag == "nom1") {
            old = true;
            z1 = e.readInt();
        } else if (tag == "nom2") {
            old = true;
            z2 = e.readInt();
        } else if (tag == "nom3") {
            old = true;
            z3 = e.readInt();
        } else if (tag == "nom4") {
            old = true;
            z4 = e.readInt();
        } else if (tag == "subtype") {
            int i = e.readInt();
            if (s->score()->mscVersion() <= 114) {
                if (i == 0x40000104) {
                    timeSigType = TimeSigType::FOUR_FOUR;
                } else if (i == 0x40002084) {
                    timeSigType = TimeSigType::ALLA_BREVE;
                } else {
                    timeSigType = TimeSigType::NORMAL;
                }
            } else {
                timeSigType = TimeSigType(i);
            }
        } else if (tag == "showCourtesySig") {
            s->setShowCourtesySig(e.readInt());
        } else if (tag == "sigN") {
            sig.setNumerator(e.readInt());
        } else if (tag == "sigD") {
            sig.setDenominator(e.readInt());
        } else if (tag == "stretchN") {
            stretch.setNumerator(e.readInt());
        } else if (tag == "stretchD") {
            stretch.setDenominator(e.readInt());
        } else if (tag == "textN") {
            numeratorString = e.readText();
        } else if (tag == "textD") {
            denominatorString = e.readText();
        } else if (tag == "Groups") {
            Groups groups;
            TRead::read(&groups, e, ctx);
            s->setGroups(groups);
        } else if (TRead::readStyledProperty(s, tag, e, ctx)) {
        } else if (!readItemProperties(s, e, ctx)) {
            e.unknown();
        }
    }
    if (old) {
        sig.set(z1 + z2 + z3 + z4, n);
    }
    stretch.reduce();

    // HACK: handle time signatures from scores before 3.5 differently on some special occasions.
    // See https://musescore.org/node/308139.
    String version = s->score()->mscoreVersion();
    if (!version.isEmpty() && (version >= u"3.0") && (version < u"3.5")) {
        if ((timeSigType == TimeSigType::NORMAL) && !numeratorString.isEmpty() && denominatorString.isEmpty()) {
            if (numeratorString == String::number(sig.numerator())) {
                numeratorString.clear();
            } else {
                denominatorString = String::number(sig.denominator());
            }
        }
    }

    s->setSig(sig, timeSigType);
    s->setStretch(stretch);
    s->setNumeratorString(numeratorString);
    s->setDenominatorString(denominatorString);
}

void TRead::read(TimeSigMap* item, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "sig") {
            SigEvent t;
            int tick = TRead::read(&t, e, ctx.fileDivision());
            (*item)[tick] = t;
        } else {
            e.unknown();
        }
    }
    item->normalize();
}

int TRead::read(SigEvent* item, XmlReader& e, int fileDivision)
{
    int tick  = e.intAttribute("tick", 0);
    tick      = tick * Constants::DIVISION / fileDivision;

    int numerator = 1;
    int denominator = 1;
    int denominator2 = -1;
    int numerator2   = -1;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "nom") {
            numerator = e.readInt();
        } else if (tag == "denom") {
            denominator = e.readInt();
        } else if (tag == "nom2") {
            numerator2 = e.readInt();
        } else if (tag == "denom2") {
            denominator2 = e.readInt();
        } else {
            e.unknown();
        }
    }
    if ((numerator2 == -1) || (denominator2 == -1)) {
        numerator2   = numerator;
        denominator2 = denominator;
    }

    item->setTimesig(TimeSigFrac(numerator, denominator));
    item->setNominal(TimeSigFrac(numerator2, denominator2));
    return tick;
}

void TRead::read(TremoloCompat& t, XmlReader& e, ReadContext& ctx)
{
    auto createDefaultTremolo = [](TremoloCompat& t) {
        t.single = Factory::createTremoloSingleChord(t.parent);
        t.single->setTrack(t.parent->track());
        t.single->setTremoloType(TremoloType::R8);
    };

    auto item = [createDefaultTremolo](TremoloCompat& t) -> EngravingItem* {
        if (t.two) {
            return t.two;
        }

        if (!t.single) {
            // If no item been created yet at this point,
            // that means no "subtype" tag was present in the XML file.
            // In this case, we create a single eighth-note tremolo,
            // since that was the default.
            createDefaultTremolo(t);
        }

        return t.single;
    };

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            TremoloType type = TConv::fromXml(e.readAsciiText(), TremoloType::INVALID_TREMOLO);
            if (isTremoloTwoChord(type)) {
                t.two = Factory::createTremoloTwoChord(t.parent);
                t.two->setTrack(t.parent->track());
                t.two->setTremoloType(type);
            } else {
                t.single = Factory::createTremoloSingleChord(t.parent);
                t.single->setTrack(t.parent->track());
                t.single->setTremoloType(type);
            }
        }
        // Style needs special handling other than readStyledProperty()
        // to avoid calling customStyleApplicable() in setProperty(),
        // which cannot be called now because durationType() isn't defined yet.
        else if (tag == "strokeStyle") {
            if (t.two) {
                t.two->setTremoloStyle(TremoloStyle(e.readInt()));
                t.two->setPropertyFlags(Pid::TREMOLO_STYLE, PropertyFlags::UNSTYLED);
            } else {
                UNREACHABLE;
                e.skipCurrentElement();
            }
        } else if (tag == "Fragment") {
            if (t.two) {
                BeamFragment f = BeamFragment();
                int idx = t.two->directionIdx();
                t.two->setUserModified(true);
                double _spatium = t.two->spatium();
                while (e.readNextStartElement()) {
                    const AsciiStringView tag1(e.name());
                    if (tag1 == "y1") {
                        f.py1[idx] = e.readDouble() * _spatium;
                    } else if (tag1 == "y2") {
                        f.py2[idx] = e.readDouble() * _spatium;
                    } else {
                        e.unknown();
                    }
                }
                t.two->setBeamFragment(f);
            } else {
                UNREACHABLE;
                e.skipCurrentElement();
            }
        } else if (tag == "play") {
            if (t.two) {
                t.two->setPlayTremolo(e.readBool());
            } else if (t.single) {
                t.single->setPlayTremolo(e.readBool());
            } else {
                UNREACHABLE;
            }
        } else if (TRead::readStyledProperty(item(t), tag, e, ctx)) {
        } else if (!readItemProperties(item(t), e, ctx)) {
            e.unknown();
        }
    }

    if (!t.two && !t.single) {
        // If no item been created yet at this point,
        // that means no "subtype" tag was present in the XML file.
        // In this case, we create a single eighth-note tremolo,
        // since that was the default.
        createDefaultTremolo(t);
    }
}

static constexpr std::array<Pid, 18> TextBasePropertyId { {
    Pid::TEXT_STYLE,
    Pid::FONT_FACE,
    Pid::FONT_SIZE,
    Pid::TEXT_LINE_SPACING,
    Pid::FONT_STYLE,
    Pid::COLOR,
    Pid::FRAME_TYPE,
    Pid::FRAME_WIDTH,
    Pid::FRAME_PADDING,
    Pid::FRAME_ROUND,
    Pid::FRAME_FG_COLOR,
    Pid::FRAME_BG_COLOR,
    Pid::ALIGN,
} };

bool TRead::readTextProperties(TextBase* t, XmlReader& xml, ReadContext& ctx)
{
    if (t->isStaffTextBase()) {
        return readProperties(dynamic_cast<StaffTextBase*>(t), xml, ctx);
    }
    return readProperties(t, xml, ctx);
}

bool TRead::readProperties(TextBase* t, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    for (Pid i : TextBasePropertyId) {
        if (TRead::readProperty(t, tag, e, ctx, i)) {
            return true;
        }
    }

    if (tag == "text") {
        String str = e.readXml();
        t->setXmlText(str);
        t->checkCustomFormatting(str);
    } else if (tag == "bold") {
        bool val = e.readInt();
        if (val) {
            t->setFontStyle(t->fontStyle() + FontStyle::Bold);
        } else {
            t->setFontStyle(t->fontStyle() - FontStyle::Bold);
        }
        if (t->isStyled(Pid::FONT_STYLE)) {
            t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "italic") {
        bool val = e.readInt();
        if (val) {
            t->setFontStyle(t->fontStyle() + FontStyle::Italic);
        } else {
            t->setFontStyle(t->fontStyle() - FontStyle::Italic);
        }
        if (t->isStyled(Pid::FONT_STYLE)) {
            t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "underline") {
        bool val = e.readInt();
        if (val) {
            t->setFontStyle(t->fontStyle() + FontStyle::Underline);
        } else {
            t->setFontStyle(t->fontStyle() - FontStyle::Underline);
        }
        if (t->isStyled(Pid::FONT_STYLE)) {
            t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "strike") {
        bool val = e.readInt();
        if (val) {
            t->setFontStyle(t->fontStyle() + FontStyle::Strike);
        } else {
            t->setFontStyle(t->fontStyle() - FontStyle::Strike);
        }
        if (t->isStyled(Pid::FONT_STYLE)) {
            t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (!readItemProperties(t, e, ctx)) {
        return false;
    }
    return true;
}

bool TRead::readProperties(TextLineBase* b, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    for (Pid i : TextLineBase::textLineBasePropertyIds()) {
        if (TRead::readProperty(b, tag, e, ctx, i)) {
            b->setPropertyFlags(i, PropertyFlags::UNSTYLED);
            return true;
        }
    }
    return readProperties(static_cast<SLine*>(b), e, ctx);
}

void TRead::read(Trill* t, XmlReader& e, ReadContext& ctx)
{
    t->eraseSpannerSegments();

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            t->setTrillType(TConv::fromXml(e.readAsciiText(), TrillType::TRILL_LINE));
        } else if (tag == "Ornament") {
            Ornament* ornament = t->ornament();
            if (!ornament) {
                ornament = Factory::createOrnament(toChordRest(t->parentItem(true)));
                t->setOrnament(ornament);
            }
            ornament->setSymId(Ornament::fromTrillType(t->trillType()));
            TRead::read(ornament, e, ctx);
            ornament->setTrack(t->track());
        } else if (tag == "Accidental") {
            Accidental* accidental = Factory::createAccidental(t);
            TRead::read(accidental, e, ctx);
            accidental->setParent(t);
            t->setAccidental(accidental);
            if (t->ornament()) {
                t->ornament()->setTrillOldCompatAccidental(accidental);
            }
        } else if (tag == "ornamentStyle") {
            readProperty(t, e, ctx, Pid::ORNAMENT_STYLE);
        } else if (!readProperties(static_cast<SLine*>(t), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Vibrato* v, XmlReader& e, ReadContext& ctx)
{
    v->eraseSpannerSegments();

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            v->setVibratoType(TConv::fromXml(e.readAsciiText(), VibratoType::GUITAR_VIBRATO));
        } else if (!readProperties(static_cast<SLine*>(v), e, ctx)) {
            e.unknown();
        }
    }
}

void TRead::read(Volta* v, XmlReader& e, ReadContext& ctx)
{
    v->eraseSpannerSegments();

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "endings") {
            v->setEndings(TConv::fromXml(e.readText(), std::vector<int>()));
        } else if (TRead::readStyledProperty(v, tag, e, ctx)) {
        } else if (!readProperties(v, e, ctx)) {
            e.unknown();
        }
    }
}

bool TRead::readProperties(Volta* v, XmlReader& e, ReadContext& ctx)
{
    if (!readProperties(static_cast<TextLineBase*>(v), e, ctx)) {
        return false;
    }

    if (v->anchor() != Volta::VOLTA_ANCHOR) {
        // Volta strictly assumes that its anchor is measure, so don't let old scores override this.
        LOGW("Correcting volta anchor type from %d to %d", int(v->anchor()), int(Volta::VOLTA_ANCHOR));
        v->setAnchor(Volta::VOLTA_ANCHOR);
    }

    return true;
}

void TRead::readSpanner(XmlReader& e, ReadContext& ctx, EngravingItem* current, track_idx_t track)
{
    std::shared_ptr<ConnectorInfoReader> info(new ConnectorInfoReader(e, &ctx, current, static_cast<int>(track)));
    ConnectorInfoReader::readConnector(info, e, ctx);
}

void TRead::readSpanner(XmlReader& e, ReadContext& ctx, Score* current, track_idx_t track)
{
    std::shared_ptr<ConnectorInfoReader> info(new ConnectorInfoReader(e, &ctx, current, static_cast<int>(track)));
    ConnectorInfoReader::readConnector(info, e, ctx);
}
