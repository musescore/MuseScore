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

#include "engravingobject.h"
#include "elementgroup.h"

#include "mscore.h"
#include "shape.h"
#include "sig.h"

#include "infrastructure/draw/color.h"
#include "infrastructure/draw/geometry.h"
#include "infrastructure/draw/painter.h"

#include "modularity/ioc.h"
#include "iengravingconfiguration.h"

#include "types/symid.h"
#include "types/fraction.h"

namespace mu::engraving {
class Factory;
class AccessibleItem;
}

namespace mu::engraving {
#ifdef Q_OS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#endif

enum class Pid;
class StaffType;
class XmlReader;
class XmlWriter;
class MuseScoreView;

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

enum class KerningType
{
    KERNING,
    NON_KERNING,
    LIMITED_KERNING,
    SAME_VOICE_LIMIT,
    KERNING_UNTIL_ORIGIN,
    NOT_SET,
};

class ElementEditData;

//---------------------------------------------------------
//   EditData
//    used in editDrag
//---------------------------------------------------------

class EditData
{
    std::list<std::shared_ptr<ElementEditData> > data;
    MuseScoreView* view_ { 0 };

public:
    MuseScoreView* view() const { return view_; }

    std::vector<mu::RectF> grip;
    int grips                        { 0 };                 // number of grips
    Grip curGrip                     { Grip::NO_GRIP };

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
    QString preeditString;

    Qt::MouseButtons buttons         { Qt::NoButton };

    // drop data:
    mu::PointF dragOffset;
    EngravingItem* element                 { 0 };
    EngravingItem* dropElement             { 0 };

    EditData(MuseScoreView* v = nullptr)
        : view_(v) {}

    void clear();

    std::shared_ptr<ElementEditData> getData(const EngravingItem*) const;
    void addData(std::shared_ptr<ElementEditData>);
    bool control(bool textEditing = false) const;
    bool shift() const { return modifiers & Qt::ShiftModifier; }
    bool isStartEndGrip() { return curGrip == Grip::START || curGrip == Grip::END; }
};

using ElementEditDataPtr = std::shared_ptr<ElementEditData>;

class EngravingItemList : public std::list<EngravingItem*>
{
public:

    EngravingItem* at(size_t i) const;
};

//-------------------------------------------------------------------
//    @@ EngravingItem
///     \brief Base class of score layout elements
///
///     The EngravingItem class is the virtual base class of all
///     score layout elements.
//-------------------------------------------------------------------

class EngravingItem : public EngravingObject
{
    INJECT_STATIC(engraving, IEngravingConfiguration, engravingConfiguration)

    mutable mu::RectF _bbox;  ///< Bounding box relative to _pos + _offset
    qreal _mag;                     ///< standard magnification (derived value)
    mu::PointF _pos;          ///< Reference position, relative to _parent, set by autoplace
    mu::PointF _offset;       ///< offset from reference position, set by autoplace or user
    OffsetChange _offsetChanged;    ///< set by user actions that change offset, used by autoplace
    mu::PointF _changedPos;   ///< position set when changing offset
    Spatium _minDistance;           ///< autoplace min distance
    track_idx_t _track = mu::nidx; ///< staffIdx * VOICES + voice
    mutable ElementFlags _flags;
    ///< valid after call to layout()
    uint _tag;                    ///< tag bitmask

    AccessibleItem* m_accessible = nullptr;
    bool m_accessibleEnabled = false;

    bool m_colorsInversionEnabled = true;

    virtual bool sameVoiceKerningLimited() const { return false; }
    virtual bool neverKernable() const { return false; }
    virtual bool alwaysKernable() const { return false; }
    KerningType _userSetKerning = KerningType::NOT_SET;

protected:
    mutable int _z;
    mu::draw::Color _color;                ///< element color attribute
    bool _skipDraw{ false };

    friend class Factory;
    EngravingItem(const ElementType& type, EngravingObject* se = 0, ElementFlags = ElementFlag::NOTHING);
    EngravingItem(const EngravingItem&);
    virtual AccessibleItem* createAccessible();
    virtual KerningType doComputeKerningType(const EngravingItem*) const { return KerningType::KERNING; }

public:

    virtual ~EngravingItem();

    KerningType computeKerningType(const EngravingItem* nextItem) const;
    virtual double computePadding(const EngravingItem* nextItem) const;

    virtual void setupAccessible();
    bool accessibleEnabled() const;
    void setAccessibleEnabled(bool enabled);

    EngravingItem& operator=(const EngravingItem&) = delete;
    //@ create a copy of the element
    Q_INVOKABLE virtual EngravingItem* clone() const = 0;
    virtual EngravingItem* linkedClone();

    void deleteLater();

    EngravingItem* parentItem() const;
    EngravingItemList childrenItems() const;

    EngravingItem* findAncestor(ElementType t);
    const EngravingItem* findAncestor(ElementType t) const;

    Measure* findMeasure();
    const Measure* findMeasure() const;
    MeasureBase* findMeasureBase();
    const MeasureBase* findMeasureBase() const;

    //!Note Returns basic representative for the current element.
    //!     For example: notes->chord, chords->beam, etc.
    virtual EngravingItem* elementBase() const { return const_cast<EngravingItem*>(this); }

    virtual bool isEngravingItem() const override { return true; }

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

    bool sizeIsSpatiumDependent() const override { return !flag(ElementFlag::SIZE_SPATIUM_DEPENDENT); }
    void setSizeIsSpatiumDependent(bool v) { setFlag(ElementFlag::SIZE_SPATIUM_DEPENDENT, !v); }
    bool offsetIsSpatiumDependent() const override;

    PlacementV placement() const { return PlacementV(!flag(ElementFlag::PLACE_ABOVE)); }
    void setPlacement(PlacementV val) { setFlag(ElementFlag::PLACE_ABOVE, !bool(val)); }
    bool placeAbove() const { return placement() == PlacementV::ABOVE; }
    bool placeBelow() const { return placement() == PlacementV::BELOW; }
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
    virtual void setPos(qreal x, qreal y) { _pos.setX(x), _pos.setY(y); }
    virtual void setPos(const mu::PointF& p) { _pos = p; }
    mu::PointF& rpos() { return _pos; }
    qreal& rxpos() { return _pos.rx(); }
    qreal& rypos() { return _pos.ry(); }
    virtual void move(const mu::PointF& s) { _pos += s; }
    bool skipDraw() const { return _skipDraw; }

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
    virtual Shape shape() const { return Shape(bbox(), this); }
    virtual qreal baseLine() const { return -height(); }

    virtual int subtype() const { return -1; }                    // for select gui

    virtual void draw(mu::draw::Painter*) const {}
    void drawAt(mu::draw::Painter* p, const mu::PointF& pt) const { p->translate(pt); draw(p); p->translate(-pt); }

    virtual void writeProperties(XmlWriter& xml) const;
    virtual bool readProperties(XmlReader&);

    virtual void write(XmlWriter&) const;
    virtual void read(XmlReader&);

//       virtual ElementGroup getElementGroup() { return SingleElementGroup(this); }
    virtual std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const EngravingItem*)> isDragged)
    {
        Q_UNUSED(isDragged);
        return std::unique_ptr<ElementGroup>(new SingleElementGroup(this));
    }

    virtual void startDrag(EditData&);
    virtual mu::RectF drag(EditData&);
    virtual void endDrag(EditData&);
    /** Returns anchor lines displayed while dragging element in canvas coordinates. */
    virtual std::vector<mu::LineF> dragAnchorLines() const { return std::vector<mu::LineF>(); }
    /**
     * A generic \ref dragAnchorLines() implementation which can be used in
     * dragAnchorLines() overrides in descendants. It is not made its default
     * implementation in EngravingItem class since showing anchor lines is not
     * desirable for most element types.
     * TODO: maybe Annotation class could be extracted which would be a base
     * class of various annotation types and which would have this
     * dragAnchorLines() implementation by default.
     */
    std::vector<mu::LineF> genericDragAnchorLines() const;

    virtual bool isEditable() const { return !flag(ElementFlag::GENERATED); }
    virtual bool needStartEditingAfterSelecting() const { return false; }

    virtual void startEdit(EditData&);
    virtual bool isEditAllowed(EditData&) const;
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
    virtual std::vector<mu::LineF> gripAnchorLines(Grip) const { return std::vector<mu::LineF>(); }

    virtual int gripsCount() const { return 0; }
    virtual Grip initialEditModeGrip() const { return Grip::NO_GRIP; }
    virtual Grip defaultGrip() const { return Grip::NO_GRIP; }
    /** Returns grips positions in page coordinates. */
    virtual std::vector<mu::PointF> gripsPositions(const EditData& = EditData()) const { return std::vector<mu::PointF>(); }

    bool hasGrips() const;

    track_idx_t track() const;
    virtual void setTrack(track_idx_t val);

    int z() const;
    void setZ(int val);

    staff_idx_t staffIdx() const;
    void setStaffIdx(staff_idx_t val);
    staff_idx_t staffIdxOrNextVisible() const; // for system objects migrating
    bool isTopSystemObject() const;
    virtual staff_idx_t vStaffIdx() const;
    voice_idx_t voice() const;
    void setVoice(voice_idx_t v);
    Staff* staff() const;
    bool hasStaff() const;
    const StaffType* staffType() const;
    bool onTabStaff() const;
    Part* part() const;

    virtual void add(EngravingItem*);
    virtual void remove(EngravingItem*);
    virtual void added() {}
    virtual void removed() {}
    virtual void change(EngravingItem* o, EngravingItem* n);

    virtual void layout() {}
    virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
    virtual void localSpatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

    // debug functions
    virtual void dump() const;
    virtual Q_INVOKABLE QString subtypeName() const;
    //@ Returns the human-readable name of the element type
    //@ Returns the name of the element type
    virtual Q_INVOKABLE QString _name() const { return QString(typeName()); }

    virtual mu::draw::Color color() const;
    mu::draw::Color curColor() const;
    mu::draw::Color curColor(bool isVisible) const;
    mu::draw::Color curColor(bool isVisible, mu::draw::Color normalColor) const;
    virtual void setColor(const mu::draw::Color& c) { _color = c; }

    void undoSetColor(const mu::draw::Color& c);
    void undoSetVisible(bool v);
    void undoAddElement(EngravingItem* element);

    static ElementType readType(XmlReader& node, mu::PointF*, Fraction*);
    static EngravingItem* readMimeData(Score* score, const mu::ByteArray& data, mu::PointF*, Fraction*);

    virtual mu::ByteArray mimeData(const mu::PointF&) const;
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
    virtual EngravingItem* drop(EditData&) { return 0; }

/**
 delivers mouseEvent to element in edit mode
 returns true if mouse event is accepted by element
 */
    virtual bool mousePress(EditData&) { return false; }

    mutable bool itemDiscovered      { false };       ///< helper flag for bsp

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    virtual void reset() override;           // reset all properties & position to default

    virtual qreal mag() const { return _mag; }
    void setMag(qreal val) { _mag = val; }
    qreal magS() const;

    bool isPrintable() const;
    bool isPlayable() const;
    qreal point(const Spatium sp) const { return sp.val() * spatium(); }

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

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;
    PropertyValue propertyDefault(Pid) const override;
    Pid propertyId(const QStringRef& xmlName) const override;
    virtual EngravingItem* propertyDelegate(Pid) { return 0; }     // return Spanner for SpannerSegment for some properties

    bool custom(Pid) const;
    virtual bool isUserModified() const;

    void drawSymbol(SymId id, mu::draw::Painter* p, const mu::PointF& o = mu::PointF(), qreal scale = 1.0) const;
    void drawSymbol(SymId id, mu::draw::Painter* p, const mu::PointF& o, int n) const;
    void drawSymbols(const SymIdList&, mu::draw::Painter* p, const mu::PointF& o = mu::PointF(), qreal scale = 1.0) const;
    void drawSymbols(const SymIdList&, mu::draw::Painter* p, const mu::PointF& o, const mu::SizeF& scale) const;
    qreal symHeight(SymId id) const;
    qreal symWidth(SymId id) const;
    qreal symWidth(const SymIdList&) const;
    mu::RectF symBbox(SymId id) const;
    mu::RectF symBbox(const SymIdList&) const;

    mu::PointF symSmuflAnchor(SymId symId, SmuflAnchorId anchorId) const;

    qreal symAdvance(SymId id) const;
    bool symIsValid(SymId id) const;

    bool concertPitch() const;
    virtual EngravingItem* nextElement();   // selects the next score element, (notes, rests etc. as well as articulation etc.)
    virtual EngravingItem* prevElement();   // selects the next score element, (notes, rests etc. as well as articulation etc.)
    virtual EngravingItem* nextSegmentElement();    //< Used for navigation
    virtual EngravingItem* prevSegmentElement();    //< next-element and prev-element command

    AccessibleItem* accessible() const;
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
    virtual void drawEditMode(mu::draw::Painter* painter, EditData& editData, qreal currentViewScaling);

    void autoplaceSegmentElement(bool above, bool add);          // helper functions
    void autoplaceMeasureElement(bool above, bool add);
    void autoplaceSegmentElement(bool add = true) { autoplaceSegmentElement(placeAbove(), add); }
    void autoplaceMeasureElement(bool add = true) { autoplaceMeasureElement(placeAbove(), add); }
    void autoplaceCalculateOffset(mu::RectF& r, qreal minDistance);
    qreal rebaseOffset(bool nox = true);
    bool rebaseMinDistance(qreal& md, qreal& yd, qreal sp, qreal rebase, bool above, bool fix);

    qreal styleP(Sid idx) const;

    bool colorsInversionEnabled() const;
    void setColorsInverionEnabled(bool enabled);

    virtual QString formatBarsAndBeats() const;
    std::pair<int, float> barbeat() const;

private:
    void initAccessibleIfNeed();
};

using ElementPtr = std::shared_ptr<EngravingItem>;

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
    PropertyValue data;
    PropertyFlags f;
};

class ElementEditData
{
public:
    EngravingItem* e = nullptr;
    std::list<PropertyData> propertyData;
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

class ElementList : public std::vector<EngravingItem*>
{
public:
    ElementList() {}
    bool remove(EngravingItem*);
    void replace(EngravingItem* old, EngravingItem* n);
    void write(XmlWriter&) const;
    void write(XmlWriter&, const char* name) const;
};

//---------------------------------------------------------
//   @@ Compound
//---------------------------------------------------------

class Compound : public EngravingItem
{
    std::list<EngravingItem*> elements;

protected:
    const std::list<EngravingItem*>& getElements() const { return elements; }

public:
    Compound(const ElementType& type, Score*);
    Compound(const Compound&);

    virtual void draw(mu::draw::Painter*) const;
    virtual void addElement(EngravingItem*, qreal x, qreal y);
    void clear();
    virtual void setSelected(bool f);
    virtual void setVisible(bool);
    virtual void layout();
};

extern bool elementLessThan(const EngravingItem* const, const EngravingItem* const);
extern void collectElements(void* data, EngravingItem* e);
} // mu::engraving

Q_DECLARE_METATYPE(mu::engraving::ElementType);

#endif
