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

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "elementgroup.h"
#include "spatium.h"
#include "fraction.h"
#include "scoreElement.h"
#include "shape.h"
#include "sig.h"
#include "symid.h"

#include "draw/geometry.h"
#include "draw/painter.h"

namespace mu {
namespace score {
class AccessibleElement;
}
}

namespace Ms {
#ifdef Q_OS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#endif

#ifndef VOICES
#define VOICES 4
#endif

// Defined in Windows headers, conflicts with member functions named small().
#undef small

enum class Pid;
enum class SmuflAnchorId;
class StaffType;
class XmlReader;
class XmlWriter;

//---------------------------------------------------------
//   Grip
//---------------------------------------------------------

enum class Grip {
    NO_GRIP = -1,
    START = 0, END = 1,                           // arpeggio etc.
    MIDDLE = 2, APERTURE = 3,                     // Line
    /*START, END , */
    BEZIER1 = 2, SHOULDER = 3, BEZIER2 = 4, DRAG = 5,       // Slur
    GRIPS = 6                       // number of grips for slur
};

//---------------------------------------------------------
//   OffsetChange
//---------------------------------------------------------

enum class OffsetChange {
    RELATIVE_OFFSET   = -1,
    NONE              =  0,
    ABSOLUTE_OFFSET   =  1
};

//---------------------------------------------------------
//   ElementFlag
//---------------------------------------------------------

enum class ElementFlag {
    NOTHING                = 0x00000000,
    DROP_TARGET            = 0x00000001,
    NOT_SELECTABLE         = 0x00000002,
    MOVABLE                = 0x00000004,
    COMPOSITION            = 0x00000008,         // true if element is part of another element
    HAS_TAG                = 0x00000010,         // true if this is a layered element
    ON_STAFF               = 0x00000020,
    SELECTED               = 0x00000040,
    GENERATED              = 0x00000080,
    INVISIBLE              = 0x00000100,
    NO_AUTOPLACE           = 0x00000200,
    SYSTEM                 = 0x00000400,
    PLACE_ABOVE            = 0x00000800,
    SIZE_SPATIUM_DEPENDENT = 0x00001000,

    // measure flags
    REPEAT_END             = 0x00002000,
    REPEAT_START           = 0x00004000,
    REPEAT_JUMP            = 0x00008000,
    IRREGULAR              = 0x00010000,
    LINE_BREAK             = 0x00020000,
    PAGE_BREAK             = 0x00040000,
    SECTION_BREAK          = 0x00080000,
    NO_BREAK               = 0x00100000,
    HEADER                 = 0x00200000,
    TRAILER                = 0x00400000,      // also used in segment
    KEYSIG                 = 0x00800000,

    // segment flags
    ENABLED                = 0x01000000,      // used for segments
    EMPTY                  = 0x02000000,
    WRITTEN                = 0x04000000,
};

typedef QFlags<ElementFlag> ElementFlags;
Q_DECLARE_OPERATORS_FOR_FLAGS(ElementFlags);

class ElementEditData;

//---------------------------------------------------------
//   EditData
//    used in editDrag
//---------------------------------------------------------

class EditData
{
    QList<ElementEditData*> data;
    MuseScoreView* view_ { 0 };

public:
    MuseScoreView* view() const { return view_; }

    QVector<mu::RectF> grip;
    int grips                        { 0 };                 // number of grips
    Grip curGrip                     { Grip(0) };

    mu::PointF pos;
    mu::PointF startMove;
    mu::PointF normalizedStartMove; ///< Introduced for transition of drag logic. Don't use in new code.
    mu::Point startMovePixel;
    mu::PointF lastPos;
    mu::PointF delta;               ///< This property is deprecated, use evtDelta or moveDelta instead. In normal drag equals to moveDelta, in edit drag - to evtDelta
    mu::PointF evtDelta;            ///< Mouse offset for the last mouse move event
    mu::PointF moveDelta;           ///< Mouse offset from the start of mouse move
    bool hRaster                     { false };
    bool vRaster                     { false };

    int key                          { 0 };
    Qt::KeyboardModifiers modifiers  { /*0*/ };   // '0' initialized via default constructor, doing it here too results in compiler warning with Qt 5.15
    QString s;

    Qt::MouseButtons buttons         { Qt::NoButton };

    // drop data:
    mu::PointF dragOffset;
    Element* element                 { 0 };
    Element* dropElement             { 0 };

    EditData(MuseScoreView* v = nullptr)
        : view_(v) {}
    ~EditData();
    void clearData();

    ElementEditData* getData(const Element*) const;
    void addData(ElementEditData*);
    bool control(bool textEditing = false) const;
    bool shift() const { return modifiers & Qt::ShiftModifier; }
    bool isStartEndGrip() { return curGrip == Grip::START || curGrip == Grip::END; }
};

//-------------------------------------------------------------------
//    @@ Element
///     \brief Base class of score layout elements
///
///     The Element class is the virtual base class of all
///     score layout elements.
//-------------------------------------------------------------------

class Element : public ScoreElement
{
    Element* _parent { 0 };
    mutable mu::RectF _bbox;  ///< Bounding box relative to _pos + _offset
    qreal _mag;                     ///< standard magnification (derived value)
    mu::PointF _pos;          ///< Reference position, relative to _parent, set by autoplace
    mu::PointF _offset;       ///< offset from reference position, set by autoplace or user
    OffsetChange _offsetChanged;    ///< set by user actions that change offset, used by autoplace
    mu::PointF _changedPos;   ///< position set when changing offset
    Spatium _minDistance;           ///< autoplace min distance
    int _track;                     ///< staffIdx * VOICES + voice
    mutable ElementFlags _flags;
    ///< valid after call to layout()
    uint _tag;                    ///< tag bitmask

    mu::score::AccessibleElement* m_accessible = nullptr;

public:
    enum class EditBehavior {
        SelectOnly,
        Edit,
    };

protected:
    mutable int _z;
    QColor _color;                ///< element color attribute

public:
    Element(Score* = 0, ElementFlags = ElementFlag::NOTHING, mu::score::AccessibleElement* access = nullptr);
    Element(const Element&);
    virtual ~Element();

    Element& operator=(const Element&) = delete;
    //@ create a copy of the element
    Q_INVOKABLE virtual Ms::Element* clone() const = 0;
    virtual Element* linkedClone();

    void deleteLater();

    Element* parent() const { return _parent; }
    void setParent(Element* e) { _parent = e; }

    virtual ScoreElement* treeParent() const override { return _parent; }

    Element* findAncestor(ElementType t);
    const Element* findAncestor(ElementType t) const;

    Measure* findMeasure();
    const Measure* findMeasure() const;
    MeasureBase* findMeasureBase();
    const MeasureBase* findMeasureBase() const;

    //!Note Returns basic representative for the current element.
    //!     For example: notes->chord, chords->beam, etc.
    virtual Element* elementBase() const { return const_cast<Element*>(this); }

    virtual bool isElement() const override { return true; }

    qreal spatium() const;

    inline void setFlag(ElementFlag f, bool v)
    {
        if (v) {
            _flags |= f;
        } else {
            _flags &= ~ElementFlags(f);
        }
    }

    inline void setFlag(ElementFlag f, bool v) const
    {
        if (v) {
            _flags |= f;
        } else {
            _flags &= ~ElementFlags(f);
        }
    }

    inline bool flag(ElementFlag f) const { return _flags & f; }

    bool selected() const;
    virtual void setSelected(bool f);

    bool visible() const { return !flag(ElementFlag::INVISIBLE); }
    virtual void setVisible(bool f) { setFlag(ElementFlag::INVISIBLE, !f); }

    bool isInteractionAvailable() const;

    virtual bool sizeIsSpatiumDependent() const override { return !flag(ElementFlag::SIZE_SPATIUM_DEPENDENT); }
    void setSizeIsSpatiumDependent(bool v) { setFlag(ElementFlag::SIZE_SPATIUM_DEPENDENT, !v); }
    virtual bool offsetIsSpatiumDependent() const override;

    Placement placement() const { return Placement(!flag(ElementFlag::PLACE_ABOVE)); }
    void setPlacement(Placement val) { setFlag(ElementFlag::PLACE_ABOVE, !bool(val)); }
    bool placeAbove() const { return placement() == Placement::ABOVE; }
    bool placeBelow() const { return placement() == Placement::BELOW; }
    virtual bool placeMultiple() const { return true; }

    bool generated() const { return flag(ElementFlag::GENERATED); }
    void setGenerated(bool val) { setFlag(ElementFlag::GENERATED, val); }

    Spatium minDistance() const { return _minDistance; }
    void setMinDistance(Spatium v) { _minDistance = v; }
    OffsetChange offsetChanged() const { return _offsetChanged; }
    void setOffsetChanged(bool v, bool absolute = true, const mu::PointF& diff = mu::PointF());

    const mu::PointF& ipos() const { return _pos; }
    virtual const mu::PointF pos() const { return _pos + _offset; }
    virtual qreal x() const { return _pos.x() + _offset.x(); }
    virtual qreal y() const { return _pos.y() + _offset.y(); }
    void setPos(qreal x, qreal y) { _pos.setX(x), _pos.setY(y); }
    void setPos(const mu::PointF& p) { _pos = p; }
    mu::PointF& rpos() { return _pos; }
    qreal& rxpos() { return _pos.rx(); }
    qreal& rypos() { return _pos.ry(); }
    virtual void move(const mu::PointF& s) { _pos += s; }

    virtual mu::PointF pagePos() const;            ///< position in page coordinates
    virtual mu::PointF canvasPos() const;          ///< position in canvas coordinates
    qreal pageX() const;
    qreal canvasX() const;

    mu::PointF mapFromCanvas(const mu::PointF& p) const { return p - canvasPos(); }
    mu::PointF mapToCanvas(const mu::PointF& p) const { return p + canvasPos(); }

    const mu::PointF& offset() const { return _offset; }
    virtual void setOffset(const mu::PointF& o) { _offset = o; }
    void setOffset(qreal x, qreal y) { _offset.setX(x), _offset.setY(y); }
    mu::PointF& roffset() { return _offset; }
    qreal& rxoffset() { return _offset.rx(); }
    qreal& ryoffset() { return _offset.ry(); }

    virtual Fraction tick() const;
    virtual Fraction rtick() const;
    virtual Fraction playTick() const;   ///< Returns the tick at which playback should begin when this element is selected. Defaults to the element's own tick position.

    Fraction beat() const;

    bool isNudged() const { return !_offset.isNull(); }

    virtual const mu::RectF& bbox() const { return _bbox; }
    virtual mu::RectF& bbox() { return _bbox; }
    virtual qreal height() const { return bbox().height(); }
    virtual void setHeight(qreal v) { _bbox.setHeight(v); }
    virtual qreal width() const { return bbox().width(); }
    virtual void setWidth(qreal v) { _bbox.setWidth(v); }
    mu::RectF abbox() const { return bbox().translated(pagePos()); }
    mu::RectF pageBoundingRect() const { return bbox().translated(pagePos()); }
    mu::RectF canvasBoundingRect() const { return bbox().translated(canvasPos()); }
    virtual void setbbox(const mu::RectF& r) const { _bbox = r; }
    virtual void addbbox(const mu::RectF& r) const { _bbox.unite(r); }
    bool contains(const mu::PointF& p) const;
    bool intersects(const mu::RectF& r) const;
#ifndef NDEBUG
    virtual Shape shape() const { return Shape(bbox(), name()); }
#else
    virtual Shape shape() const { return Shape(bbox()); }
#endif
    virtual qreal baseLine() const { return -height(); }

    virtual int subtype() const { return -1; }                    // for select gui

    virtual void draw(mu::draw::Painter*) const {}
    void drawAt(mu::draw::Painter* p, const mu::PointF& pt) const { p->translate(pt); draw(p); p->translate(-pt); }

    virtual void writeProperties(XmlWriter& xml) const;
    virtual bool readProperties(XmlReader&);

    virtual void write(XmlWriter&) const;
    virtual void read(XmlReader&);

//       virtual ElementGroup getElementGroup() { return SingleElementGroup(this); }
    virtual std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const Element*)> isDragged)
    {
        Q_UNUSED(isDragged);
        return std::unique_ptr<ElementGroup>(new SingleElementGroup(this));
    }

    virtual void startDrag(EditData&);
    virtual mu::RectF drag(EditData&);
    virtual void endDrag(EditData&);
    /** Returns anchor lines displayed while dragging element in canvas coordinates. */
    virtual QVector<mu::LineF> dragAnchorLines() const { return QVector<mu::LineF>(); }
    /**
     * A generic \ref dragAnchorLines() implementation which can be used in
     * dragAnchorLines() overrides in descendants. It is not made its default
     * implementation in Element class since showing anchor lines is not
     * desirable for most element types.
     * TODO: maybe Annotation class could be extracted which would be a base
     * class of various annotation types and which would have this
     * dragAnchorLines() implementation by default.
     */
    QVector<mu::LineF> genericDragAnchorLines() const;

    virtual bool isEditable() const { return !flag(ElementFlag::GENERATED); }

    virtual void startEdit(EditData&);
    virtual bool edit(EditData&);
    virtual void startEditDrag(EditData&);
    virtual void editDrag(EditData&);
    virtual void endEditDrag(EditData&);
    virtual void endEdit(EditData&);

    virtual void editCut(EditData&) {}
    virtual void editCopy(EditData&) {}

    void updateGrips(EditData&) const;
    virtual bool nextGrip(EditData&) const;
    virtual bool prevGrip(EditData&) const;
    /** Returns anchor lines displayed while dragging element's grip in canvas coordinates. */
    virtual QVector<mu::LineF> gripAnchorLines(Grip) const { return QVector<mu::LineF>(); }

    virtual EditBehavior normalModeEditBehavior() const { return EditBehavior::SelectOnly; }
    virtual int gripsCount() const { return 0; }
    virtual Grip initialEditModeGrip() const { return Grip::NO_GRIP; }
    virtual Grip defaultGrip() const { return Grip::NO_GRIP; }
    /** Returns grips positions in page coordinates. */
    virtual std::vector<mu::PointF> gripsPositions(const EditData& = EditData()) const { return std::vector<mu::PointF>(); }

    int track() const;
    virtual void setTrack(int val);

    int z() const;
    void setZ(int val);

    int staffIdx() const;
    void setStaffIdx(int val);
    virtual int vStaffIdx() const;
    int voice() const;
    void setVoice(int v);
    Staff* staff() const;
    bool hasStaff() const;
    const StaffType* staffType() const;
    bool onTabStaff() const;
    Part* part() const;

    virtual void add(Element*);
    virtual void remove(Element*);
    virtual void change(Element* o, Element* n);

    virtual void layout() {}
    virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
    virtual void localSpatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

    // debug functions
    virtual void dump() const;
    virtual Q_INVOKABLE QString subtypeName() const;
    //@ Returns the human-readable name of the element type
    //@ Returns the name of the element type
    virtual Q_INVOKABLE QString _name() const { return QString(name()); }

    virtual QColor color() const { return _color; }
    QColor curColor() const;
    QColor curColor(bool isVisible) const;
    QColor curColor(bool isVisible, QColor normalColor) const;
    virtual void setColor(const QColor& c) { _color = c; }
    void undoSetColor(const QColor& c);
    void undoSetVisible(bool v);

    static ElementType readType(XmlReader& node, mu::PointF*, Fraction*);
    static Element* readMimeData(Score* score, const QByteArray& data, mu::PointF*, Fraction*);

    virtual QByteArray mimeData(const mu::PointF&) const;
/**
 Return true if this element accepts a drop at canvas relative \a pos
 of given element \a type and \a subtype.

 Reimplemented by elements that accept drops. Used to change cursor shape while
 dragging to indicate drop targets.
*/
    virtual bool acceptDrop(EditData&) const { return false; }

/**
 Handle a dropped element at canvas relative \a pos of given element
 \a type and \a subtype. Returns dropped element if any.
 The ownership of element in DropData is transferred to the called
 element (if not used, element has to be deleted).
 The returned element will be selected if not in note edit mode.

 Reimplemented by elements that accept drops.
*/
    virtual Element* drop(EditData&) { return 0; }

/**
 delivers mouseEvent to element in edit mode
 returns true if mouse event is accepted by element
 */
    virtual bool mousePress(EditData&) { return false; }

    mutable bool itemDiscovered      { false };       ///< helper flag for bsp

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    virtual void reset() override;           // reset all properties & position to default

    virtual qreal mag() const { return _mag; }
    void setMag(qreal val) { _mag = val; }
    qreal magS() const;

    bool isPrintable() const;
    qreal point(const Spatium sp) const { return sp.val() * spatium(); }

    static Ms::Element* create(Ms::ElementType type, Score*);
    static Element* name2Element(const QStringRef&, Score*);

    bool systemFlag() const { return flag(ElementFlag::SYSTEM); }
    void setSystemFlag(bool v) const { setFlag(ElementFlag::SYSTEM, v); }

    bool header() const { return flag(ElementFlag::HEADER); }
    void setHeader(bool v) { setFlag(ElementFlag::HEADER, v); }

    bool trailer() const { return flag(ElementFlag::TRAILER); }
    void setTrailer(bool val) { setFlag(ElementFlag::TRAILER, val); }

    bool selectable() const { return !flag(ElementFlag::NOT_SELECTABLE); }
    void setSelectable(bool val) { setFlag(ElementFlag::NOT_SELECTABLE, !val); }

    bool dropTarget() const { return flag(ElementFlag::DROP_TARGET); }
    void setDropTarget(bool v) const { setFlag(ElementFlag::DROP_TARGET, v); }

    bool composition() const { return flag(ElementFlag::COMPOSITION); }
    void setComposition(bool v) const { setFlag(ElementFlag::COMPOSITION, v); }

    virtual bool isMovable() const { return flag(ElementFlag::MOVABLE); }

    bool enabled() const { return flag(ElementFlag::ENABLED); }
    void setEnabled(bool val) { setFlag(ElementFlag::ENABLED, val); }

    uint tag() const { return _tag; }
    void setTag(uint val) { _tag = val; }

    bool autoplace() const;
    virtual void setAutoplace(bool v) { setFlag(ElementFlag::NO_AUTOPLACE, !v); }
    bool addToSkyline() const { return !(_flags & (ElementFlag::INVISIBLE | ElementFlag::NO_AUTOPLACE)); }

    virtual QVariant getProperty(Pid) const override;
    virtual bool setProperty(Pid, const QVariant&) override;
    virtual void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;
    using ScoreElement::undoChangeProperty;
    virtual QVariant propertyDefault(Pid) const override;
    virtual Pid propertyId(const QStringRef& xmlName) const override;
    virtual QString propertyUserValue(Pid) const override;
    virtual Element* propertyDelegate(Pid) { return 0; }    // return Spanner for SpannerSegment for some properties

    bool custom(Pid) const;
    virtual bool isUserModified() const;

    void drawSymbol(SymId id, mu::draw::Painter* p, const mu::PointF& o = mu::PointF(), qreal scale = 1.0) const;
    void drawSymbol(SymId id, mu::draw::Painter* p, const mu::PointF& o, int n) const;
    void drawSymbols(const std::vector<SymId>&, mu::draw::Painter* p, const mu::PointF& o = mu::PointF(), qreal scale = 1.0) const;
    void drawSymbols(const std::vector<SymId>&, mu::draw::Painter* p, const mu::PointF& o, const mu::SizeF& scale) const;
    qreal symHeight(SymId id) const;
    qreal symWidth(SymId id) const;
    qreal symWidth(const std::vector<SymId>&) const;
    mu::RectF symBbox(SymId id) const;
    mu::RectF symBbox(const std::vector<SymId>&) const;

    mu::PointF symSmuflAnchor(SymId symId, SmuflAnchorId anchorId) const;

    qreal symAdvance(SymId id) const;
    bool symIsValid(SymId id) const;

    bool concertPitch() const;
    virtual Element* nextElement();   // selects the next score element, (notes, rests etc. as well as articulation etc.)
    virtual Element* prevElement();   // selects the next score element, (notes, rests etc. as well as articulation etc.)
    virtual Element* nextSegmentElement();    //< Used for navigation
    virtual Element* prevSegmentElement();    //< next-element and prev-element command

    mu::score::AccessibleElement* accessible() const;
    virtual QString accessibleInfo() const;           //< used to populate the status bar
    virtual QString screenReaderInfo() const          //< by default returns accessibleInfo, but can be overridden
    {
        return accessibleInfo();
    }

    //  if the screen-reader needs a special string (see note for example)
    virtual QString accessibleExtraInfo() const        // used to return info that will be appended to accessibleInfo
    {
        return QString();                              // and passed only to the screen-reader
    }

    virtual void triggerLayout() const;
    virtual void triggerLayoutAll() const;
    virtual void drawEditMode(mu::draw::Painter*, EditData&);

    void autoplaceSegmentElement(bool above, bool add);          // helper functions
    void autoplaceMeasureElement(bool above, bool add);
    void autoplaceSegmentElement(bool add = true) { autoplaceSegmentElement(placeAbove(), add); }
    void autoplaceMeasureElement(bool add = true) { autoplaceMeasureElement(placeAbove(), add); }
    void autoplaceCalculateOffset(mu::RectF& r, qreal minDistance);
    qreal rebaseOffset(bool nox = true);
    bool rebaseMinDistance(qreal& md, qreal& yd, qreal sp, qreal rebase, bool above, bool fix);

    qreal styleP(Sid idx) const;
};

using ElementPtr = std::shared_ptr<Element>;

//-----------------------------------------------------------------------------
//   ElementEditData
//    holds element specific data during element editing:
//
//    startEditDrag(EditData&)    creates data and attaches it to EditData
//       editDrag(EditData&)
//    endEditDrag(EditData&)      use data to create undo records
//-----------------------------------------------------------------------------

enum class EditDataType : signed char {
    ElementEditData,
    TextEditData,
    BarLineEditData,
    BeamEditData,
    NoteEditData,
};

struct PropertyData {
    Pid id;
    QVariant data;
    PropertyFlags f;
};

class ElementEditData
{
public:
    Element* e;
    QList<PropertyData> propertyData;
    mu::PointF initOffset;   ///< for dragging: difference between actual offset and editData.moveDelta

    virtual ~ElementEditData() = default;
    void pushProperty(Pid pid)
    {
        propertyData.push_back(PropertyData({ pid, e->getProperty(pid), e->propertyFlags(pid) }));
    }

    virtual EditDataType type() { return EditDataType::ElementEditData; }
};

//---------------------------------------------------------
//   ElementList
//---------------------------------------------------------

class ElementList : public std::vector<Element*>
{
public:
    ElementList() {}
    bool remove(Element*);
    void replace(Element* old, Element* n);
    void write(XmlWriter&) const;
    void write(XmlWriter&, const char* name) const;
};

//---------------------------------------------------------
//   @@ Compound
//---------------------------------------------------------

class Compound : public Element
{
    QList<Element*> elements;

protected:
    const QList<Element*>& getElements() const { return elements; }

public:
    Compound(Score*);
    Compound(const Compound&);

    virtual void draw(mu::draw::Painter*) const;
    virtual void addElement(Element*, qreal x, qreal y);
    void clear();
    virtual void setSelected(bool f);
    virtual void setVisible(bool);
    virtual void layout();
};

extern bool elementLessThan(const Element* const, const Element* const);
extern void collectElements(void* data, Element* e);

extern void paintElement(mu::draw::Painter& painter, const Element* element);
extern void paintElements(mu::draw::Painter& painter, const QList<Element*>& elements);

template<typename T> std::shared_ptr<T> makeElement(Ms::Score* score)
{
    return std::make_shared<T>(score);
}
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::ElementType);

#endif
