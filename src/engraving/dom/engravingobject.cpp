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

#include "engravingobject.h"

#include "global/containers.h"

#include "style/textstyle.h"
#include "types/typesconv.h"

#include "bracketItem.h"
#include "linkedobjects.h"
#include "masterscore.h"
#include "score.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
ElementStyle const EngravingObject::EMPTY_STYLE;

EngravingObject::EngravingObject(const ElementType& type, EngravingObject* parent)
    : m_type(type)
{
    IF_ASSERT_FAILED(this != parent) {
        return;
    }

    if (type != ElementType::SCORE) {
        assert(parent);
    }

    doSetParent(parent);
    if (m_parent) {
        doSetScore(m_parent->score());
    }

    if (type == ElementType::SCORE) {
        m_score = static_cast<Score*>(this);
    }

    // reg to debug
    if (type != ElementType::SCORE) {
        if (m_score && m_score->elementsProvider()) {
            m_score->elementsProvider()->reg(this);
        }
    }
}

EngravingObject::EngravingObject(const EngravingObject& se)
{
    m_type = se.m_type;
    doSetParent(se.m_parent);
    m_score = se.m_score;
    m_isParentExplicitlySet = se.m_isParentExplicitlySet;
    m_elementStyle = se.m_elementStyle;
    if (m_elementStyle) {
        size_t n = m_elementStyle->size();
        m_propertyFlagsList = new PropertyFlags[n];
        for (size_t i = 0; i < n; ++i) {
            m_propertyFlagsList[i] = se.m_propertyFlagsList[i];
        }
    }
    m_links = 0;

    // reg to debug
    if (m_type != ElementType::SCORE) {
        if (m_score && m_score->elementsProvider()) {
            m_score->elementsProvider()->reg(this);
        }
    }
}

//---------------------------------------------------------
//   ~ScoreElement
//---------------------------------------------------------

EngravingObject::~EngravingObject()
{
    if (m_parent) {
        m_parent->removeChild(this);
    }

    {
        bool isPaletteScore = score()->isPaletteScore();
        bool canMoveToDummy = !this->isType(ElementType::ROOT_ITEM)
                              && !this->isType(ElementType::DUMMY)
                              && !this->isType(ElementType::SCORE)
                              && score()->rootItem() && score()->rootItem()->dummy();

        // copy because moveToDummy might modify children
        EngravingObjectList children = m_children;
        for (EngravingObject* c : children) {
            if (canMoveToDummy) {
                c->moveToDummy();
            } else if (!isPaletteScore) {
                delete c;
            } else {
                c->m_parent = nullptr;
            }
        }

        m_children.clear();
    }

    if (score() && score()->elementsProvider()) {
        score()->elementsProvider()->unreg(this);
    }

    if (m_links) {
        m_links->remove(this);
        if (m_links->empty()) {
            delete m_links;
            m_links = 0;
        }
    }
    delete[] m_propertyFlagsList;
}

void EngravingObject::doSetParent(EngravingObject* p)
{
    if (m_parent == p) {
        return;
    }

    IF_ASSERT_FAILED(p != this) {
        return;
    }

    if (m_parent) {
        m_parent->removeChild(this);
    }

    m_parent = p;

    if (m_parent) {
        m_parent->addChild(this);
    }
}

void EngravingObject::doSetScore(Score* sc)
{
    if (m_score == sc) {
        return;
    }

    m_score = sc;

    for (EngravingObject* ch : m_children) {
        ch->doSetScore(sc);
    }
}

void EngravingObject::moveToDummy()
{
    Score* sc = score();
    if (sc) {
        if (sc->dummy() && sc->dummy() != this) {
            setParent(sc->dummy());
        }
    }
}

void EngravingObject::setScore(Score* s)
{
    doSetScore(s);

    if (!m_parent) {
        return;
    }

    if (m_parent->score() == s) {
        return;
    }

    if (m_parent->isType(ElementType::DUMMY)) {
        moveToDummy();
    } else if (m_parent->isType(ElementType::SCORE)) {
        setParent(s);
    }
}

void EngravingObject::addChild(EngravingObject* o)
{
    IF_ASSERT_FAILED(o != this) {
        return;
    }

    m_children.push_back(o);
}

void EngravingObject::removeChild(EngravingObject* o)
{
    IF_ASSERT_FAILED(o->m_parent == this) {
        return;
    }
    o->m_parent = nullptr;
    muse::remove(m_children, o);
}

EngravingObject* EngravingObject::parent() const
{
    return m_parent;
}

EngravingObject* EngravingObject::explicitParent() const
{
    if (!m_isParentExplicitlySet) {
        return nullptr;
    }
    return m_parent;
}

void EngravingObject::setParent(EngravingObject* p)
{
    setParentInternal(p);
}

void EngravingObject::setParentInternal(EngravingObject* p)
{
    IF_ASSERT_FAILED(this != p) {
        return;
    }

    if (!p) {
        moveToDummy();
        return;
    }

    doSetParent(p);
    if (m_parent) {
        doSetScore(m_parent->score());
    }

    if (p && !p->isType(ElementType::DUMMY)) {
        m_isParentExplicitlySet = true;
    } else {
        m_isParentExplicitlySet = false;
    }
}

void EngravingObject::resetExplicitParent()
{
    m_isParentExplicitlySet = false;
}

Score* EngravingObject::score() const
{
    return m_score;
}

MasterScore* EngravingObject::masterScore() const
{
    return score()->masterScore();
}

bool EngravingObject::onSameScore(const EngravingObject* other) const
{
    return this->score() == other->score();
}

const MStyle& EngravingObject::style() const
{
    return score()->style();
}

//---------------------------------------------------------
//   scanElements
/// Recursively apply scanElements to all children.
/// See also EngravingItem::scanElements.
//---------------------------------------------------------

void EngravingObject::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (EngravingObject* child : scanChildren()) {
        child->scanElements(data, func, all);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue EngravingObject::propertyDefault(Pid pid) const
{
    Sid sid = getPropertyStyle(pid);
    if (sid != Sid::NOSTYLE) {
        return styleValue(pid, sid);
    }
    //      LOGD("<%s>(%d) not found in <%s>", propertyQmlName(pid), int(pid), typeName());
    return PropertyValue();
}

//---------------------------------------------------------
//   initElementStyle
//---------------------------------------------------------

void EngravingObject::initElementStyle(const ElementStyle* ss)
{
    m_elementStyle = ss;
    size_t n      = m_elementStyle->size();
    delete[] m_propertyFlagsList;
    m_propertyFlagsList = new PropertyFlags[n];
    for (size_t i = 0; i < n; ++i) {
        m_propertyFlagsList[i] = PropertyFlags::STYLED;
    }
    for (const StyledProperty& spp : *m_elementStyle) {
        //            setProperty(spp.pid, styleValue(spp.pid, spp.sid));
        setProperty(spp.pid, styleValue(spp.pid, getPropertyStyle(spp.pid)));
    }
}

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void EngravingObject::resetProperty(Pid pid)
{
    PropertyValue v = propertyDefault(pid);
    if (v.isValid()) {
        setProperty(pid, v);
        PropertyFlags p = propertyFlags(pid);
        if (p == PropertyFlags::UNSTYLED) {
            setPropertyFlags(pid, PropertyFlags::STYLED);
        }
    }
}

//---------------------------------------------------------
//   undoResetProperty
//---------------------------------------------------------

void EngravingObject::undoResetProperty(Pid id)
{
    PropertyFlags f = propertyFlags(id);
    if (f == PropertyFlags::UNSTYLED) {
        f = PropertyFlags::STYLED;
    }
    undoChangeProperty(id, propertyDefault(id), f);
}

//---------------------------------------------------------
//   isStyled
//---------------------------------------------------------

bool EngravingObject::isStyled(Pid pid) const
{
    PropertyFlags f = propertyFlags(pid);
    return f == PropertyFlags::STYLED;
}

//---------------------------------------------------------
//   changeProperty
//---------------------------------------------------------

static void changeProperty(EngravingObject* e, Pid t, const PropertyValue& st, PropertyFlags ps)
{
    if (e->getProperty(t) != st || e->propertyFlags(t) != ps) {
        if (e->isBracketItem()) {
            BracketItem* bi = toBracketItem(e);
            e->score()->undo(new ChangeBracketProperty(bi->staff(), bi->column(), t, st, ps));
        } else {
            e->score()->undo(new ChangeProperty(e, t, st, ps));
        }
    }
}

//---------------------------------------------------------
//   changeProperties
//---------------------------------------------------------

static void changeProperties(EngravingObject* object, Pid propertyId, const PropertyValue& propertyValue, PropertyFlags propertyFlag)
{
    const std::list<EngravingObject*> linkList = object->linkListForPropertyPropagation();
    for (EngravingObject* linkedObject : linkList) {
        if (linkedObject == object) {
            changeProperty(object, propertyId, propertyValue, propertyFlag);
            continue;
        }

        if (!object->isEngravingItem() || !linkedObject->isEngravingItem()) {
            continue;
        }

        EngravingItem* item = toEngravingItem(object);
        EngravingItem* linkedItem = toEngravingItem(linkedObject);
        PropertyPropagation propertyPropagate = item->propertyPropagation(linkedItem, propertyId);
        switch (propertyPropagate) {
        case PropertyPropagation::NONE:
            break;
        case PropertyPropagation::PROPAGATE:
            changeProperty(linkedObject, propertyId, propertyValue, propertyFlag);
            break;
        case PropertyPropagation::UNLINK:
            item->unlinkPropertyFromMaster(propertyId);
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void EngravingObject::undoChangeProperty(Pid id, const PropertyValue& v)
{
    undoChangeProperty(id, v, propertyFlags(id));
}

void EngravingObject::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if ((getProperty(id) == v) && (propertyFlags(id) == ps)) {
        return;
    }
    if (id == Pid::PLACEMENT || id == Pid::HAIRPIN_TYPE) {
        // first set property, then set offset for above/below if styled
        changeProperties(this, id, v, ps);

        if (isStyled(Pid::OFFSET)) {
            // TODO: maybe it just makes more sense to do this in EngravingItem::undoChangeProperty,
            // but some of the overrides call ScoreElement explicitly
            double sp;
            if (isEngravingItem()) {
                sp = toEngravingItem(this)->spatium();
            } else {
                sp = style().spatium();
            }
            setProperty(Pid::OFFSET, style().styleV(getPropertyStyle(Pid::OFFSET)).value<PointF>() * sp);
            EngravingItem* e = toEngravingItem(this);
            e->setOffsetChanged(false);
        }
    } else if (id == Pid::TEXT_STYLE) {
        //
        // change a list of properties
        //
        auto l = textStyle(v.value<TextStyleType>());
        // Change to ElementStyle defaults
        for (const auto& p : *l) {
            if (p.sid == Sid::NOSTYLE) {
                break;
            }
            changeProperties(this, p.pid, style().styleV(p.sid), PropertyFlags::STYLED);
        }
    } else if (id == Pid::OFFSET) {
        // TODO: do this in caller?
        if (isEngravingItem()) {
            EngravingItem* e = toEngravingItem(this);
            if (e->offset() != v.value<PointF>()) {
                e->setOffsetChanged(true, false, v.value<PointF>() - e->offset());
            }
        }
    } else if (id == Pid::EXCLUDE_FROM_OTHER_PARTS) {
        if (isEngravingItem() && getProperty(Pid::EXCLUDE_FROM_OTHER_PARTS) != v) {
            EngravingItem* delegate = toEngravingItem(this)->propertyDelegate(id);
            if (delegate) {
                delegate->manageExclusionFromParts(v.toBool());
            } else {
                toEngravingItem(this)->manageExclusionFromParts(v.toBool());
            }
        }
    } else if (id == Pid::VOICE_ASSIGNMENT) {
        if (v.value<VoiceAssignment>() != VoiceAssignment::CURRENT_VOICE_ONLY) {
            changeProperties(this, Pid::VOICE, 0, ps);
        }
    }
    changeProperties(this, id, v, ps);
    if (id != Pid::GENERATED) {
        changeProperties(this, Pid::GENERATED, false, PropertyFlags::NOSTYLE);
    }
}

//---------------------------------------------------------
//   undoPushProperty
//---------------------------------------------------------

void EngravingObject::undoPushProperty(Pid id)
{
    PropertyValue val = getProperty(id);
    score()->undoStack()->pushWithoutPerforming(new ChangeProperty(this, id, val));
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void EngravingObject::reset()
{
    for (const StyledProperty& spp : *styledProperties()) {
        undoResetProperty(spp.pid);
    }
}

//---------------------------------------------------------
//   linkTo
//    link this to element
//---------------------------------------------------------
void EngravingObject::linkTo(EngravingObject* element)
{
    assert(element != this);
    assert(!m_links);

    if (element->links()) {
        setLinks(element->m_links);
        assert(m_links->contains(element));
    } else {
        setLinks(new LinkedObjects());
        m_links->push_back(element);
        element->setLinks(m_links);
    }
    assert(!m_links->contains(this));
    m_links->push_back(this);
}

//---------------------------------------------------------
//   unlink
//---------------------------------------------------------

void EngravingObject::unlink()
{
    if (!m_links) {
        return;
    }

    assert(m_links->contains(this));
    m_links->remove(this);

    // if link list is empty, remove list
    if (m_links->size() <= 1) {
        if (!m_links->empty()) {
            m_links->front()->m_links = nullptr;
        }
        delete m_links;
    }
    m_links = 0;   // this element is not linked anymore
}

//---------------------------------------------------------
//   isLinked
///  return true if se is different and
///  linked to this element
//---------------------------------------------------------

bool EngravingObject::isLinked(EngravingObject* se) const
{
    if (se == this || !m_links) {
        return false;
    }

    if (se == nullptr) {
        return !m_links->empty() && m_links->mainElement() != this;
    }

    return m_links->contains(se);
}

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void EngravingObject::undoUnlink()
{
    if (m_links) {
        score()->undo(new Unlink(this));
    }
}

void EngravingObject::setLinks(LinkedObjects* le)
{
    m_links = le;
}

//---------------------------------------------------------
//   linkList
//---------------------------------------------------------

std::list<EngravingObject*> EngravingObject::linkList() const
{
    std::list<EngravingObject*> el;
    if (m_links) {
        el = *m_links;
    } else {
        el.push_back(const_cast<EngravingObject*>(this));
    }
    return el;
}

//---------------------------------------------------------
//   getPropertyFlagsIdx
//---------------------------------------------------------

int EngravingObject::getPropertyFlagsIdx(Pid id) const
{
    int i = 0;
    for (const StyledProperty& p : *m_elementStyle) {
        if (p.pid == id) {
            return i;
        }
        ++i;
    }
    return -1;
}

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

PropertyFlags EngravingObject::propertyFlags(Pid id) const
{
    static PropertyFlags f = PropertyFlags::NOSTYLE;

    int i = getPropertyFlagsIdx(id);
    if (i == -1) {
        return f;
    }
    return m_propertyFlagsList[i];
}

//---------------------------------------------------------
//   setPropertyFlags
//---------------------------------------------------------

void EngravingObject::setPropertyFlags(Pid id, PropertyFlags f)
{
    int i = getPropertyFlagsIdx(id);
    if (i == -1) {
        return;
    }
    m_propertyFlagsList[i] = f;
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid EngravingObject::getPropertyStyle(Pid id) const
{
    for (const StyledProperty& p : *m_elementStyle) {
        if (p.pid == id) {
            return p.sid;
        }
    }
    return Sid::NOSTYLE;
}

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void EngravingObject::styleChanged()
{
    for (const StyledProperty& spp : *m_elementStyle) {
        PropertyFlags f = propertyFlags(spp.pid);
        if (f == PropertyFlags::STYLED) {
            setProperty(spp.pid, styleValue(spp.pid, getPropertyStyle(spp.pid)));
        }
    }
}

const char* EngravingObject::typeName() const
{
    return TConv::toXml(type()).ascii();
}

TranslatableString EngravingObject::typeUserName() const
{
    return TConv::capitalizedUserName(type());
}

String EngravingObject::translatedTypeUserName() const
{
    return typeUserName().translated();
}

EID EngravingObject::eid() const
{
    return masterScore()->eidRegister()->EIDFromItem(this);
}

void EngravingObject::setEID(EID id) const
{
    masterScore()->eidRegister()->registerItemEID(id, this);
}

EID EngravingObject::assignNewEID() const
{
    return masterScore()->eidRegister()->newEIDForItem(this);
}

//---------------------------------------------------------
//   isSLineSegment
//---------------------------------------------------------

bool EngravingObject::isSLineSegment() const
{
    return isHairpinSegment() || isOttavaSegment() || isPedalSegment()
           || isTrillSegment() || isVoltaSegment() || isTextLineSegment()
           || isGlissandoSegment() || isLetRingSegment() || isVibratoSegment() || isPalmMuteSegment()
           || isGradualTempoChangeSegment();
}

//---------------------------------------------------------
//   isText
//---------------------------------------------------------

bool EngravingObject::isTextBase() const
{
    return type() == ElementType::TEXT
           || type() == ElementType::LYRICS
           || type() == ElementType::DYNAMIC
           || type() == ElementType::EXPRESSION
           || type() == ElementType::FINGERING
           || type() == ElementType::HARMONY
           || type() == ElementType::MARKER
           || type() == ElementType::JUMP
           || type() == ElementType::STAFF_TEXT
           || type() == ElementType::SYSTEM_TEXT
           || type() == ElementType::TRIPLET_FEEL
           || type() == ElementType::PLAY_COUNT_TEXT
           || type() == ElementType::PLAYTECH_ANNOTATION
           || type() == ElementType::CAPO
           || type() == ElementType::STRING_TUNINGS
           || type() == ElementType::REHEARSAL_MARK
           || type() == ElementType::INSTRUMENT_CHANGE
           || type() == ElementType::FIGURED_BASS
           || type() == ElementType::TEMPO_TEXT
           || type() == ElementType::INSTRUMENT_NAME
           || type() == ElementType::MEASURE_NUMBER
           || type() == ElementType::MMREST_RANGE
           || type() == ElementType::STICKING
           || type() == ElementType::HARP_DIAGRAM
           || type() == ElementType::GUITAR_BEND_TEXT
           || type() == ElementType::HAMMER_ON_PULL_OFF_TEXT;
}

//---------------------------------------------------------
//   styleValue
//---------------------------------------------------------

PropertyValue EngravingObject::styleValue(Pid pid, Sid sid) const
{
    switch (propertyType(pid)) {
    case P_TYPE::MILLIMETRE:
        return style().styleMM(sid);
    case P_TYPE::POINT: {
        PointF val = style().styleV(sid).value<PointF>();
        if (offsetIsSpatiumDependent()) {
            val *= style().spatium();
            if (isEngravingItem()) {
                const EngravingItem* e = toEngravingItem(this);
                if (e->staff() && !e->systemFlag()) {
                    val *= e->staff()->staffMag(e->tick());
                }
            }
        } else {
            val *= DPMM;
        }
        return val;
    }
    default:
        return style().styleV(sid);
    }
}
}
