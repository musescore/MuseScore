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
#include "connectorinforeader.h"

#include "../../infrastructure/rtti.h"

#include "../../libmscore/factory.h"
#include "../../libmscore/score.h"
#include "../../libmscore/engravingitem.h"
#include "../../libmscore/spanner.h"
#include "../../libmscore/chordrest.h"
#include "../../libmscore/measure.h"
#include "../../libmscore/note.h"
#include "../../libmscore/tie.h"
#include "../../libmscore/chord.h"
#include "../../libmscore/staff.h"

#include "../types/typesconv.h"

#include "../xmlreader.h"

#include "tread.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

//---------------------------------------------------------
//   ConnectorInfoReader
//---------------------------------------------------------

ConnectorInfoReader::ConnectorInfoReader(XmlReader& e, EngravingItem* current, int track)
    : ConnectorInfo(current, track), _reader(&e), _connector(nullptr), _connectorReceiver(current)
{}

//---------------------------------------------------------
//   readPositionInfo
//---------------------------------------------------------

static Location readPositionInfo(const XmlReader& e, int track)
{
    Location info = e.context()->location();
    info.setTrack(track);
    return info;
}

//---------------------------------------------------------
//   ConnectorInfoReader
//---------------------------------------------------------

ConnectorInfoReader::ConnectorInfoReader(XmlReader& e, Score* current, int track)
    : ConnectorInfo(current, readPositionInfo(e, track)), _reader(&e), _connector(nullptr), _connectorReceiver(current)
{
    setCurrentUpdated(true);
}

//---------------------------------------------------------
//   ConnectorInfoReader::read
//---------------------------------------------------------

bool ConnectorInfoReader::read()
{
    XmlReader& e = *_reader;
    const AsciiStringView name(e.asciiAttribute("type"));
    _type = TConv::fromXml(name, ElementType::INVALID);

    e.context()->fillLocation(_currentLoc);

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "prev") {
            readEndpointLocation(_prevLoc);
        } else if (tag == "next") {
            readEndpointLocation(_nextLoc);
        } else {
            if (tag == name) {
                _connector = Factory::createItemByName(tag, _connectorReceiver->score()->dummy());
            } else {
                LOGW("ConnectorInfoReader::read: element tag (%s) does not match connector type (%s). Is the file corrupted?",
                     tag.ascii(), name.ascii());
            }

            if (!_connector) {
                e.unknown();
                return false;
            }
            _connector->setTrack(_currentLoc.track());
            rw400::TRead::readItem(_connector, e, *e.context());
        }
    }
    return true;
}

//---------------------------------------------------------
//   ConnectorInfoReader::readEndpointLocation
//---------------------------------------------------------

void ConnectorInfoReader::readEndpointLocation(Location& l)
{
    XmlReader& e = *_reader;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "location") {
            l = Location::relative();
            rw400::TRead::read(&l, e, *e.context());
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
        updateCurrentInfo(_reader->context()->pasteMode());
    }
    if (hasPrevious()) {
        _prevLoc.toAbsolute(_currentLoc);
    }
    if (hasNext()) {
        _nextLoc.toAbsolute(_currentLoc);
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
        if (T::classof(item) || std::is_same<T, ChordRest>::value) {
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
        ReadAddConnectorVisitor::visit(ReadAddConnectorTypes {}, r->_connectorReceiver, r, pasteMode);
        r = r->next();
    }
}

//---------------------------------------------------------
//   ConnectorInfoReader::readConnector
//---------------------------------------------------------

void ConnectorInfoReader::readConnector(std::unique_ptr<ConnectorInfoReader> info, XmlReader& e)
{
    if (!info->read()) {
        e.skipCurrentElement();
        return;
    }
    e.context()->addConnectorInfoLater(std::move(info));
}

//---------------------------------------------------------
//   ConnectorInfoReader::connector
//---------------------------------------------------------

EngravingItem* ConnectorInfoReader::connector()
{
    // connector should be contained in the first node normally.
    ConnectorInfo* i = findFirst();
    if (i) {
        return static_cast<ConnectorInfoReader*>(i)->_connector;
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
            if (ii->_connector) {
                c = ii->_connector;
                ii->_connector = nullptr;
            }
            ii = ii->prev();
            if (ii == this) {
                break;
            }
        }
        return c;
    }
    EngravingItem* c = i->_connector;
    i->_connector = nullptr;
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
                item->setTieFor(tie);
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
                if (sp->isGlissando() && item->explicitParent() && item->explicitParent()->isChord()) {
                    toChord(item->explicitParent())->setEndsGlissando(true);
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
