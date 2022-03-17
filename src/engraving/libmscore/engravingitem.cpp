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

/**
 \file
 Implementation of EngravingItem, ElementList
*/

#include "engravingitem.h"

#include <cmath>
#include <QBuffer>

#include "draw/pen.h"
#include "style/style.h"
#include "rw/xml.h"
#include "rw/writecontext.h"

#include "accessibility/accessibleitem.h"
#include "accessibility/accessibleroot.h"

#include "accidental.h"
#include "ambitus.h"
#include "arpeggio.h"
#include "articulation.h"
#include "bagpembell.h"
#include "barline.h"
#include "bend.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "chord.h"
#include "chordline.h"
#include "chordrest.h"
#include "clef.h"
#include "connector.h"
#include "dynamic.h"
#include "figuredbass.h"
#include "fingering.h"
#include "fret.h"
#include "glissando.h"
#include "hairpin.h"
#include "harmony.h"
#include "actionicon.h"
#include "image.h"
#include "instrumentname.h"
#include "instrchange.h"
#include "jump.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "lyrics.h"
#include "marker.h"
#include "measure.h"
#include "mmrest.h"
#include "mscore.h"
#include "notedot.h"
#include "note.h"
#include "noteline.h"
#include "ottava.h"
#include "page.h"
#include "pedal.h"
#include "rehearsalmark.h"
#include "measurerepeat.h"
#include "rest.h"
#include "score.h"
#include "scorefont.h"
#include "segment.h"
#include "slide.h"
#include "slur.h"
#include "spacer.h"
#include "staff.h"
#include "staffstate.h"
#include "stafftext.h"
#include "systemtext.h"
#include "stafftype.h"
#include "stem.h"
#include "sticking.h"
#include "symbol.h"
#include "system.h"
#include "tempotext.h"
#include "textframe.h"
#include "text.h"
#include "measurenumber.h"
#include "mmrestrange.h"
#include "textline.h"
#include "tie.h"
#include "timesig.h"
#include "tremolobar.h"
#include "tremolo.h"
#include "trill.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"
#include "systemdivider.h"
#include "stafftypechange.h"
#include "stafflines.h"
#include "letring.h"
#include "vibrato.h"
#include "palmmute.h"
#include "fermata.h"
#include "shape.h"
#include "factory.h"
#include "linkedobjects.h"

#include "masterscore.h"

//#include "musescoreCore.h"

#include "config.h"

#include "log.h"
#define LOG_PROP() if (0) LOGD()

using namespace mu;
using namespace mu::engraving;

namespace Ms {
// extern bool showInvisible;

EngravingItem* EngravingItemList::at(size_t i) const
{
    return *std::next(begin(), i);
}

EngravingItem::EngravingItem(const ElementType& type, EngravingObject* se, ElementFlags f)
    : EngravingObject(type, se)
{
    _flags         = f;
    _track         = -1;
    _color         = engravingConfiguration()->defaultColor();
    _mag           = 1.0;
    _tag           = 1;
    _z             = -1;
    _offsetChanged = OffsetChange::NONE;
    _minDistance   = Spatium(0.0);
}

EngravingItem::EngravingItem(const EngravingItem& e)
    : EngravingObject(e)
{
    _bbox       = e._bbox;
    _mag        = e._mag;
    _pos        = e._pos;
    _offset     = e._offset;
    _track      = e._track;
    _flags      = e._flags;
    setFlag(ElementFlag::SELECTED, false);
    _tag        = e._tag;
    _z          = e._z;
    _color      = e._color;
    _offsetChanged = e._offsetChanged;
    _minDistance   = e._minDistance;
    itemDiscovered = false;

    //! TODO Please don't remove (igor.korsukov@gmail.com)
    //m_accessible = e.m_accessible->clone(this);
}

EngravingItem::~EngravingItem()
{
    delete m_accessible;
    Score::onElementDestruction(this);
}

void EngravingItem::setupAccessible()
{
    static std::list<ElementType> accessibleDisabled = {
        ElementType::LEDGER_LINE
    };

    if (score() && !score()->isPaletteScore()) {
        if (std::find(accessibleDisabled.begin(), accessibleDisabled.end(), type()) == accessibleDisabled.end()) {
            m_accessible = createAccessible();
            m_accessible->setup();
        }
    }
}

EngravingItem* EngravingItem::parentItem() const
{
    EngravingObject* p = explicitParent();
    if (p && p->isEngravingItem()) {
        return static_cast<EngravingItem*>(p);
    }

    return nullptr;
}

EngravingItemList EngravingItem::childrenItems() const
{
    EngravingItemList list;
    for (EngravingObject* ch : children()) {
        if (ch->isEngravingItem()) {
            list.push_back(static_cast<EngravingItem*>(ch));
        }
    }
    return list;
}

mu::engraving::AccessibleItem* EngravingItem::createAccessible()
{
    return new mu::engraving::AccessibleItem(this);
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void EngravingItem::spatiumChanged(qreal oldValue, qreal newValue)
{
    if (offsetIsSpatiumDependent()) {
        _offset *= (newValue / oldValue);
    }
}

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void EngravingItem::localSpatiumChanged(qreal oldValue, qreal newValue)
{
    if (offsetIsSpatiumDependent()) {
        _offset *= (newValue / oldValue);
    }
}

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal EngravingItem::spatium() const
{
    if (systemFlag() || (explicitParent() && parentItem()->systemFlag())) {
        return score()->spatium();
    }
    Staff* s = staff();
    return s ? s->spatium(this) : score()->spatium();
}

bool EngravingItem::isInteractionAvailable() const
{
    if (!visible() && (score()->printing() || !score()->showInvisible())) {
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   offsetIsSpatiumDependent
//---------------------------------------------------------

bool EngravingItem::offsetIsSpatiumDependent() const
{
    return sizeIsSpatiumDependent() || (_flags & ElementFlag::ON_STAFF);
}

//---------------------------------------------------------
//   magS
//---------------------------------------------------------

qreal EngravingItem::magS() const
{
    return mag() * (score()->spatium() / SPATIUM20);
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString EngravingItem::subtypeName() const
{
    return "";
}

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

EngravingItem* EngravingItem::linkedClone()
{
    EngravingItem* e = clone();
    e->setAutoplace(true);
    score()->undo(new Link(e, this));
    return e;
}

//---------------------------------------------------------
//   deleteLater
//---------------------------------------------------------

void EngravingItem::deleteLater()
{
    if (selected()) {
        score()->deselect(this);
    }
    masterScore()->deleteLater(this);
}

//---------------------------------------------------------
//   scanElements
/// If leaf node, apply `func` on this element (after checking if it is visible).
/// Otherwise, recurse over all children (see ScoreElement::scanElements).
/// Note: This function is overridden in some classes to skip certain children,
/// or to apply `func` even to non-leaf nodes.
//---------------------------------------------------------

void EngravingItem::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (scanChildCount() == 0) {
        if (all || visible() || score()->showInvisible()) {
            func(data, this);
        }
    } else {
        EngravingObject::scanElements(data, func, all);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void EngravingItem::reset()
{
    undoResetProperty(Pid::AUTOPLACE);
    undoResetProperty(Pid::PLACEMENT);
    undoResetProperty(Pid::MIN_DISTANCE);
    undoResetProperty(Pid::OFFSET);
    setOffsetChanged(false);
    EngravingObject::reset();
}

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void EngravingItem::change(EngravingItem* o, EngravingItem* n)
{
    remove(o);
    add(n);
}

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* EngravingItem::staff() const
{
    if (!hasStaff() || score()->staves().empty()) {
        return nullptr;
    }

    return score()->staff(staffIdx());
}

bool EngravingItem::hasStaff() const
{
    return _track != INVALID_INDEX;
}

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType* EngravingItem::staffType() const
{
    Staff* s = staff();
    return s ? s->staffTypeForElement(this) : nullptr;
}

//---------------------------------------------------------
//   onTabStaff
//---------------------------------------------------------

bool EngravingItem::onTabStaff() const
{
    const StaffType* stt = staffType();
    return stt ? stt->isTabStaff() : false;
}

bool EngravingItem::hasGrips() const
{
    return gripsCount() > 0;
}

int EngravingItem::track() const
{
    return _track;
}

void EngravingItem::setTrack(int val)
{
    _track = val;
}

//---------------------------------------------------------
//   z
//---------------------------------------------------------

int EngravingItem::z() const
{
    if (_z == -1) {
        _z = int(type()) * 100;
    }
    return _z;
}

void EngravingItem::setZ(int val)
{
    _z = val;
}

int EngravingItem::staffIdx() const
{
    return track2staff(_track);
}

void EngravingItem::setStaffIdx(int val)
{
    _track = staff2track(val, voice());
}

int EngravingItem::staffIdxOrNextVisible() const
{
    // for system objects, sometimes the staff they're on is hidden so we have to find the next
    // best staff for them
    if (!staff()) {
        return -1;
    }
    int si = staff()->idx();
    if (!systemFlag()) {
        return si;
    }
    Measure* m = nullptr;
    if (parent() && parent()->isSegment()) {
        Segment* s = parent() ? toSegment(parent()) : nullptr;
        m = s ? s->measure() : nullptr;
    } else if (parent() && parent()->isMeasure()) {
        m = parent() ? toMeasure(parent()) : nullptr;
    }
    if (!m || !m->system() || !m->system()->staff(si)) {
        return si;
    }
    int firstVis = m->system()->firstVisibleStaff();
    if (!isLinked() || (track() == 0 && (_links->mainElement()->score() != score()
                                         || !toEngravingItem(_links->mainElement())->enabled()))) {
        // original, put on the top of the score
        return firstVis;
    }
    if (si <= firstVis) {
        // we already know this staff will be replaced by the original
        return -1;
    }
    bool foundStaff = false;
    if (!m->system()->staff(si)->show()) {
        QList<Staff*> soStaves = score()->getSystemObjectStaves();
        for (int i = 0; i < soStaves.size(); ++i) {
            int idxOrig = soStaves[i]->idx();
            if (idxOrig == si) {
                // this is the staff we are supposed to be on
                for (int idxNew = si + 1; idxNew < score()->staves().size(); ++idxNew) {
                    if (i + 1 < soStaves.size() && idxNew >= score()->staffIdx(soStaves[i + 1]->part())) {
                        // This is the flag to not show this element
                        si = -1;
                        break;
                    } else if (m->system()->staff(idxNew)->show()) {
                        // Move current element to this staff and finish
                        foundStaff = true;
                        si = idxNew;
                        break;
                    }
                }
                break;
            }
        }
    } else {
        // the staff this object should be on is visible, so npnp
        foundStaff = true;
    }
    return foundStaff ? si : -1;
}

int EngravingItem::vStaffIdx() const
{
    return staffIdx();
}

int EngravingItem::voice() const
{
    return track2voice(_track);
}

void EngravingItem::setVoice(int v)
{
    _track = (_track / VOICES) * VOICES + v;
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction EngravingItem::tick() const
{
    const EngravingItem* e = this;
    while (e->explicitParent()) {
        if (e->explicitParent()->isSegment()) {
            return toSegment(e->explicitParent())->tick();
        } else if (e->explicitParent()->isMeasureBase()) {
            return toMeasureBase(e->explicitParent())->tick();
        }
        e = e->parentItem();
    }
    return Fraction(0, 1);
}

//---------------------------------------------------------
//   rtick
//---------------------------------------------------------

Fraction EngravingItem::rtick() const
{
    const EngravingItem* e = this;
    while (e->explicitParent()) {
        if (e->explicitParent()->isSegment()) {
            return toSegment(e->explicitParent())->rtick();
        }
        e = e->parentItem();
    }
    return Fraction(0, 1);
}

//---------------------------------------------------------
//   playTick
//---------------------------------------------------------

Fraction EngravingItem::playTick() const
{
    // Play from the element's tick position by default.
    return tick();
}

//---------------------------------------------------------
//   beat
//---------------------------------------------------------

Fraction EngravingItem::beat() const
{
    // Returns an appropriate fraction of ticks for use as a "Beat" reference
    // in the Select All Similar filter.
    int bar, beat, ticks;
    TimeSigMap* tsm = score()->sigmap();
    tsm->tickValues(tick().ticks(), &bar, &beat, &ticks);
    int ticksB = ticks_beat(tsm->timesig(tick().ticks()).timesig().denominator());

    Fraction complexFraction((++beat * ticksB) + ticks, ticksB);
    return complexFraction.reduced();
}

//---------------------------------------------------------
//   part
//---------------------------------------------------------

Part* EngravingItem::part() const
{
    Staff* s = staff();
    return s ? s->part() : 0;
}

draw::Color EngravingItem::color() const
{
    return _color;
}

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

mu::draw::Color EngravingItem::curColor() const
{
    return curColor(visible());
}

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

mu::draw::Color EngravingItem::curColor(bool isVisible) const
{
    return curColor(isVisible, color());
}

mu::draw::Color EngravingItem::curColor(bool isVisible, Color normalColor) const
{
    // the default element color is always interpreted as black in printing
    if (score() && score()->printing()) {
        return (normalColor == engravingConfiguration()->defaultColor()) ? Color::black : normalColor;
    }

    if (flag(ElementFlag::DROP_TARGET)) {
        return engravingConfiguration()->highlightSelectionColor(track() == -1 ? 0 : voice());
    }

    bool marked = false;
    if (isNote()) {
        marked = toNote(this)->mark();
    }

    if (selected() || marked) {
        Color originalColor = engravingConfiguration()->selectionColor(track() == -1 ? 0 : voice());

        if (isVisible) {
            return originalColor;
        }

        constexpr float tint = .6f; // Between 0 and 1. Higher means lighter, lower means darker

        int red = originalColor.red();
        int green = originalColor.green();
        int blue = originalColor.blue();

        return Color(red + tint * (255 - red), green + tint * (255 - green), blue + tint * (255 - blue));
    }

    if (!isVisible) {
        return engravingConfiguration()->invisibleColor();
    }

    if (m_colorsInversionEnabled && engravingConfiguration()->scoreInversionEnabled()) {
        return engravingConfiguration()->scoreInversionColor();
    }

    return normalColor;
}

//---------------------------------------------------------
//   pagePos
//    return position in canvas coordinates
//---------------------------------------------------------

PointF EngravingItem::pagePos() const
{
    PointF p(pos());
    if (explicitParent() == 0) {
        return p;
    }
    int idx = vStaffIdx();
    if (systemFlag()) {
        idx = staffIdxOrNextVisible();
        if (idx == -1) {
            idx = vStaffIdx();
        }
    }

    if (_flags & ElementFlag::ON_STAFF) {
        System* system = nullptr;
        Measure* measure = nullptr;
        if (explicitParent()->isSegment()) {
            measure = toSegment(explicitParent())->measure();
        } else if (explicitParent()->isMeasure()) {           // used in measure number
            measure = toMeasure(explicitParent());
        } else if (explicitParent()->isSystem()) {
            system = toSystem(explicitParent());
        } else if (explicitParent()->isFretDiagram()) {
            return p + parentItem()->pagePos();
        } else {
            qFatal("this %s parent %s\n", typeName(), explicitParent()->typeName());
        }
        if (measure) {
            system = measure->system();
            p.ry() += measure->staffLines(idx)->y();
        }
        if (system) {
            if (system->staves()->size() <= idx) {
                qDebug("staffIdx out of bounds: %s", typeName());
            }
            p.ry() += system->staffYpage(idx);
        }
        p.rx() = pageX();
    } else {
        if (explicitParent()->explicitParent()) {
            p += parentItem()->pagePos();
        }
    }
    return p;
}

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

PointF EngravingItem::canvasPos() const
{
    PointF p(pos());
    if (explicitParent() == nullptr) {
        return p;
    }
    int idx = vStaffIdx();
    if (systemFlag()) {
        idx = staffIdxOrNextVisible();
        if (idx == -1) {
            idx = vStaffIdx();
        }
    }

    if (_flags & ElementFlag::ON_STAFF) {
        System* system = nullptr;
        Measure* measure = nullptr;
        if (explicitParent()->isSegment()) {
            measure = toSegment(explicitParent())->measure();
        } else if (explicitParent()->isMeasure()) {     // used in measure number
            measure = toMeasure(explicitParent());
        }
        // system = toMeasure(parent())->system();
        else if (explicitParent()->isSystem()) {
            system = toSystem(explicitParent());
        } else if (explicitParent()->isChord()) {       // grace chord
            measure = toSegment(explicitParent()->explicitParent())->measure();
        } else if (explicitParent()->isFretDiagram()) {
            return p + parentItem()->canvasPos() + PointF(toFretDiagram(explicitParent())->centerX(), 0.0);
        } else {
            qFatal("this %s parent %s\n", typeName(), explicitParent()->typeName());
        }
        if (measure) {
            const StaffLines* lines = measure->staffLines(idx);
            p.ry() += lines ? lines->y() : 0;

            system = measure->system();

            if (system) {
                Page* page = system->page();
                if (page) {
                    p.ry() += page->y();
                }
            }
        }
        if (system) {
            p.ry() += system->staffYpage(idx);
        }
        p.rx() = canvasX();
    } else {
        p += parentItem()->canvasPos();
    }
    return p;
}

//---------------------------------------------------------
//   pageX
//---------------------------------------------------------

qreal EngravingItem::pageX() const
{
    qreal xp = x();
    for (EngravingItem* e = parentItem(); e && e->parentItem(); e = e->parentItem()) {
        xp += e->x();
    }
    return xp;
}

//---------------------------------------------------------
//    canvasX
//---------------------------------------------------------

qreal EngravingItem::canvasX() const
{
    qreal xp = x();
    for (EngravingItem* e = parentItem(); e; e = e->parentItem()) {
        xp += e->x();
    }
    return xp;
}

//---------------------------------------------------------
//   contains
//    Return true if p is inside the shape of the object.
//    Note: p is in page coordinates
//---------------------------------------------------------

bool EngravingItem::contains(const mu::PointF& p) const
{
    return shape().contains(p - pagePos());
}

//---------------------------------------------------------
//  intersects
//    Return true if \a rr intersects bounding box of object.
//    Note: \a rr is in page coordinates
//---------------------------------------------------------

bool EngravingItem::intersects(const RectF& rr) const
{
    return shape().intersects(rr.translated(-pagePos()));
}

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void EngravingItem::writeProperties(XmlWriter& xml) const
{
    bool autoplaceEnabled = score()->styleB(Sid::autoplaceEnabled);
    if (!autoplaceEnabled) {
        score()->setStyleValue(Sid::autoplaceEnabled, true);
        writeProperty(xml, Pid::AUTOPLACE);
        score()->setStyleValue(Sid::autoplaceEnabled, autoplaceEnabled);
    } else {
        writeProperty(xml, Pid::AUTOPLACE);
    }

    // copy paste should not keep links
    if (_links && (_links->size() > 1) && !xml.clipboardmode()) {
        WriteContext* ctx = xml.context();
        IF_ASSERT_FAILED(ctx) {
            return;
        }

        if (MScore::debugMode) {
            xml.tag("lid", _links->lid());
        }

        EngravingItem* me = static_cast<EngravingItem*>(_links->mainElement());
        Q_ASSERT(type() == me->type());
        Staff* s = staff();
        if (!s) {
            s = score()->staff(xml.curTrack() / VOICES);
            if (!s) {
                qWarning("EngravingItem::writeProperties: linked element's staff not found (%s)", typeName());
            }
        }
        Location loc = Location::positionForElement(this);
        if (me == this) {
            xml.tagE("linkedMain");
            int index = ctx->assignLocalIndex(loc);
            ctx->setLidLocalIndex(_links->lid(), index);
        } else {
            if (s->links()) {
                Staff* linkedStaff = toStaff(s->links()->mainElement());
                loc.setStaff(linkedStaff->idx());
            }
            xml.startObject("linked");
            if (!me->score()->isMaster()) {
                if (me->score() == score()) {
                    xml.tag("score", "same");
                } else {
                    qWarning(
                        "EngravingItem::writeProperties: linked elements belong to different scores but none of them is master score: (%s lid=%d)",
                        typeName(), _links->lid());
                }
            }

            Location mainLoc = Location::positionForElement(me);
            const int guessedLocalIndex = ctx->assignLocalIndex(mainLoc);
            if (loc != mainLoc) {
                mainLoc.toRelative(loc);
                mainLoc.write(xml);
            }
            const int indexDiff = ctx->lidLocalIndex(_links->lid()) - guessedLocalIndex;
            xml.tag("indexDiff", indexDiff, 0);
            xml.endObject();       // </linked>
        }
    }
    if ((xml.writeTrack() || track() != xml.curTrack())
        && (track() != -1) && !isBeam()) {
        // Writing track number for beams is redundant as it is calculated
        // during layout.
        int t = track() + xml.trackDiff();
        xml.tag("track", t);
    }
    if (xml.writePosition()) {
        xml.tag(Pid::POSITION, rtick());
    }
    if (_tag != 0x1) {
        for (int i = 1; i < MAX_TAGS; i++) {
            if (_tag == ((unsigned)1 << i)) {
                xml.tag("tag", score()->layerTags()[i]);
                break;
            }
        }
    }
    for (Pid pid : { Pid::OFFSET, Pid::COLOR, Pid::VISIBLE, Pid::Z, Pid::PLACEMENT }) {
        if (propertyFlags(pid) == PropertyFlags::NOSTYLE) {
            writeProperty(xml, pid);
        }
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool EngravingItem::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());

    if (readProperty(tag, e, Pid::SIZE_SPATIUM_DEPENDENT)) {
    } else if (readProperty(tag, e, Pid::OFFSET)) {
    } else if (readProperty(tag, e, Pid::MIN_DISTANCE)) {
    } else if (readProperty(tag, e, Pid::AUTOPLACE)) {
    } else if (tag == "track") {
        setTrack(e.readInt() + e.trackOffset());
    } else if (tag == "color") {
        setColor(e.readColor());
    } else if (tag == "visible") {
        setVisible(e.readInt());
    } else if (tag == "selected") { // obsolete
        e.readInt();
    } else if ((tag == "linked") || (tag == "linkedMain")) {
        ReadContext* ctx = e.context();
        IF_ASSERT_FAILED(ctx) {
            return false;
        }

        Staff* s = staff();
        if (!s) {
            s = score()->staff(e.track() / VOICES);
            if (!s) {
                qWarning("EngravingItem::readProperties: linked element's staff not found (%s)", typeName());
                e.skipCurrentElement();
                return true;
            }
        }
        if (tag == "linkedMain") {
            _links = new LinkedObjects(score());
            _links->push_back(this);

            ctx->addLink(s, _links, e.location(true));

            e.readNext();
        } else {
            Staff* ls = s->links() ? toStaff(s->links()->mainElement()) : nullptr;
            bool linkedIsMaster = ls ? ls->score()->isMaster() : false;
            Location loc = e.location(true);
            if (ls) {
                loc.setStaff(ls->idx());
            }
            Location mainLoc = Location::relative();
            bool locationRead = false;
            int localIndexDiff = 0;
            while (e.readNextStartElement()) {
                const QStringRef& ntag(e.name());

                if (ntag == "score") {
                    QString val(e.readElementText());
                    if (val == "same") {
                        linkedIsMaster = score()->isMaster();
                    }
                } else if (ntag == "location") {
                    mainLoc.read(e);
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
            LinkedObjects* link = ctx->getLink(linkedIsMaster, mainLoc, localIndexDiff);
            if (link) {
                EngravingObject* linked = link->mainElement();
                if (linked->type() == type()) {
                    linkTo(linked);
                } else {
                    qWarning("EngravingItem::readProperties: linked elements have different types: %s, %s. Input file corrupted?",
                             typeName(), linked->typeName());
                }
            }
            if (!_links) {
                qWarning("EngravingItem::readProperties: could not link %s at staff %d", typeName(), mainLoc.staff() + 1);
            }
        }
    } else if (tag == "lid") {
        if (score()->mscVersion() >= 301) {
            e.skipCurrentElement();
            return true;
        }
        int id = e.readInt();
        _links = e.linkIds().value(id);
        if (!_links) {
            if (!score()->isMaster()) {       // DEBUG
                qDebug("---link %d not found (%d)", id, e.linkIds().size());
            }
            _links = new LinkedObjects(score(), id);
            e.linkIds().insert(id, _links);
        }
#ifndef NDEBUG
        else {
            for (EngravingObject* eee : *_links) {
                EngravingItem* ee = static_cast<EngravingItem*>(eee);
                if (ee->type() != type()) {
                    qFatal("link %s(%d) type mismatch %s linked to %s",
                           ee->typeName(), id, ee->typeName(), typeName());
                }
            }
        }
#endif
        Q_ASSERT(!_links->contains(this));
        _links->append(this);
    } else if (tag == "tick") {
        int val = e.readInt();
        if (val >= 0) {
            e.setTick(Fraction::fromTicks(score()->fileDivision(val)));             // obsolete
        }
    } else if (tag == "pos") {           // obsolete
        readProperty(e, Pid::OFFSET);
    } else if (tag == "voice") {
        setVoice(e.readInt());
    } else if (tag == "tag") {
        QString val(e.readElementText());
        for (int i = 1; i < MAX_TAGS; i++) {
            if (score()->layerTags()[i] == val) {
                _tag = 1 << i;
                break;
            }
        }
    } else if (readProperty(tag, e, Pid::PLACEMENT)) {
    } else if (tag == "z") {
        setZ(e.readInt());
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void EngravingItem::write(XmlWriter& xml) const
{
    xml.startObject(this);
    writeProperties(xml);
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void EngravingItem::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (!readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   remove
///   Remove \a el from the list. Return true on success.
//---------------------------------------------------------

bool ElementList::remove(EngravingItem* el)
{
    auto i = find(begin(), end(), el);
    if (i == end()) {
        return false;
    }
    erase(i);
    return true;
}

//---------------------------------------------------------
//   replace
//---------------------------------------------------------

void ElementList::replace(EngravingItem* o, EngravingItem* n)
{
    auto i = find(begin(), end(), o);
    if (i == end()) {
        qDebug("ElementList::replace: element not found");
        return;
    }
    *i = n;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ElementList::write(XmlWriter& xml) const
{
    for (const EngravingItem* e : *this) {
        e->write(xml);
    }
}

//---------------------------------------------------------
//   Compound
//---------------------------------------------------------

Compound::Compound(const ElementType& type, Score* s)
    : EngravingItem(type, s)
{
}

Compound::Compound(const Compound& c)
    : EngravingItem(c)
{
    elements.clear();
    foreach (EngravingItem* e, c.elements) {
        elements.append(e->clone());
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Compound::draw(mu::draw::Painter* painter) const
{
    foreach (EngravingItem* e, elements) {
        PointF pt(e->pos());
        painter->translate(pt);
        e->draw(painter);
        painter->translate(-pt);
    }
}

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 offset \a x and \a y are in Point units
*/

void Compound::addElement(EngravingItem* e, qreal x, qreal y)
{
    e->setPos(x, y);
    e->setParent(this);
    elements.push_back(e);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Compound::layout()
{
    setbbox(RectF());
    for (auto i = elements.begin(); i != elements.end(); ++i) {
        EngravingItem* e = *i;
        e->layout();
        addbbox(e->bbox().translated(e->pos()));
    }
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Compound::setSelected(bool f)
{
    EngravingItem::setSelected(f);
    for (auto i = elements.begin(); i != elements.end(); ++i) {
        (*i)->setSelected(f);
    }
}

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Compound::setVisible(bool f)
{
    EngravingItem::setVisible(f);
    for (auto i = elements.begin(); i != elements.end(); ++i) {
        (*i)->setVisible(f);
    }
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Compound::clear()
{
    foreach (EngravingItem* e, elements) {
        if (e->selected()) {
            score()->deselect(e);
        }
        delete e;
    }
    elements.clear();
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void EngravingItem::dump() const
{
    qDebug("---EngravingItem: %s, pos(%4.2f,%4.2f)"
           "\n   bbox(%g,%g,%g,%g)"
           "\n   abox(%g,%g,%g,%g)"
           "\n  parent: %p",
           typeName(), ipos().x(), ipos().y(),
           _bbox.x(), _bbox.y(), _bbox.width(), _bbox.height(),
           abbox().x(), abbox().y(), abbox().width(), abbox().height(),
           explicitParent());
}

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray EngravingItem::mimeData(const PointF& dragOffset) const
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    XmlWriter xml(score(), &buffer);
    xml.setClipboardmode(true);
    xml.startObject("EngravingItem");
    if (isNote()) {
        xml.tag("duration", toNote(this)->chord()->ticks());
    }
    if (!dragOffset.isNull()) {
        xml.tag("dragOffset", dragOffset);
    }
    write(xml);
    xml.endObject();
    buffer.close();
    return buffer.buffer();
}

//---------------------------------------------------------
//   readType
//    return new position of QDomElement in e
//---------------------------------------------------------

ElementType EngravingItem::readType(XmlReader& e, PointF* dragOffset,
                                    Fraction* duration)
{
    while (e.readNextStartElement()) {
        if (e.name() == "EngravingItem") {
            while (e.readNextStartElement()) {
                const QStringRef& tag = e.name();
                if (tag == "dragOffset") {
                    *dragOffset = e.readPoint();
                } else if (tag == "duration") {
                    *duration = e.readFraction();
                } else {
                    ElementType type = Factory::name2type(tag);
                    if (type == ElementType::INVALID) {
                        break;
                    }
                    return type;
                }
            }
        } else {
            e.unknown();
        }
    }
    return ElementType::INVALID;
}

//---------------------------------------------------------
//   readMimeData
//---------------------------------------------------------

EngravingItem* EngravingItem::readMimeData(Score* score, const QByteArray& data, PointF* dragOffset, Fraction* duration)
{
    XmlReader e(data);
    const ElementType type = EngravingItem::readType(e, dragOffset, duration);
    e.setPasteMode(true);

    if (type == ElementType::INVALID) {
        qDebug("cannot read type");
        return nullptr;
    }

    EngravingItem* el = Factory::createItem(type, score->dummy(), false);
    if (el) {
        el->read(e);
    }

    return el;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void EngravingItem::add(EngravingItem* e)
{
    qDebug("EngravingItem: cannot add %s to %s", e->typeName(), typeName());
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void EngravingItem::remove(EngravingItem* e)
{
    qFatal("EngravingItem: cannot remove %s from %s", e->typeName(), typeName());
}

//---------------------------------------------------------
//   elementLessThan
//---------------------------------------------------------

bool elementLessThan(const EngravingItem* const e1, const EngravingItem* const e2)
{
    if (e1->z() == e2->z()) {
        if (e1->selected()) {
            return false;
        } else if (e2->selected()) {
            return true;
        } else if (!e1->visible()) {
            return true;
        } else if (!e2->visible()) {
            return false;
        } else {
            return e1->track() > e2->track();
        }
    }
    return e1->z() <= e2->z();
}

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void collectElements(void* data, EngravingItem* e)
{
    QList<EngravingItem*>* el = static_cast<QList<EngravingItem*>*>(data);
    el->append(e);
}

//---------------------------------------------------------
//   autoplace
//---------------------------------------------------------

bool EngravingItem::autoplace() const
{
    if (!score() || !score()->styleB(Sid::autoplaceEnabled)) {
        return false;
    }
    return !flag(ElementFlag::NO_AUTOPLACE);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue EngravingItem::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TICK:
        return tick();
    case Pid::TRACK:
        return track();
    case Pid::VOICE:
        return voice();
    case Pid::POSITION:
        return rtick();
    case Pid::GENERATED:
        return generated();
    case Pid::COLOR:
        return PropertyValue::fromValue(color());
    case Pid::VISIBLE:
        return visible();
    case Pid::SELECTED:
        return selected();
    case Pid::OFFSET:
        return PropertyValue::fromValue(_offset);
    case Pid::MIN_DISTANCE:
        return _minDistance;
    case Pid::PLACEMENT:
        return placement();
    case Pid::AUTOPLACE:
        return autoplace();
    case Pid::Z:
        return z();
    case Pid::SYSTEM_FLAG:
        return systemFlag();
    case Pid::SIZE_SPATIUM_DEPENDENT:
        return sizeIsSpatiumDependent();
    default:
        if (explicitParent()) {
            return explicitParent()->getProperty(propertyId);
        }

        return PropertyValue();
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool EngravingItem::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::TRACK:
        setTrack(v.toInt());
        break;
    case Pid::VOICE:
        setVoice(v.toInt());
        break;
    case Pid::GENERATED:
        setGenerated(v.toBool());
        break;
    case Pid::COLOR:
        setColor(v.value<mu::draw::Color>());
        break;
    case Pid::VISIBLE:
        setVisible(v.toBool());
        break;
    case Pid::SELECTED:
        setSelected(v.toBool());
        break;
    case Pid::OFFSET:
        _offset = v.value<PointF>();
        break;
    case Pid::MIN_DISTANCE:
        setMinDistance(v.value<Spatium>());
        break;
    case Pid::PLACEMENT:
        setPlacement(v.value<PlacementV>());
        break;
    case Pid::AUTOPLACE:
        setAutoplace(v.toBool());
        break;
    case Pid::Z:
        setZ(v.toInt());
        break;
    case Pid::SYSTEM_FLAG:
        setSystemFlag(v.toBool());
        break;
    case Pid::SIZE_SPATIUM_DEPENDENT:
        setSizeIsSpatiumDependent(v.toBool());
        break;
    default:
        if (explicitParent()) {
            return explicitParent()->setProperty(propertyId, v);
        }

        LOG_PROP() << typeName() << " unknown <" << propertyName(propertyId) << ">(" << int(propertyId) << "), data: " << v.toString();
        return false;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void EngravingItem::undoChangeProperty(Pid pid, const PropertyValue& val, PropertyFlags ps)
{
    if (pid == Pid::AUTOPLACE && (val.toBool() == true && !autoplace())) {
        // Switching autoplacement on. Save user-defined
        // placement properties to undo stack.
        undoPushProperty(Pid::PLACEMENT);
        undoPushProperty(Pid::OFFSET);
    }
    EngravingObject::undoChangeProperty(pid, val, ps);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue EngravingItem::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::GENERATED:
        return false;
    case Pid::VISIBLE:
        return true;
    case Pid::COLOR:
        return PropertyValue::fromValue(engravingConfiguration()->defaultColor());
    case Pid::PLACEMENT: {
        PropertyValue v = EngravingObject::propertyDefault(pid);
        if (v.isValid()) {        // if it's a styled property
            return v;
        }
        return PlacementV::BELOW;
    }
    case Pid::SELECTED:
        return false;
    case Pid::OFFSET: {
        PropertyValue v = EngravingObject::propertyDefault(pid);
        if (v.isValid()) {        // if it's a styled property
            return v;
        }
        return PropertyValue::fromValue(PointF());
    }
    case Pid::MIN_DISTANCE: {
        PropertyValue v = EngravingObject::propertyDefault(pid);
        if (v.isValid()) {
            return v;
        }
        return 0.0;
    }
    case Pid::AUTOPLACE:
        return true;
    case Pid::Z:
        return int(type()) * 100;
    default: {
        PropertyValue v = EngravingObject::propertyDefault(pid);

        if (v.isValid()) {
            return v;
        }

        if (explicitParent()) {
            return explicitParent()->propertyDefault(pid);
        }

        return PropertyValue();
    }
    }
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid EngravingItem::propertyId(const QStringRef& name) const
{
    if (name == "pos" || name == "offset") {
        return Pid::OFFSET;
    }
    return EngravingObject::propertyId(name);
}

//---------------------------------------------------------
//   custom
//    check if property is != default
//---------------------------------------------------------

bool EngravingItem::custom(Pid id) const
{
    return propertyDefault(id) != getProperty(id);
}

//---------------------------------------------------------
//   isPrintable
//---------------------------------------------------------

bool EngravingItem::isPrintable() const
{
    switch (type()) {
    case ElementType::PAGE:
    case ElementType::SYSTEM:
    case ElementType::MEASURE:
    case ElementType::SEGMENT:
    case ElementType::VBOX:
    case ElementType::HBOX:
    case ElementType::TBOX:
    case ElementType::FBOX:
    case ElementType::SPACER:
    case ElementType::SHADOW_NOTE:
    case ElementType::LASSO:
    case ElementType::ELEMENT_LIST:
    case ElementType::STAFF_LIST:
    case ElementType::MEASURE_LIST:
    case ElementType::SELECTION:
        return false;
    default:
        return true;
    }
}

bool EngravingItem::isPlayable() const
{
    switch (type()) {
    case ElementType::NOTE:
    case ElementType::CHORD:
        return true;
    default:
        return false;
    }
}

//---------------------------------------------------------
//   findAncestor
//---------------------------------------------------------

EngravingItem* EngravingItem::findAncestor(ElementType t)
{
    EngravingItem* e = this;
    while (e && e->type() != t) {
        e = e->parentItem();
    }
    return e;
}

const EngravingItem* EngravingItem::findAncestor(ElementType t) const
{
    const EngravingItem* e = this;
    while (e && e->type() != t) {
        e = e->parentItem();
    }
    return e;
}

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

Measure* EngravingItem::findMeasure()
{
    if (isMeasure()) {
        return toMeasure(this);
    } else if (explicitParent()) {
        return parentItem()->findMeasure();
    } else {
        return 0;
    }
}

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

const Measure* EngravingItem::findMeasure() const
{
    EngravingItem* e = const_cast<EngravingItem*>(this);
    return e->findMeasure();
}

//---------------------------------------------------------
//   findMeasureBase
//---------------------------------------------------------

MeasureBase* EngravingItem::findMeasureBase()
{
    if (isMeasureBase()) {
        return toMeasureBase(this);
    } else if (explicitParent()) {
        return parentItem()->findMeasureBase();
    } else {
        return 0;
    }
}

//---------------------------------------------------------
//   findMeasureBase
//---------------------------------------------------------

const MeasureBase* EngravingItem::findMeasureBase() const
{
    EngravingItem* e = const_cast<EngravingItem*>(this);
    return e->findMeasureBase();
}

//---------------------------------------------------------
//   undoSetColor
//---------------------------------------------------------

void EngravingItem::undoSetColor(const mu::draw::Color& c)
{
    undoChangeProperty(Pid::COLOR, PropertyValue::fromValue(c));
}

//---------------------------------------------------------
//   undoSetVisible
//---------------------------------------------------------

void EngravingItem::undoSetVisible(bool v)
{
    undoChangeProperty(Pid::VISIBLE, v);
}

void EngravingItem::undoAddElement(EngravingItem* element)
{
    score()->undoAddElement(element);
}

//---------------------------------------------------------
//   drawSymbol
//---------------------------------------------------------

void EngravingItem::drawSymbol(SymId id, mu::draw::Painter* p, const mu::PointF& o, qreal scale) const
{
    score()->scoreFont()->draw(id, p, magS() * scale, o);
}

void EngravingItem::drawSymbol(SymId id, mu::draw::Painter* p, const mu::PointF& o, int n) const
{
    score()->scoreFont()->draw(id, p, magS(), o, n);
}

void EngravingItem::drawSymbols(const SymIdList& symbols, mu::draw::Painter* p, const PointF& o, qreal scale) const
{
    score()->scoreFont()->draw(symbols, p, magS() * scale, o);
}

void EngravingItem::drawSymbols(const SymIdList& symbols, mu::draw::Painter* p, const PointF& o, const SizeF& scale) const
{
    score()->scoreFont()->draw(symbols, p, SizeF(magS() * scale), PointF(o));
}

//---------------------------------------------------------
//   symHeight
//---------------------------------------------------------

qreal EngravingItem::symHeight(SymId id) const
{
    return score()->scoreFont()->height(id, magS());
}

//---------------------------------------------------------
//   symWidth
//---------------------------------------------------------

qreal EngravingItem::symWidth(SymId id) const
{
    return score()->scoreFont()->width(id, magS());
}

qreal EngravingItem::symWidth(const SymIdList& symbols) const
{
    return score()->scoreFont()->width(symbols, magS());
}

//---------------------------------------------------------
//   symAdvance
//---------------------------------------------------------

qreal EngravingItem::symAdvance(SymId id) const
{
    return score()->scoreFont()->advance(id, magS());
}

//---------------------------------------------------------
//   symBbox
//---------------------------------------------------------

RectF EngravingItem::symBbox(SymId id) const
{
    return score()->scoreFont()->bbox(id, magS());
}

RectF EngravingItem::symBbox(const SymIdList& symbols) const
{
    return score()->scoreFont()->bbox(symbols, magS());
}

//---------------------------------------------------------
//   symSmuflAnchor
//---------------------------------------------------------

PointF EngravingItem::symSmuflAnchor(SymId symId, SmuflAnchorId anchorId) const
{
    return score()->scoreFont()->smuflAnchor(symId, anchorId, magS());
}

//---------------------------------------------------------
//   symIsValid
//---------------------------------------------------------

bool EngravingItem::symIsValid(SymId id) const
{
    return score()->scoreFont()->isValid(id);
}

//---------------------------------------------------------
//   concertPitch
//---------------------------------------------------------

bool EngravingItem::concertPitch() const
{
    return score()->styleB(Sid::concertPitch);
}

//---------------------------------------------------------
//   nextElement
//   selects the next score element
//---------------------------------------------------------

EngravingItem* EngravingItem::nextElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().isEmpty()) {
        e = score()->selection().elements().first();
    }
    if (e) {
        switch (e->type()) {
        case ElementType::SEGMENT: {
            Segment* s = toSegment(e);
            return s->nextElement(staffIdx());
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(e);
            return m->nextElementStaff(staffIdx());
        }
        case ElementType::CLEF:
        case ElementType::KEYSIG:
        case ElementType::TIMESIG:
        case ElementType::BAR_LINE:
            return nextSegmentElement();
        default: {
            return e->parentItem()->nextElement();
        }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   prevElement
//   selects the previous score element
//---------------------------------------------------------

EngravingItem* EngravingItem::prevElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().isEmpty()) {
        e = score()->selection().elements().last();
    }
    if (e) {
        switch (e->type()) {
        case ElementType::SEGMENT: {
            Segment* s = toSegment(e);
            return s->prevElement(staffIdx());
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(e);
            return m->prevElementStaff(staffIdx());
        }
        case ElementType::CLEF:
        case ElementType::KEYSIG:
        case ElementType::TIMESIG:
        case ElementType::BAR_LINE:
            return prevSegmentElement();
        default: {
            return e->parentItem()->prevElement();
        }
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------------------------
//   nextSegmentElement
//   This function is used in for the next-element command to navigate between main elements
//   of segments. (Note, Rest, Clef, Time Signature, Key Signature, Barline, Ambitus, Breath, etc.)
//   The default implementation is to look for the first such element. After it is found each
//   element knows how to find the next one and overrides this method
//------------------------------------------------------------------------------------------

EngravingItem* EngravingItem::nextSegmentElement()
{
    EngravingItem* p = this;
    while (p) {
        switch (p->type()) {
        case ElementType::NOTE:
            if (toNote(p)->chord()->isGrace()) {
                break;
            }
            return p;
        case ElementType::REST:
        case ElementType::MMREST:
            return p;
        case ElementType::CHORD: {
            Chord* c = toChord(p);
            if (!c->isGrace()) {
                return c->notes().back();
            }
        }
        break;
        case ElementType::SEGMENT: {
            Segment* s = toSegment(p);
            return s->firstElement(staffIdx());
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(p);
            return m->nextElementStaff(staffIdx());
        }
        case ElementType::SYSTEM: {
            System* sys = toSystem(p);
            return sys->nextSegmentElement();
        }
        default:
            break;
        }
        p = p->parentItem();
    }
    return score()->firstElement();
}

//------------------------------------------------------------------------------------------
//   prevSegmentElement
//   This function is used in for the prev-element command to navigate between main elements
//   of segments. (Note, Rest, Clef, Time Signature, Key Signature, Barline, Ambitus, Breath, etc.)
//   The default implementation is to look for the first such element. After it is found each
//   element knows how to find the previous one and overrides this method
//------------------------------------------------------------------------------------------

EngravingItem* EngravingItem::prevSegmentElement()
{
    EngravingItem* p = this;
    while (p) {
        switch (p->type()) {
        case ElementType::NOTE:
            if (toNote(p)->chord()->isGrace()) {
                break;
            }
            return p;
        case ElementType::REST:
        case ElementType::MMREST:
            return p;
        case ElementType::CHORD: {
            Chord* c = toChord(p);
            if (!c->isGrace()) {
                return c->notes().front();
            }
        }
        break;
        case ElementType::SEGMENT: {
            Segment* s = toSegment(p);
            return s->lastElement(staffIdx());
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(p);
            return m->prevElementStaff(staffIdx());
        }
        case ElementType::SYSTEM: {
            System* sys = toSystem(p);
            return sys->prevSegmentElement();
        }
        default:
            break;
        }
        p = p->parentItem();
    }
    return score()->firstElement();
}

mu::engraving::AccessibleItem* EngravingItem::accessible() const
{
    return m_accessible;
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString EngravingItem::accessibleInfo() const
{
    return EngravingItem::typeUserName();
}

//---------------------------------------------------------
//   nextGrip
//---------------------------------------------------------

bool EngravingItem::nextGrip(EditData& ed) const
{
    int i = int(ed.curGrip) + 1;
    if (i >= ed.grips) {
        ed.curGrip = Grip(0);
        return false;
    }
    ed.curGrip = Grip(i);
    return true;
}

//---------------------------------------------------------
//   prevGrip
//---------------------------------------------------------

bool EngravingItem::prevGrip(EditData& ed) const
{
    int i = int(ed.curGrip) - 1;
    if (i < 0) {
        ed.curGrip = Grip(ed.grips - 1);
        return false;
    }
    ed.curGrip = Grip(i);
    return true;
}

//---------------------------------------------------------
//   isUserModified
//    Check if this element was modified by user and
//    therefore must be saved.
//---------------------------------------------------------

bool EngravingItem::isUserModified() const
{
    for (const StyledProperty& spp : *styledProperties()) {
        Pid pid = spp.pid;
        PropertyValue val = getProperty(pid);
        PropertyValue defaultValue = propertyDefault(pid);

        if (propertyType(pid) == P_TYPE::MILLIMETRE) {
            if (qAbs(val.value<Millimetre>() - defaultValue.value<Millimetre>()) > 0.0001) {         // we don’t care spatium diffs that small
                return true;
            }
        } else {
            if (getProperty(pid) != propertyDefault(pid)) {
                return true;
            }
        }
    }
    for (Pid p : { Pid::VISIBLE, Pid::OFFSET, Pid::COLOR, Pid::Z, Pid::AUTOPLACE }) {
        if (getProperty(p) != propertyDefault(p)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void EngravingItem::triggerLayout() const
{
    if (explicitParent()) {
        score()->setLayout(tick(), staffIdx(), this);
    }
}

//---------------------------------------------------------
//   triggerLayoutAll
//---------------------------------------------------------

void EngravingItem::triggerLayoutAll() const
{
    if (explicitParent()) {
        score()->setLayoutAll(staffIdx(), this);
    }
}

//---------------------------------------------------------
//   control
//---------------------------------------------------------

bool EditData::control(bool textEditing) const
{
    if (textEditing) {
        return modifiers & CONTROL_MODIFIER;
    } else {
        return modifiers & Qt::ControlModifier;
    }
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void EditData::clear()
{
    *this = EditData(view_);
}

//---------------------------------------------------------
//   getData
//---------------------------------------------------------

std::shared_ptr<ElementEditData> EditData::getData(const EngravingItem* e) const
{
    for (std::shared_ptr<ElementEditData> ed : data) {
        if (ed->e == e) {
            return ed;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   addData
//---------------------------------------------------------

void EditData::addData(std::shared_ptr<ElementEditData> ed)
{
    data.push_back(ed);
}

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void EngravingItem::drawEditMode(mu::draw::Painter* p, EditData& ed, qreal /*currentViewScaling*/)
{
    using namespace mu::draw;
    Pen pen(engravingConfiguration()->defaultColor(), 0.0);
    p->setPen(pen);
    for (int i = 0; i < ed.grips; ++i) {
        if (Grip(i) == ed.curGrip) {
            p->setBrush(engravingConfiguration()->formattingMarksColor());
        } else {
            p->setBrush(BrushStyle::NoBrush);
        }
        p->drawRect(ed.grip[i]);
    }
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void EngravingItem::startDrag(EditData& ed)
{
    if (!isMovable()) {
        return;
    }
    std::shared_ptr<ElementEditData> eed = std::make_shared<ElementEditData>();
    eed->e = this;
    eed->pushProperty(Pid::OFFSET);
    eed->pushProperty(Pid::AUTOPLACE);
    eed->initOffset = offset();
    ed.addData(eed);
    if (ed.modifiers & Qt::AltModifier) {
        setAutoplace(false);
    }
}

//---------------------------------------------------------
//   drag
///   Return update Rect relative to canvas.
//---------------------------------------------------------

RectF EngravingItem::drag(EditData& ed)
{
    if (!isMovable()) {
        return RectF();
    }

    const RectF r0(canvasBoundingRect());

    const ElementEditDataPtr eed = ed.getData(this);

    const PointF offset0 = ed.moveDelta + eed->initOffset;
    qreal x = offset0.x();
    qreal y = offset0.y();

    qreal _spatium = spatium();
    if (ed.hRaster) {
        qreal hRaster = _spatium / MScore::hRaster();
        int n = lrint(x / hRaster);
        x = hRaster * n;
    }
    if (ed.vRaster) {
        qreal vRaster = _spatium / MScore::vRaster();
        int n = lrint(y / vRaster);
        y = vRaster * n;
    }

    setOffset(PointF(x, y));
    setOffsetChanged(true);
//      setGenerated(false);

    if (isTextBase()) {           // TODO: check for other types
        //
        // restrict move to page boundaries
        //
        const RectF r(canvasBoundingRect());
        Page* p = 0;
        EngravingItem* e = this;
        while (e) {
            if (e->isPage()) {
                p = toPage(e);
                break;
            }
            e = e->parentItem();
        }
        if (p) {
            bool move = false;
            RectF pr(p->canvasBoundingRect());
            if (r.right() > pr.right()) {
                x -= r.right() - pr.right();
                move = true;
            } else if (r.left() < pr.left()) {
                x += pr.left() - r.left();
                move = true;
            }
            if (r.bottom() > pr.bottom()) {
                y -= r.bottom() - pr.bottom();
                move = true;
            } else if (r.top() < pr.top()) {
                y += pr.top() - r.top();
                move = true;
            }
            if (move) {
                setOffset(PointF(x, y));
            }
        }
    }
    return canvasBoundingRect().united(r0);
}

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void EngravingItem::endDrag(EditData& ed)
{
    if (!isMovable()) {
        return;
    }
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }
    for (const PropertyData& pd : qAsConst(eed->propertyData)) {
        setPropertyFlags(pd.id, pd.f);     // reset initial property flags state
        PropertyFlags f = pd.f;
        if (f == PropertyFlags::STYLED) {
            f = PropertyFlags::UNSTYLED;
        }
        score()->undoPropertyChanged(this, pd.id, pd.data, f);
        setGenerated(false);
    }
}

//---------------------------------------------------------
//   genericDragAnchorLines
//---------------------------------------------------------

QVector<LineF> EngravingItem::genericDragAnchorLines() const
{
    qreal xp = 0.0;
    for (EngravingItem* e = parentItem(); e; e = e->parentItem()) {
        xp += e->x();
    }
    qreal yp;
    if (explicitParent()->isSegment() || explicitParent()->isMeasure()) {
        Measure* meas = explicitParent()->isSegment() ? toSegment(explicitParent())->measure() : toMeasure(explicitParent());
        System* system = meas->system();
        const int stIdx = staffIdxOrNextVisible();
        if (stIdx < 0) {
            return { LineF() };
        }
        yp = system ? system->staffCanvasYpage(stIdx) : 0.0;
        if (placement() == PlacementV::BELOW) {
            yp += system ? system->staff(stIdx)->bbox().height() : 0.0;
        }
        //adjust anchor Y positions to staffType offset
        if (staff()) {
            yp += staff()->staffTypeForElement(this)->yoffset().val() * spatium();
        }
    } else {
        yp = parentItem()->canvasPos().y();
    }
    PointF p1(xp, yp);
    LineF anchorLine(p1, canvasPos());
    return { anchorLine };
}

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void EngravingItem::updateGrips(EditData& ed) const
{
    const auto positions(gripsPositions(ed));
    const size_t ngrips = positions.size();
    for (int i = 0; i < int(ngrips); ++i) {
        ed.grip[i].translate(positions[i]);
    }
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void EngravingItem::startEdit(EditData& ed)
{
    std::shared_ptr<ElementEditData> elementData = std::make_shared<ElementEditData>();
    elementData->e = this;
    ed.addData(elementData);
}

//---------------------------------------------------------
//   isEditAllowed
//---------------------------------------------------------

bool EngravingItem::isEditAllowed(EditData& ed) const
{
    return ed.key == Qt::Key_Home;
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool EngravingItem::edit(EditData& ed)
{
    if (ed.key == Qt::Key_Home) {
        setOffset(PointF());
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void EngravingItem::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        eed = std::make_shared<ElementEditData>();
        eed->e = this;
        ed.addData(eed);
    }
    eed->pushProperty(Pid::OFFSET);
    eed->pushProperty(Pid::AUTOPLACE);
    if (ed.modifiers & Qt::AltModifier) {
        setAutoplace(false);
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void EngravingItem::editDrag(EditData& ed)
{
    score()->addRefresh(canvasBoundingRect());
    setOffset(offset() + ed.delta);
    setOffsetChanged(true);
    score()->addRefresh(canvasBoundingRect());
}

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void EngravingItem::endEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    bool changed = false;
    if (eed) {
        for (const PropertyData& pd : qAsConst(eed->propertyData)) {
            setPropertyFlags(pd.id, pd.f);       // reset initial property flags state
            PropertyFlags f = pd.f;
            if (f == PropertyFlags::STYLED) {
                f = PropertyFlags::UNSTYLED;
            }
            if (score()->undoPropertyChanged(this, pd.id, pd.data, f)) {
                changed = true;
            }
        }
        eed->propertyData.clear();
    }
    if (changed) {
        undoChangeProperty(Pid::GENERATED, false);
    }
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void EngravingItem::endEdit(EditData&)
{
}

//---------------------------------------------------------
//   styleP
//---------------------------------------------------------

qreal EngravingItem::styleP(Sid idx) const
{
    return score()->styleMM(idx);
}

bool EngravingItem::colorsInversionEnabled() const
{
    return m_colorsInversionEnabled;
}

void EngravingItem::setColorsInverionEnabled(bool enabled)
{
    m_colorsInversionEnabled = enabled;
}

//---------------------------------------------------------
//   setOffsetChanged
//---------------------------------------------------------

void EngravingItem::setOffsetChanged(bool v, bool absolute, const PointF& diff)
{
    if (v) {
        _offsetChanged = absolute ? OffsetChange::ABSOLUTE_OFFSET : OffsetChange::RELATIVE_OFFSET;
    } else {
        _offsetChanged = OffsetChange::NONE;
    }
    _changedPos = pos() + diff;
}

//---------------------------------------------------------
//   rebaseOffset
//    calculates new offset for moved elements
//    for drag & other actions that result in absolute position, apply the new offset
//    for nudge & other actions that result in relative adjustment, return the vertical difference
//---------------------------------------------------------

qreal EngravingItem::rebaseOffset(bool nox)
{
    PointF off = offset();
    PointF p = _changedPos - pos();
    if (nox) {
        p.rx() = 0.0;
    }
    //OffsetChange saveChangedValue = _offsetChanged;

    bool staffRelative = staff() && explicitParent() && !(explicitParent()->isNote() || explicitParent()->isRest());
    if (staffRelative && propertyFlags(Pid::PLACEMENT) != PropertyFlags::NOSTYLE) {
        // check if flipped
        // TODO: elements that support PLACEMENT but not as a styled property (add supportsPlacement() method?)
        // TODO: refactor to take advantage of existing cmdFlip() algorithms
        // TODO: adjustPlacement() (from read206.cpp) on read for 3.0 as well
        RectF r = bbox().translated(_changedPos);
        qreal staffHeight = staff()->height();
        EngravingItem* e = isSpannerSegment() ? toSpannerSegment(this)->spanner() : this;
        bool multi = e->isSpanner() && toSpanner(e)->spannerSegments().size() > 1;
        bool above = e->placeAbove();
        bool flipped = above ? r.top() > staffHeight : r.bottom() < 0.0;
        if (flipped && !multi) {
            off.ry() += above ? -staffHeight : staffHeight;
            undoChangeProperty(Pid::OFFSET, PropertyValue::fromValue(off + p));
            _offsetChanged = OffsetChange::ABSOLUTE_OFFSET;             //saveChangedValue;
            rypos() += above ? staffHeight : -staffHeight;
            PropertyFlags pf = e->propertyFlags(Pid::PLACEMENT);
            if (pf == PropertyFlags::STYLED) {
                pf = PropertyFlags::UNSTYLED;
            }
            PlacementV place = above ? PlacementV::BELOW : PlacementV::ABOVE;
            e->undoChangeProperty(Pid::PLACEMENT, int(place), pf);
            undoResetProperty(Pid::MIN_DISTANCE);
            return 0.0;
        }
    }

    if (offsetChanged() == OffsetChange::ABSOLUTE_OFFSET) {
        undoChangeProperty(Pid::OFFSET, PropertyValue::fromValue(off + p));
        _offsetChanged = OffsetChange::ABSOLUTE_OFFSET;                 //saveChangedValue;
        // allow autoplace to manage min distance even when not needed
        undoResetProperty(Pid::MIN_DISTANCE);
        return 0.0;
    }

    // allow autoplace to manage min distance even when not needed
    undoResetProperty(Pid::MIN_DISTANCE);
    return p.y();
}

//---------------------------------------------------------
//   rebaseMinDistance
//    calculates new minDistance for moved elements
//    if necessary, also rebases offset
//    updates md, yd
//    returns true if shape needs to be rebased
//---------------------------------------------------------

bool EngravingItem::rebaseMinDistance(qreal& md, qreal& yd, qreal sp, qreal rebase, bool above, bool fix)
{
    bool rc = false;
    PropertyFlags pf = propertyFlags(Pid::MIN_DISTANCE);
    if (pf == PropertyFlags::STYLED) {
        pf = PropertyFlags::UNSTYLED;
    }
    qreal adjustedY = pos().y() + yd;
    qreal diff = _changedPos.y() - adjustedY;
    if (fix) {
        undoChangeProperty(Pid::MIN_DISTANCE, -999.0, pf);
        yd = 0.0;
    } else if (!isStyled(Pid::MIN_DISTANCE)) {
        md = (above ? md + yd : md - yd) / sp;
        undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
        yd += diff;
    } else {
        // min distance still styled
        // user apparently moved element into skyline
        // but perhaps not really, if performing a relative adjustment
        if (_offsetChanged == OffsetChange::RELATIVE_OFFSET) {
            // relative movement (cursor): fix only if moving vertically into direction of skyline
            if ((above && diff > 0.0) || (!above && diff < 0.0)) {
                // rebase offset
                PointF p = offset();
                p.ry() += rebase;
                undoChangeProperty(Pid::OFFSET, p);
                md = (above ? md - diff : md + diff) / sp;
                undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
                rc = true;
                yd = 0.0;
            }
        } else {
            // absolute movement (drag): fix unconditionally
            md = (above ? md + yd : md - yd) / sp;
            undoChangeProperty(Pid::MIN_DISTANCE, md, pf);
            yd = 0.0;
        }
    }
    return rc;
}

//---------------------------------------------------------
//   autoplaceSegmentElement
//---------------------------------------------------------

void EngravingItem::autoplaceSegmentElement(bool above, bool add)
{
    // rebase vertical offset on drag
    qreal rebase = 0.0;
    if (offsetChanged() != OffsetChange::NONE) {
        rebase = rebaseOffset();
    }

    if (autoplace() && explicitParent()) {
        Segment* s = toSegment(explicitParent());
        Measure* m = s->measure();

        qreal sp = score()->spatium();
        int si = staffIdxOrNextVisible();

        // if there's no good staff for this object, obliterate it
        _skipDraw = (si == -1);
        setSelectable(!_skipDraw);
        if (_skipDraw) {
            return;
        }

        qreal mag = staff()->staffMag(this);
        sp *= mag;
        qreal minDistance = _minDistance.val() * sp;

        SysStaff* ss = m->system()->staff(si);
        RectF r = bbox().translated(m->pos() + s->pos() + pos());

        // Adjust bbox Y pos for staffType offset
        if (staffType()) {
            qreal stYOffset = staffType()->yoffset().val() * sp;
            r.translate(0.0, stYOffset);
        }

        SkylineLine sk(!above);
        qreal d;
        if (above) {
            sk.add(r.x(), r.bottom(), r.width());
            d = sk.minDistance(ss->skyline().north());
        } else {
            sk.add(r.x(), r.top(), r.width());
            d = ss->skyline().south().minDistance(sk);
        }

        if (d > -minDistance) {
            qreal yd = d + minDistance;
            if (above) {
                yd *= -1.0;
            }
            if (offsetChanged() != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                if (rebaseMinDistance(minDistance, yd, sp, rebase, above, inStaff)) {
                    r.translate(0.0, rebase);
                }
            }
            rypos() += yd;
            r.translate(PointF(0.0, yd));
        }
        if (add && addToSkyline()) {
            ss->skyline().add(r);
        }
    }
    setOffsetChanged(false);
}

//---------------------------------------------------------
//   autoplaceMeasureElement
//---------------------------------------------------------

void EngravingItem::autoplaceMeasureElement(bool above, bool add)
{
    // rebase vertical offset on drag
    qreal rebase = 0.0;
    if (offsetChanged() != OffsetChange::NONE) {
        rebase = rebaseOffset();
    }

    if (autoplace() && explicitParent()) {
        Measure* m = toMeasure(explicitParent());
        int si = staffIdxOrNextVisible();

        // if there's no good staff for this object, obliterate it
        _skipDraw = (si == -1);
        setSelectable(!_skipDraw);
        if (_skipDraw) {
            return;
        }

        qreal sp = score()->spatium();
        qreal minDistance = _minDistance.val() * sp;

        SysStaff* ss = m->system()->staff(si);
        // shape rather than bbox is good for tuplets especially
        Shape sh = shape().translated(m->pos() + pos());

        SkylineLine sk(!above);
        qreal d;
        if (above) {
            sk.add(sh);
            d = sk.minDistance(ss->skyline().north());
        } else {
            sk.add(sh);
            d = ss->skyline().south().minDistance(sk);
        }
        if (d > -minDistance) {
            qreal yd = d + minDistance;
            if (above) {
                yd *= -1.0;
            }
            if (offsetChanged() != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                bool inStaff = above ? sh.bottom() + rebase > 0.0 : sh.top() + rebase < staff()->height();
                if (rebaseMinDistance(minDistance, yd, sp, rebase, above, inStaff)) {
                    sh.translateY(rebase);
                }
            }
            rypos() += yd;
            sh.translateY(yd);
        }
        if (add && addToSkyline()) {
            ss->skyline().add(sh);
        }
    }
    setOffsetChanged(false);
}

bool EngravingItem::selected() const
{
    return flag(ElementFlag::SELECTED);
}

void EngravingItem::setSelected(bool f)
{
    setFlag(ElementFlag::SELECTED, f);

    if (f) {
        if (m_accessible) {
            AccessibleRoot* accRoot = score()->rootItem()->accessible()->accessibleRoot();
            if (accRoot && accRoot->registered()) {
                accRoot->setFocusedElement(nullptr);
            }

            AccessibleRoot* dummyAccRoot = score()->dummy()->rootItem()->accessible()->accessibleRoot();
            if (dummyAccRoot && dummyAccRoot->registered()) {
                dummyAccRoot->setFocusedElement(nullptr);
            }

            AccessibleRoot* currAccRoot = m_accessible->accessibleRoot();
            if (currAccRoot && currAccRoot->registered()) {
                currAccRoot->setFocusedElement(m_accessible);
            }
        }
    }
}
}
