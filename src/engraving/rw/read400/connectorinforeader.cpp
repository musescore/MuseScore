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
#include "connectorinforeader.h"

#include "../../infrastructure/rtti.h"

#include "../../dom/factory.h"
#include "../../dom/score.h"
#include "../../dom/engravingitem.h"
#include "../../dom/spanner.h"
#include "../../dom/chordrest.h"
#include "../../dom/measure.h"
#include "../../dom/note.h"
#include "../../dom/noteline.h"
#include "../../dom/tie.h"
#include "../../dom/chord.h"
#include "../../dom/staff.h"

#include "../types/typesconv.h"

#include "../xmlreader.h"

#include "tread.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::read400;

//---------------------------------------------------------
//   ConnectorInfoReader
//---------------------------------------------------------

ConnectorInfoReader::ConnectorInfoReader(XmlReader& e, ReadContext* ctx, EngravingItem* current, int track)
    : ConnectorInfo(current, track), m_reader(&e), m_ctx(ctx), m_connector(nullptr), m_connectorReceiver(current)
{}

//---------------------------------------------------------
//   readPositionInfo
//---------------------------------------------------------

static Location readPositionInfo(ReadContext* ctx, int track)
{
    Location info = ctx->location();
    info.setTrack(track);
    return info;
}

//---------------------------------------------------------
//   ConnectorInfoReader
//---------------------------------------------------------

ConnectorInfoReader::ConnectorInfoReader(XmlReader& e, ReadContext* ctx, Score* current, int track)
    : ConnectorInfo(current, readPositionInfo(ctx, track)), m_reader(&e), m_ctx(ctx), m_connector(nullptr), m_connectorReceiver(current)
{
    setCurrentUpdated(true);
}

//---------------------------------------------------------
//   ConnectorInfoReader::read
//---------------------------------------------------------

bool ConnectorInfoReader::read()
{
    XmlReader& e = *m_reader;
    const AsciiStringView name(e.asciiAttribute("type"));
    m_type = TConv::fromXml(name, ElementType::INVALID);

    m_ctx->fillLocation(m_currentLoc);

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "prev") {
            readEndpointLocation(m_prevLoc);
        } else if (tag == "next") {
            readEndpointLocation(m_nextLoc);
        } else {
            if (tag == name) {
                m_connector = Factory::createItemByName(tag, m_connectorReceiver->score()->dummy());
            } else {
                LOGW("ConnectorInfoReader::read: element tag (%s) does not match connector type (%s). Is the file corrupted?",
                     tag.ascii(), name.ascii());
            }

            if (!m_connector) {
                e.unknown();
                return false;
            }
            m_connector->setTrack(m_currentLoc.track());
            read400::TRead::readItem(m_connector, e, *m_ctx);
        }
    }
    return true;
}

//---------------------------------------------------------
//   ConnectorInfoReader::readEndpointLocation
//---------------------------------------------------------

void ConnectorInfoReader::readEndpointLocation(Location& l)
{
    XmlReader& e = *m_reader;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "location") {
            l = Location::relative();
            read400::TRead::read(&l, e, *m_ctx);
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   ConnectorInfoReader::update
//---------------------------------------------------------

void ConnectorInfoReader::update()
{
    if (!currentUpdated()) {
        updateCurrentInfo(m_ctx->pasteMode());
    }
    if (hasPrevious()) {
        m_prevLoc.toAbsolute(m_currentLoc);
    }
    if (hasNext()) {
        m_nextLoc.toAbsolute(m_currentLoc);
    }
}

//---------------------------------------------------------
//   ConnectorInfoReader::addToScore
//---------------------------------------------------------

class ReadAddConnectorVisitor : public rtti::Visitor<ReadAddConnectorVisitor>
{
public:

    template<typename T>
    static bool doVisit(EngravingObject* item, ConnectorInfoReader* info, bool pasteMode)
    {
        if (T::classof(item) || (std::is_same<T, ChordRest>::value && item->isChordRest())) {
            ConnectorInfoReader::readAddConnector(static_cast<T*>(item), info, pasteMode);
            return true;
        }
        return false;
    }
};

using ReadAddConnectorTypes = rtti::TypeList<ChordRest, Measure, Note, Score>;

void ConnectorInfoReader::addToScore(bool pasteMode)
{
    ConnectorInfoReader* r = this;
    while (r->prev()) {
        r = r->prev();
    }
    while (r) {
        bool found = ReadAddConnectorVisitor::visit(ReadAddConnectorTypes {}, r->m_connectorReceiver, r, pasteMode);
        DO_ASSERT(found);
        r = r->next();
    }
}

//---------------------------------------------------------
//   ConnectorInfoReader::readConnector
//---------------------------------------------------------

void ConnectorInfoReader::readConnector(std::shared_ptr<ConnectorInfoReader> info, XmlReader& e, ReadContext& ctx)
{
    if (!info->read()) {
        e.skipCurrentElement();
        return;
    }
    ctx.addConnectorInfoLater(info);
}

//---------------------------------------------------------
//   ConnectorInfoReader::connector
//---------------------------------------------------------

EngravingItem* ConnectorInfoReader::connector()
{
    // connector should be contained in the first node normally.
    ConnectorInfo* i = findFirst();
    if (i) {
        return static_cast<ConnectorInfoReader*>(i)->m_connector;
    }
    return nullptr;
}

//---------------------------------------------------------
//   ConnectorInfoReader::connector
//---------------------------------------------------------

const EngravingItem* ConnectorInfoReader::connector() const
{
    return const_cast<ConnectorInfoReader*>(this)->connector();
}

//---------------------------------------------------------
//   ConnectorInfoReader::releaseConnector
//---------------------------------------------------------

EngravingItem* ConnectorInfoReader::releaseConnector()
{
    ConnectorInfoReader* i = static_cast<ConnectorInfoReader*>(findFirst());
    if (!i) {
        // circular connector?
        ConnectorInfoReader* ii = this;
        EngravingItem* c = nullptr;
        while (ii->prev()) {
            if (ii->m_connector) {
                c = ii->m_connector;
                ii->m_connector = nullptr;
            }
            ii = ii->prev();
            if (ii == this) {
                break;
            }
        }
        return c;
    }
    EngravingItem* c = i->m_connector;
    i->m_connector = nullptr;
    return c;
}

void ConnectorInfoReader::readAddConnector(ChordRest* item, ConnectorInfoReader* info, bool pasteMode)
{
    const ElementType type = info->type();
    switch (type) {
    case ElementType::SLUR:
    {
        Spanner* spanner = toSpanner(info->connector());
        const Location& l = info->location();

        if (info->isStart()) {
            spanner->setTrack(l.track());
            spanner->setTick(item->tick());
            spanner->setStartElement(item);
            if (pasteMode) {
                item->score()->undoAddElement(spanner);
                for (EngravingObject* ee : spanner->linkList()) {
                    if (ee == spanner) {
                        continue;
                    }
                    Spanner* ls = toSpanner(ee);
                    ls->setTick(spanner->tick());
                    for (EngravingObject* eee : item->linkList()) {
                        ChordRest* cr = toChordRest(eee);
                        if (cr->score() == eee->score() && cr->staffIdx() == ls->staffIdx()) {
                            ls->setTrack(cr->track());
                            if (ls->isSlur()) {
                                ls->setStartElement(cr);
                            }
                            break;
                        }
                    }
                }
            } else {
                item->score()->addSpanner(spanner);
            }
        } else if (info->isEnd()) {
            spanner->setTrack2(l.track());
            spanner->setTick2(item->tick());
            spanner->setEndElement(item);
            if (pasteMode) {
                for (EngravingObject* ee : spanner->linkList()) {
                    if (ee == spanner) {
                        continue;
                    }
                    Spanner* ls = static_cast<Spanner*>(ee);
                    ls->setTick2(spanner->tick2());
                    for (EngravingObject* eee : item->linkList()) {
                        ChordRest* cr = toChordRest(eee);
                        if (cr->score() == eee->score() && cr->staffIdx() == ls->staffIdx()) {
                            ls->setTrack2(cr->track());
                            if (ls->type() == ElementType::SLUR) {
                                ls->setEndElement(cr);
                            }
                            break;
                        }
                    }
                }
            }
        } else {
            LOGD("ChordRest::readAddConnector(): Slur end is neither start nor end");
        }
    }
    break;
    default:
        break;
    }
}

void ConnectorInfoReader::readAddConnector(Measure* item, ConnectorInfoReader* info, bool pasteMode)
{
    const ElementType type = info->type();
    switch (type) {
    case ElementType::HAIRPIN:
    case ElementType::PEDAL:
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::TEXTLINE:
    case ElementType::LET_RING:
    case ElementType::GRADUAL_TEMPO_CHANGE:
    case ElementType::VIBRATO:
    case ElementType::PALM_MUTE:
    case ElementType::WHAMMY_BAR:
    case ElementType::RASGUEADO:
    case ElementType::HARMONIC_MARK:
    case ElementType::PICK_SCRAPE:
    case ElementType::VOLTA:
    {
        Spanner* sp = toSpanner(info->connector());
        const Location& l = info->location();
        Fraction lTick    = l.frac();
        Fraction spTick   = pasteMode ? lTick : (item->tick() + lTick);
        if (info->isStart()) {
            sp->setTrack(l.track());
            sp->setTrack2(sp->track());
            sp->setTick(spTick);
            item->score()->addSpanner(sp);
        } else if (info->isEnd()) {
            sp->setTrack2(l.track());
            sp->setTick2(spTick);
        }
    }
    break;
    default:
        break;
    }
}

void ConnectorInfoReader::readAddConnector(Note* item, ConnectorInfoReader* info, bool pasteMode)
{
    const ElementType type = info->type();
    const Location& l = info->location();
    switch (type) {
    case ElementType::TIE:
    case ElementType::TEXTLINE:
    case ElementType::GLISSANDO:
    {
        Spanner* sp = toSpanner(info->connector());
        if (info->isStart()) {
            sp->setTrack(l.track());
            sp->setTick(item->tick());
            if (sp->isTie()) {
                Note* n = item;
                while (n->tieFor()) {
                    n = n->tieFor()->endNote();
                }
                Tie* tie = toTie(sp);
                tie->setParent(n);
                tie->setStartNote(n);
                n->setTieFor(tie);
            } else {
                sp->setAnchor(Spanner::Anchor::NOTE);
                sp->setStartElement(item);
                item->addSpannerFor(sp);
                sp->setParent(item);
            }
        } else if (info->isEnd()) {
            sp->setTrack2(l.track());
            sp->setTick2(item->tick());
            sp->setEndElement(item);
            if (sp->isTie()) {
                item->setTieBack(toTie(sp));
            } else {
                bool isNoteAnchoredTextLine = sp->isNoteLine() && toNoteLine(sp)->enforceMinLength();
                if ((sp->isGlissando() || isNoteAnchoredTextLine) && item->explicitParent() && item->explicitParent()->isChord()) {
                    toChord(item->explicitParent())->setEndsNoteAnchoredLine(true);
                }
                item->addSpannerBack(sp);
            }

            // As spanners get added after being fully read, they
            // do not get cloned with the note when pasting to
            // linked staves. So add this spanner explicitly.
            if (pasteMode) {
                item->score()->undoAddElement(sp);
            }
        }
    }
    default:
        break;
    }
}

void ConnectorInfoReader::readAddConnector(Score* item, ConnectorInfoReader* info, bool pasteMode)
{
    if (!pasteMode) {
        // How did we get there?
        LOGD("Score::readAddConnector is called not in paste mode.");
        return;
    }
    const ElementType type = info->type();
    switch (type) {
    case ElementType::HAIRPIN:
    case ElementType::PEDAL:
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::TEXTLINE:
    case ElementType::VOLTA:
    case ElementType::PALM_MUTE:
    case ElementType::WHAMMY_BAR:
    case ElementType::RASGUEADO:
    case ElementType::HARMONIC_MARK:
    case ElementType::PICK_SCRAPE:
    case ElementType::LET_RING:
    case ElementType::GRADUAL_TEMPO_CHANGE:
    case ElementType::VIBRATO:
    {
        Spanner* sp = toSpanner(info->connector());
        const Location& l = info->location();
        if (info->isStart()) {
            sp->setAnchor(Spanner::Anchor::SEGMENT);
            sp->setTrack(l.track());
            sp->setTrack2(l.track());
            sp->setTick(l.frac());
        } else if (info->isEnd()) {
            sp->setTick2(l.frac());
            item->undoAddElement(sp);
            if (sp->isOttava()) {
                sp->staff()->updateOttava();
            }
        }
    }
    break;
    default:
        break;
    }
}
