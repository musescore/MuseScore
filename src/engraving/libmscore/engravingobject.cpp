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

#include "engravingobject.h"

#include <iterator>
#include <unordered_set>

#include "rw/xml.h"
#include "style/textstyle.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "bracketItem.h"
#include "factory.h"
#include "linkedobjects.h"
#include "masterscore.h"
#include "score.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
ElementStyle const EngravingObject::emptyStyle;

EngravingObject* EngravingObjectList::at(size_t i) const
{
    return *std::next(begin(), i);
}

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

    if (elementsProvider()) {
        elementsProvider()->reg(this);
    }
}

EngravingObject::EngravingObject(const EngravingObject& se)
{
    m_type = se.m_type;
    doSetParent(se.m_parent);
    m_score = se.m_score;
    m_isParentExplicitlySet = se.m_isParentExplicitlySet;
    _elementStyle = se._elementStyle;
    if (_elementStyle) {
        size_t n = _elementStyle->size();
        _propertyFlagsList = new PropertyFlags[n];
        for (size_t i = 0; i < n; ++i) {
            _propertyFlagsList[i] = se._propertyFlagsList[i];
        }
    }
    _links = 0;

    if (elementsProvider()) {
        elementsProvider()->reg(this);
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

    if (!this->isType(ElementType::ROOT_ITEM)
        && !this->isType(ElementType::DUMMY)
        && !this->isType(ElementType::SCORE)) {
        EngravingObjectList children = m_children;
        for (EngravingObject* c : children) {
            c->m_parent = nullptr;
            c->moveToDummy();
        }
    } else {
        bool isPaletteScore = score()->isPaletteScore();
        for (EngravingObject* c : m_children) {
            c->m_parent = nullptr;
            if (!isPaletteScore) {
                delete c;
            }
        }
        m_children.clear();
    }

    if (elementsProvider()) {
        elementsProvider()->unreg(this);
    }

    if (_links) {
        _links->remove(this);
        if (_links->empty()) {
            delete _links;
            _links = 0;
        }
    }
    delete[] _propertyFlagsList;
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
    m_children.remove(o);
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

const MStyle* EngravingObject::style() const
{
    return &score()->style();
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
    _elementStyle = ss;
    size_t n      = _elementStyle->size();
    delete[] _propertyFlagsList;
    _propertyFlagsList = new PropertyFlags[n];
    for (size_t i = 0; i < n; ++i) {
        _propertyFlagsList[i] = PropertyFlags::STYLED;
    }
    for (const StyledProperty& spp : *_elementStyle) {
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

static void changeProperties(EngravingObject* e, Pid t, const PropertyValue& st, PropertyFlags ps)
{
    if (propertyLink(t)) {
        for (EngravingObject* ee : e->linkList()) {
            changeProperty(ee, t, st, ps);
        }
    } else {
        changeProperty(e, t, st, ps);
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
                sp = score()->spatium();
            }
            EngravingObject::undoChangeProperty(Pid::OFFSET, score()->styleV(getPropertyStyle(Pid::OFFSET)).value<PointF>() * sp);
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
            changeProperties(this, p.pid, score()->styleV(p.sid), PropertyFlags::STYLED);
        }
    } else if (id == Pid::OFFSET) {
        // TODO: do this in caller?
        if (isEngravingItem()) {
            EngravingItem* e = toEngravingItem(this);
            if (e->offset() != v.value<PointF>()) {
                e->setOffsetChanged(true, false, v.value<PointF>() - e->offset());
            }
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
    score()->undoStack()->push1(new ChangeProperty(this, id, val));
}

//---------------------------------------------------------
//   readProperty
//---------------------------------------------------------

void EngravingObject::readProperty(XmlReader& e, Pid id)
{
    PropertyValue v = mu::engraving::readProperty(id, e);
    switch (propertyType(id)) {
    case P_TYPE::MILLIMETRE: //! NOTE type mm, but stored in xml as spatium
        v = v.value<Spatium>().toMM(score()->spatium());
        break;
    case P_TYPE::POINT:
        if (offsetIsSpatiumDependent()) {
            v = v.value<PointF>() * score()->spatium();
        } else {
            v = v.value<PointF>() * DPMM;
        }
        break;
    default:
        break;
    }
    setProperty(id, v);
    if (isStyled(id)) {
        setPropertyFlags(id, PropertyFlags::UNSTYLED);
    }
}

bool EngravingObject::readProperty(const AsciiStringView& s, XmlReader& e, Pid id)
{
    if (s == propertyName(id)) {
        readProperty(e, id);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
//   writeProperty
//
//    - styled properties are never written
//    - unstyled properties are always written regardless of value,
//    - properties without style are written if different from default value
//-----------------------------------------------------------------------------

void EngravingObject::writeProperty(XmlWriter& xml, Pid pid) const
{
    if (isStyled(pid)) {
        return;
    }
    PropertyValue p = getProperty(pid);
    if (!p.isValid()) {
        LOGD("%s invalid property %d <%s>", typeName(), int(pid), propertyName(pid));
        return;
    }
    PropertyFlags f = propertyFlags(pid);
    PropertyValue d = (f != PropertyFlags::STYLED) ? propertyDefault(pid) : PropertyValue();

    if (pid == Pid::FONT_STYLE) {
        FontStyle ds = FontStyle(d.isValid() ? d.toInt() : 0);
        FontStyle fs = FontStyle(p.toInt());
        if ((fs& FontStyle::Bold) != (ds & FontStyle::Bold)) {
            xml.tag("bold", fs & FontStyle::Bold);
        }
        if ((fs& FontStyle::Italic) != (ds & FontStyle::Italic)) {
            xml.tag("italic", fs & FontStyle::Italic);
        }
        if ((fs& FontStyle::Underline) != (ds & FontStyle::Underline)) {
            xml.tag("underline", fs & FontStyle::Underline);
        }
        if ((fs& FontStyle::Strike) != (ds & FontStyle::Strike)) {
            xml.tag("strike", fs & FontStyle::Strike);
        }
        return;
    }

    P_TYPE type = propertyType(pid);
    if (P_TYPE::MILLIMETRE == type) {
        double f1 = p.toReal();
        if (d.isValid() && std::abs(f1 - d.toReal()) < 0.0001) {            // fuzzy compare
            return;
        }
        p = PropertyValue(Spatium::fromMM(f1, score()->spatium()));
        d = PropertyValue();
    } else if (P_TYPE::POINT == type) {
        PointF p1 = p.value<PointF>();
        if (d.isValid()) {
            PointF p2 = d.value<PointF>();
            if ((std::abs(p1.x() - p2.x()) < 0.0001) && (std::abs(p1.y() - p2.y()) < 0.0001)) {
                return;
            }
        }
        double q = offsetIsSpatiumDependent() ? score()->spatium() : DPMM;
        p = PropertyValue(p1 / q);
        d = PropertyValue();
    }
    xml.tagProperty(pid, p, d);
}

//---------------------------------------------------------
//   readStyledProperty
//---------------------------------------------------------

bool EngravingObject::readStyledProperty(XmlReader& e, const AsciiStringView& tag)
{
    for (const StyledProperty& spp : *styledProperties()) {
        if (readProperty(tag, e, spp.pid)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   writeStyledProperties
//---------------------------------------------------------

void EngravingObject::writeStyledProperties(XmlWriter& xml) const
{
    for (const StyledProperty& spp : *styledProperties()) {
        writeProperty(xml, spp.pid);
    }
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
//   readAddConnector
//---------------------------------------------------------

void EngravingObject::readAddConnector(ConnectorInfoReader* info, bool pasteMode)
{
    UNUSED(pasteMode);
    LOGD("Cannot add connector %s to %s", info->connector()->typeName(), typeName());
}

//---------------------------------------------------------
//   linkTo
//    link this to element
//---------------------------------------------------------
void EngravingObject::linkTo(EngravingObject* element)
{
    assert(element != this);
    assert(!_links);

    if (element->links()) {
        _links = element->_links;
        assert(_links->contains(element));
    } else {
        if (isStaff()) {
            _links = new LinkedObjects(score(), -1);       // don’t use lid
        } else {
            _links = new LinkedObjects(score());
        }
        _links->push_back(element);
        element->_links = _links;
    }
    assert(!_links->contains(this));
    _links->push_back(this);
}

//---------------------------------------------------------
//   unlink
//---------------------------------------------------------

void EngravingObject::unlink()
{
    if (!_links) {
        return;
    }

    assert(_links->contains(this));
    _links->remove(this);

    // if link list is empty, remove list
    if (_links->size() <= 1) {
        if (!_links->empty()) {
            _links->front()->_links = 0;
        }
        delete _links;
    }
    _links = 0;   // this element is not linked anymore
}

//---------------------------------------------------------
//   isLinked
///  return true if se is different and
///  linked to this element
//---------------------------------------------------------

bool EngravingObject::isLinked(EngravingObject* se) const
{
    if (se == this || !_links) {
        return false;
    }

    if (se == nullptr) {
        return !_links->empty() && _links->mainElement() != this;
    }

    return _links->contains(se);
}

//---------------------------------------------------------
//   findLinkedInScore
///  if exists, returns the linked object in the required
///  score, else returns null
//---------------------------------------------------------

EngravingObject* EngravingObject::findLinkedInScore(Score* score) const
{
    if (score == this || !_links || _links->empty()) {
        return nullptr;
    }
    auto findElem = std::find_if(_links->begin(), _links->end(),
                                 [score](EngravingObject* engObj) { return engObj && engObj->score() == score; });
    return findElem != _links->end() ? *findElem : nullptr;
}

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void EngravingObject::undoUnlink()
{
    if (_links) {
        score()->undo(new Unlink(this));
    }
}

//---------------------------------------------------------
//   linkList
//---------------------------------------------------------

std::list<EngravingObject*> EngravingObject::linkList() const
{
    std::list<EngravingObject*> el;
    if (_links) {
        el = *_links;
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
    for (const StyledProperty& p : *_elementStyle) {
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
    return _propertyFlagsList[i];
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
    _propertyFlagsList[i] = f;
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid EngravingObject::getPropertyStyle(Pid id) const
{
    for (const StyledProperty& p : *_elementStyle) {
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
    for (const StyledProperty& spp : *_elementStyle) {
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
    return TConv::userName(type());
}

String EngravingObject::translatedTypeUserName() const
{
    return typeUserName().translated();
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
           || type() == ElementType::FINGERING
           || type() == ElementType::HARMONY
           || type() == ElementType::MARKER
           || type() == ElementType::JUMP
           || type() == ElementType::STAFF_TEXT
           || type() == ElementType::SYSTEM_TEXT
           || type() == ElementType::TRIPLET_FEEL
           || type() == ElementType::PLAYTECH_ANNOTATION
           || type() == ElementType::REHEARSAL_MARK
           || type() == ElementType::INSTRUMENT_CHANGE
           || type() == ElementType::FIGURED_BASS
           || type() == ElementType::TEMPO_TEXT
           || type() == ElementType::INSTRUMENT_NAME
           || type() == ElementType::MEASURE_NUMBER
           || type() == ElementType::MMREST_RANGE
           || type() == ElementType::STICKING
    ;
}

//---------------------------------------------------------
//   styleValue
//---------------------------------------------------------

PropertyValue EngravingObject::styleValue(Pid pid, Sid sid) const
{
    switch (propertyType(pid)) {
    case P_TYPE::MILLIMETRE:
        return score()->styleMM(sid);
    case P_TYPE::POINT: {
        PointF val = score()->styleV(sid).value<PointF>();
        if (offsetIsSpatiumDependent()) {
            val *= score()->spatium();
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
        return score()->styleV(sid);
    }
}
}
