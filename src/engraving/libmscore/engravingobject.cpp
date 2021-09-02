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

#include "translation.h"
#include "io/xml.h"

#include "score.h"
#include "undo.h"
#include "bracket.h"
#include "bracketItem.h"
#include "measure.h"
#include "spanner.h"
#include "musescoreCore.h"
#include "sym.h"
#include "masterscore.h"
#include "factory.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
ElementStyle const EngravingObject::emptyStyle;

//---------------------------------------------------------
//   ScoreElement
//---------------------------------------------------------

EngravingObject::EngravingObject(const ElementType& type, EngravingObject* parent)
    : m_type(type), m_parent(parent)
{
    IF_ASSERT_FAILED(this != parent) {
        return;
    }

    if (type != ElementType::SCORE) {
        IF_ASSERT_FAILED(parent) {
        }
    }

    if (elementsProvider()) {
        elementsProvider()->reg(this);
    }
}

EngravingObject::EngravingObject(const EngravingObject& se)
{
    m_type = se.m_type;
    m_parent = se.m_parent;
    m_isParentExplicitlySet = se.m_isParentExplicitlySet;
    m_isDummy = se.m_isDummy;
    _elementStyle = se._elementStyle;
    if (_elementStyle) {
        size_t n = _elementStyle->size();
        _propertyFlagsList = new PropertyFlags[n];
        for (size_t i = 0; i < n; ++i) {
            _propertyFlagsList[i] = se._propertyFlagsList[i];
        }
    }
    _links = 0;
}

//---------------------------------------------------------
//   ~ScoreElement
//---------------------------------------------------------

EngravingObject::~EngravingObject()
{
    if (elementsProvider()) {
        elementsProvider()->unreg(this);
    }

    if (_links) {
        _links->removeOne(this);
        if (_links->empty()) {
            delete _links;
            _links = 0;
        }
    }
    delete[] _propertyFlagsList;
}

//---------------------------------------------------------
//   treeChildIdx
//---------------------------------------------------------

int EngravingObject::treeChildIdx(EngravingObject* child) const
{
    int i = 0;
    for (const EngravingObject* el : (*this)) {
        if (el == child) {
            return i;
        }
        i++;
    }
    return -1;
}

//---------------------------------------------------------
//   scanElements
/// Recursively apply scanElements to all children.
/// See also EngravingItem::scanElements.
//---------------------------------------------------------

void EngravingObject::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (EngravingObject* child : (*this)) {
        child->scanElements(data, func, all);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant EngravingObject::propertyDefault(Pid pid, Tid tid) const
{
    for (const StyledProperty& spp : *textStyle(tid)) {
        if (spp.pid == pid) {
            return styleValue(pid, spp.sid);
        }
    }
    return QVariant();
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant EngravingObject::propertyDefault(Pid pid) const
{
    Sid sid = getPropertyStyle(pid);
    if (sid != Sid::NOSTYLE) {
        return styleValue(pid, sid);
    }
//      qDebug("<%s>(%d) not found in <%s>", propertyQmlName(pid), int(pid), name());
    return QVariant();
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
    QVariant v = propertyDefault(pid);
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

static void changeProperty(EngravingObject* e, Pid t, const QVariant& st, PropertyFlags ps)
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

static void changeProperties(EngravingObject* e, Pid t, const QVariant& st, PropertyFlags ps)
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

void EngravingObject::undoChangeProperty(Pid id, const QVariant& v)
{
    undoChangeProperty(id, v, propertyFlags(id));
}

void EngravingObject::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
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
            qreal sp;
            if (isEngravingItem()) {
                sp = toEngravingItem(this)->spatium();
            } else {
                sp = score()->spatium();
            }
            EngravingObject::undoChangeProperty(Pid::OFFSET, score()->styleV(getPropertyStyle(Pid::OFFSET)).value<PointF>() * sp);
            EngravingItem* e = toEngravingItem(this);
            e->setOffsetChanged(false);
        }
    } else if (id == Pid::SUB_STYLE) {
        //
        // change a list of properties
        //
        auto l = textStyle(Tid(v.toInt()));
        // Change to ElementStyle defaults
        for (const StyledProperty& p : *l) {
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
        changeProperties(this, Pid::GENERATED, QVariant(false), PropertyFlags::NOSTYLE);
    }
}

//---------------------------------------------------------
//   undoPushProperty
//---------------------------------------------------------

void EngravingObject::undoPushProperty(Pid id)
{
    QVariant val = getProperty(id);
    score()->undoStack()->push1(new ChangeProperty(this, id, val));
}

//---------------------------------------------------------
//   readProperty
//---------------------------------------------------------

void EngravingObject::readProperty(XmlReader& e, Pid id)
{
    QVariant v = Ms::readProperty(id, e);
    switch (propertyType(id)) {
    case P_TYPE::SP_REAL:
        v = v.toReal() * score()->spatium();
        break;
    case P_TYPE::POINT_SP:
        v = v.value<PointF>() * score()->spatium();
        break;
    case P_TYPE::POINT_SP_MM:
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

bool EngravingObject::readProperty(const QStringRef& s, XmlReader& e, Pid id)
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
    QVariant p = getProperty(pid);
    if (!p.isValid()) {
        qDebug("%s invalid property %d <%s>", name(), int(pid), propertyName(pid));
        return;
    }
    PropertyFlags f = propertyFlags(pid);
    QVariant d = (f != PropertyFlags::STYLED) ? propertyDefault(pid) : QVariant();

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
        return;
    }

    if (propertyType(pid) == P_TYPE::SP_REAL) {
        qreal f1 = p.toReal();
        if (d.isValid() && qAbs(f1 - d.toReal()) < 0.0001) {            // fuzzy compare
            return;
        }
        p = QVariant(f1 / score()->spatium());
        d = QVariant();
    } else if (propertyType(pid) == P_TYPE::POINT_SP) {
        PointF p1 = p.value<PointF>();
        if (d.isValid()) {
            PointF p2 = d.value<PointF>();
            if ((qAbs(p1.x() - p2.x()) < 0.0001) && (qAbs(p1.y() - p2.y()) < 0.0001)) {
                return;
            }
        }
        p = QVariant(p1 / score()->spatium());
        d = QVariant();
    } else if (propertyType(pid) == P_TYPE::POINT_SP_MM) {
        PointF p1 = p.value<PointF>();
        if (d.isValid()) {
            PointF p2 = d.value<PointF>();
            if ((qAbs(p1.x() - p2.x()) < 0.0001) && (qAbs(p1.y() - p2.y()) < 0.0001)) {
                return;
            }
        }
        qreal q = offsetIsSpatiumDependent() ? score()->spatium() : DPMM;
        p = QVariant(p1 / q);
        d = QVariant();
    }
    xml.tag(pid, p, d);
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid EngravingObject::propertyId(const QStringRef& xmlName) const
{
    return Ms::propertyId(xmlName);
}

//---------------------------------------------------------
//   propertyUserValue
//---------------------------------------------------------

QString EngravingObject::propertyUserValue(Pid id) const
{
    QVariant val = getProperty(id);
    switch (propertyType(id)) {
    case P_TYPE::POINT_SP:
    {
        PointF p = val.value<PointF>();
        return QString("(%1, %2)").arg(p.x()).arg(p.y());
    }
    case P_TYPE::DIRECTION:
        return toUserString(val.value<Direction>());
    case P_TYPE::SYMID:
        return Sym::id2userName(val.value<SymId>());
    default:
        break;
    }
    return val.toString();
}

//---------------------------------------------------------
//   readStyledProperty
//---------------------------------------------------------

bool EngravingObject::readStyledProperty(XmlReader& e, const QStringRef& tag)
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
    Q_UNUSED(pasteMode);
    qDebug("Cannot add connector %s to %s", info->connector()->name(), name());
}

//---------------------------------------------------------
//   linkTo
//    link this to element
//---------------------------------------------------------

void EngravingObject::linkTo(EngravingObject* element)
{
    Q_ASSERT(element != this);
    Q_ASSERT(!_links);

    if (element->links()) {
        _links = element->_links;
        Q_ASSERT(_links->contains(element));
    } else {
        if (isStaff()) {
            _links = new LinkedElements(score(), -1);       // don’t use lid
        } else {
            _links = new LinkedElements(score());
        }
        _links->append(element);
        element->_links = _links;
    }
    Q_ASSERT(!_links->contains(this));
    _links->append(this);
}

//---------------------------------------------------------
//   unlink
//---------------------------------------------------------

void EngravingObject::unlink()
{
    if (!_links) {
        return;
    }

    Q_ASSERT(_links->contains(this));
    _links->removeOne(this);

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
        return !_links->isEmpty() && _links->mainElement() != this;
    }

    return _links->contains(se);
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

QList<EngravingObject*> EngravingObject::linkList() const
{
    QList<EngravingObject*> el;
    if (_links) {
        el = *_links;
    } else {
        el.append(const_cast<EngravingObject*>(this));
    }
    return el;
}

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

LinkedElements::LinkedElements(Score* score)
{
    _lid = score->linkId();   // create new unique id
}

LinkedElements::LinkedElements(Score* score, int id)
{
    _lid = id;
    if (_lid != -1) {
        score->linkId(id);          // remember used id
    }
}

//---------------------------------------------------------
//   setLid
//---------------------------------------------------------

void LinkedElements::setLid(Score* score, int id)
{
    _lid = id;
    score->linkId(id);
}

//---------------------------------------------------------
//   mainElement
//    Returns "main" linked element which is expected to
//    be written to the file prior to others.
//---------------------------------------------------------

EngravingObject* LinkedElements::mainElement()
{
    if (isEmpty()) {
        return nullptr;
    }
    MasterScore* ms = at(0)->masterScore();
    const bool elements = at(0)->isEngravingItem();
    const bool staves = at(0)->isStaff();
    return *std::min_element(begin(), end(), [ms, elements, staves](EngravingObject* s1, EngravingObject* s2) {
        if (s1->score() == ms && s2->score() != ms) {
            return true;
        }
        if (s1->score() != s2->score()) {
            return false;
        }
        if (staves) {
            return toStaff(s1)->idx() < toStaff(s2)->idx();
        }
        if (elements) {
            // Now we compare either two elements from master score
            // or two elements from excerpt.
            EngravingItem* e1 = toEngravingItem(s1);
            EngravingItem* e2 = toEngravingItem(s2);
            const int tr1 = e1->track();
            const int tr2 = e2->track();
            if (tr1 == tr2) {
                const Fraction tick1 = e1->tick();
                const Fraction tick2 = e2->tick();
                if (tick1 == tick2) {
                    Measure* m1 = e1->findMeasure();
                    Measure* m2 = e2->findMeasure();
                    if (!m1 || !m2) {
                        return false;
                    }

                    // MM rests are written to MSCX in the following order:
                    // 1) first measure of MM rest (m->hasMMRest() == true);
                    // 2) MM rest itself (m->isMMRest() == true);
                    // 3) other measures of MM rest (m->hasMMRest() == false).
                    //
                    // As mainElement() must find the first element that
                    // is going to be written to a file, MM rest writing
                    // order should also be considered.

                    if (m1->isMMRest() == m2->isMMRest()) {
                        // no difference if both are MM rests or both are usual measures
                        return false;
                    }

                    // MM rests may be generated but not written (e.g. if
                    // saving a file right after disabling MM rests)
                    const bool mmRestsWritten = e1->score()->styleB(Sid::createMultiMeasureRests);

                    if (m1->isMMRest()) {
                        // m1 is earlier if m2 is *not* the first MM rest measure
                        return mmRestsWritten && !m2->hasMMRest();
                    }
                    if (m2->isMMRest()) {
                        // m1 is earlier if it *is* the first MM rest measure
                        return !mmRestsWritten || m1->hasMMRest();
                    }
                    return false;
                }
                return tick1 < tick2;
            }
            return tr1 < tr2;
        }
        return false;
    });
}

void EngravingObject::setScore(Score* s)
{
    _score = s;
}

EngravingObject* EngravingObject::parent(bool isIncludeDummy) const
{
    if (!m_parent) {
        return nullptr;
    }

    //! NOTE We need to exclude a dummy for compatibility reasons.
    if (isIncludeDummy) {
        return m_parent;
    }

    if (!m_isParentExplicitlySet) {
        return nullptr;
    }

    if (m_parent->isScore()) {
        return nullptr;
    }

    if (m_parent->isDummy()) {
        return nullptr;
    }

    return m_parent;
}

void EngravingObject::setParent(EngravingObject* p, bool isExplicitly)
{
    IF_ASSERT_FAILED(this != p) {
        return;
    }

    if (!p) {
        moveToDummy();
        return;
    }

    m_parent = p;

    m_isParentExplicitlySet = isExplicitly;
}

void EngravingObject::moveToDummy()
{
    Score* sc = score();
    IF_ASSERT_FAILED(sc) {
        return;
    }

    if (sc->dummy() != this && sc->dummy()) {
        setParent(sc->dummy());
    }
}

void EngravingObject::setIsDummy(bool arg)
{
    m_isDummy = arg;
}

bool EngravingObject::isDummy() const
{
    return m_isDummy;
}

Score* EngravingObject::score(bool required) const
{
    if (_score) {
        return _score;
    }

    Score* sc = nullptr;
    EngravingObject* e = const_cast<EngravingObject*>(this);
    while (e) {
        if (e->isScore()) {
            sc = toScore(e);
            break;
        }

        e = e->m_parent;
    }

    if (required) {
        IF_ASSERT_FAILED(sc) {
        }
    }

    return sc;
}

MasterScore* EngravingObject::masterScore() const
{
    return score()->masterScore();
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

const char* EngravingObject::name() const
{
    return Factory::name(type());
}

QString EngravingObject::userName() const
{
    return qtrc("elementName", Factory::userName(type()));
}

//---------------------------------------------------------
//   isSLineSegment
//---------------------------------------------------------

bool EngravingObject::isSLineSegment() const
{
    return isHairpinSegment() || isOttavaSegment() || isPedalSegment()
           || isTrillSegment() || isVoltaSegment() || isTextLineSegment()
           || isGlissandoSegment() || isLetRingSegment() || isVibratoSegment() || isPalmMuteSegment();
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

QVariant EngravingObject::styleValue(Pid pid, Sid sid) const
{
    switch (propertyType(pid)) {
    case P_TYPE::SP_REAL:
        return score()->styleP(sid);
    case P_TYPE::POINT_SP: {
        PointF val = score()->styleV(sid).value<PointF>() * score()->spatium();
        if (isEngravingItem()) {
            const EngravingItem* e = toEngravingItem(this);
            if (e->staff() && !e->systemFlag()) {
                val *= e->staff()->staffMag(e->tick());
            }
        }
        return val;
    }
    case P_TYPE::POINT_SP_MM: {
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
