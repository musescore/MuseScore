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

#include <optional>

#include "engravingobject.h"
#include "elementgroup.h"

#include "draw/types/color.h"
#include "draw/types/geometry.h"
#include "draw/painter.h"

#include "modularity/ioc.h"
#include "iengravingconfiguration.h"
#include "rendering/iscorerenderer.h"

#include "types/fraction.h"
#include "types/symid.h"
#include "types/types.h"

#include "shape.h"
#include "editdata.h"

#define DECLARE_LAYOUTDATA_METHODS(Class) \
    const LayoutData* layoutData() const { return static_cast<const Class::LayoutData*>(EngravingItem::layoutData()); } \
    LayoutData* mutLayoutData() { return static_cast<Class::LayoutData*>(EngravingItem::mutLayoutData()); } \
    LayoutData* createLayoutData() const override { return new Class::LayoutData(); } \

namespace mu::engraving {
class Factory;
class XmlReader;

#ifndef ENGRAVING_NO_ACCESSIBILITY
class AccessibleItem;
typedef std::shared_ptr<AccessibleItem> AccessibleItemPtr;
#endif

enum class Pid;
class StaffType;

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

typedef Flags<ElementFlag> ElementFlags;
DECLARE_OPERATORS_FOR_FLAGS(ElementFlags)

enum class KerningType
{
    KERNING,
    NON_KERNING,
    LIMITED_KERNING,
    SAME_VOICE_LIMIT,
    KERNING_UNTIL_ORIGIN,
    ALLOW_COLLISION,
    NOT_SET,
};

//! NOTE Enum for detect layoutdata bad access errors
enum class LD_ACCESS {
    CHECK = 0,          // should be correct; assert if still not
    BAD,                // known to be bad; don’t assert, we’ll fix it later
    MAYBE_NOTINITED     // in this case it’s okay if we access it before it’s been inited
};

class EngravingItemList : public std::list<EngravingItem*>
{
    OBJECT_ALLOCATOR(engraving, EngravingItemList)
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
    INJECT_STATIC(IEngravingConfiguration, engravingConfiguration)
    INJECT_STATIC(rendering::IScoreRenderer, renderer)

public:

    virtual ~EngravingItem();

    EngravingItem& operator=(const EngravingItem&) = delete;
    //@ create a copy of the element
    virtual EngravingItem* clone() const = 0;
    virtual EngravingItem* linkedClone();

    void deleteLater();

    EngravingItem* parentItem(bool explicitParent = true) const;
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

    double spatium() const;

    inline void setFlag(ElementFlag f, bool v)
    {
        if (v) {
            m_flags |= f;
        } else {
            m_flags &= ~ElementFlags(f);
        }
    }

    inline void setFlag(ElementFlag f, bool v) const
    {
        if (v) {
            m_flags |= f;
        } else {
            m_flags &= ~ElementFlags(f);
        }
    }

    inline bool flag(ElementFlag f) const { return m_flags & f; }

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

    Spatium minDistance() const { return m_minDistance; }
    void setMinDistance(Spatium v) { m_minDistance = v; }

    virtual PointF pagePos() const;            ///< position in page coordinates
    virtual PointF canvasPos() const;          ///< position in canvas coordinates
    double pageX() const;
    double canvasX() const;

    PointF mapFromCanvas(const PointF& p) const { return p - canvasPos(); }
    PointF mapToCanvas(const PointF& p) const { return p + canvasPos(); }

    const PointF& offset() const { return m_offset; }
    virtual void setOffset(const PointF& o) { m_offset = o; }
    void setOffset(double x, double y) { m_offset.setX(x), m_offset.setY(y); }
    PointF& roffset() { return m_offset; }
    double& rxoffset() { return m_offset.rx(); }
    double& ryoffset() { return m_offset.ry(); }

    virtual Fraction tick() const;
    virtual Fraction rtick() const;
    virtual Fraction playTick() const;   ///< Returns the tick at which playback should begin when this element is selected. Defaults to the element's own tick position.

    Fraction beat() const;

    bool isNudged() const { return !m_offset.isNull(); }

    bool contains(const PointF& p) const;
    bool intersects(const mu::RectF& r) const;

    virtual mu::RectF hitBBox() const { return layoutData()->bbox(); }
    virtual Shape hitShape() const { return shape(); }
    Shape canvasHitShape() const { return hitShape().translate(canvasPos()); }
    bool hitShapeContains(const PointF& p) const;
    bool hitShapeIntersects(const mu::RectF& rr) const;

    virtual int subtype() const { return -1; }                    // for select gui

    void drawAt(mu::draw::Painter* p, const PointF& pt) const
    {
        p->translate(pt);
        renderer()->drawItem(this, p);
        p->translate(-pt);
    }

//       virtual ElementGroup getElementGroup() { return SingleElementGroup(this); }
    virtual std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const EngravingItem*)> /*isDragged*/)
    {
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
    virtual std::vector<PointF> gripsPositions(const EditData& = EditData()) const { return std::vector<PointF>(); }

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

    virtual void spatiumChanged(double /*oldValue*/, double /*newValue*/);
    virtual void localSpatiumChanged(double /*oldValue*/, double /*newValue*/);

    // debug functions
    virtual void dump() const;
    virtual TranslatableString subtypeUserName() const;
    virtual String translatedSubtypeUserName() const;

    virtual mu::draw::Color color() const;
    mu::draw::Color curColor() const;
    mu::draw::Color curColor(bool isVisible) const;
    mu::draw::Color curColor(bool isVisible, mu::draw::Color normalColor) const;
    virtual void setColor(const mu::draw::Color& c) { m_color = c; }

    void undoSetColor(const mu::draw::Color& c);
    void undoSetVisible(bool v);
    void undoAddElement(EngravingItem* element);

    static ElementType readType(XmlReader& node, PointF*, Fraction*);
    static EngravingItem* readMimeData(Score* score, const mu::ByteArray& data, PointF*, Fraction*);

    virtual mu::ByteArray mimeData(const PointF& dragOffset = PointF()) const;
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

    double magS() const;

    bool isPrintable() const;
    bool isPlayable() const;
    double point(const Spatium sp) const { return sp.val() * spatium(); }

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

    bool autoplace() const;
    virtual void setAutoplace(bool v) { setFlag(ElementFlag::NO_AUTOPLACE, !v); }
    bool addToSkyline() const { return !(m_flags & (ElementFlag::INVISIBLE | ElementFlag::NO_AUTOPLACE)); }

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;
    PropertyValue propertyDefault(Pid) const override;
    virtual EngravingItem* propertyDelegate(Pid) { return 0; }     // return Spanner for SpannerSegment for some properties

    bool custom(Pid) const;
    virtual bool isUserModified() const;

    void drawSymbol(SymId id, mu::draw::Painter* p, const PointF& o = PointF(), double scale = 1.0) const;
    void drawSymbols(const SymIdList&, mu::draw::Painter* p, const PointF& o = PointF(), double scale = 1.0) const;
    void drawSymbols(const SymIdList&, mu::draw::Painter* p, const PointF& o, const SizeF& scale) const;
    double symHeight(SymId id) const;
    double symWidth(SymId id) const;
    double symWidth(const SymIdList&) const;
    RectF symBbox(SymId id) const;
    RectF symBbox(const SymIdList&) const;

    PointF symSmuflAnchor(SymId symId, SmuflAnchorId anchorId) const;

    double symAdvance(SymId id) const;
    bool symIsValid(SymId id) const;

    bool concertPitch() const;
    virtual EngravingItem* nextElement();   // selects the next score element, (notes, rests etc. as well as articulation etc.)
    virtual EngravingItem* prevElement();   // selects the next score element, (notes, rests etc. as well as articulation etc.)
    virtual EngravingItem* nextSegmentElement();    //< Used for navigation
    virtual EngravingItem* prevSegmentElement();    //< next-element and prev-element command

#ifndef ENGRAVING_NO_ACCESSIBILITY
    virtual void setupAccessible();
    AccessibleItemPtr accessible() const;
    void initAccessibleIfNeed();
#endif

    bool accessibleEnabled() const;
    void setAccessibleEnabled(bool enabled);

    virtual String accessibleInfo() const;
    virtual String screenReaderInfo() const { return accessibleInfo(); }
    //  if the screen-reader needs a special string (see note for example)
    // used to return info that will be appended to accessibleInfo
    virtual String accessibleExtraInfo() const { return String(); }
    virtual String formatBarsAndBeats() const;

    virtual void triggerLayout() const;
    virtual void triggerLayoutAll() const;
    virtual void drawEditMode(draw::Painter* painter, EditData& editData, double currentViewScaling);

    double styleP(Sid idx) const;

    bool colorsInversionEnabled() const;
    void setColorsInverionEnabled(bool enabled);

    std::pair<int, float> barbeat() const;

    struct Autoplace {
        OffsetChange offsetChanged = OffsetChange::NONE;     // set by user actions that change offset, used by autoplace
        PointF changedPos;                // position set when changing offset
    };

    struct LayoutData {
        virtual ~LayoutData() = default;

        Autoplace autoplace;

        void reset()
        {
            m_bbox.reset();
        }

        bool isSkipDraw() const { return m_isSkipDraw; }
        void setIsSkipDraw(bool val) { m_isSkipDraw = val; }

        double mag() const { return m_mag; }
        void setMag(double val) { m_mag = val; }

        void resetPos() { m_pos = PointF(); }
        const PointF& pos() const { return m_pos; }
        void setPos(const PointF& p) { doSetPos(p.x(), p.y()); }
        void setPos(double x, double y) { doSetPos(x, y); }
        void setPosX(double x) { doSetPos(x, m_pos.y()); }
        void setPosY(double y) { doSetPos(m_pos.x(), y); }
        void move(const PointF& p) { doSetPos(m_pos.x() + p.x(), m_pos.y() + p.y()); }
        void moveX(double x) { doSetPos(m_pos.x() + x, m_pos.y()); }
        void moveY(double y) { doSetPos(m_pos.x(), m_pos.y() + y); }

        void resetBbox() { m_bbox.reset(); }
        const RectF& bbox(LD_ACCESS mode = LD_ACCESS::CHECK) const
        {
            if (!m_bbox.has_value()) {
                if (mode == LD_ACCESS::CHECK) {
                    LOGE() << "BAD ACCESS to bbox (not set)";
                }
                static const RectF _dummy;
                return _dummy;
            }

            return m_bbox.value();
        }

        void setBbox(const mu::RectF& r) { m_bbox = std::make_optional<RectF>(r); }
        void setBbox(double x, double y, double w, double h) { mutBbox().setRect(x, y, w, h); }
        void addBbox(const mu::RectF& r) { mutBbox().unite(r); }
        void setHeight(double v) { mutBbox().setHeight(v); }
        void setWidth(double v) { mutBbox().setWidth(v); }

        OffsetChange offsetChanged() const { return autoplace.offsetChanged; }

    protected:
        inline void doSetPos(double x, double y)
        {
            m_pos.setX(x),
            m_pos.setY(y);
        }

        mu::RectF& mutBbox()
        {
            if (!m_bbox) {
                m_bbox = std::make_optional<RectF>();
            }
            return m_bbox.value();
        }

        bool m_isSkipDraw = false;
        double m_mag = 1.0;                     // standard magnification (derived value)
        PointF m_pos;                           // Reference position, relative to _parent, set by autoplace
        std::optional<RectF> m_bbox;            // Bounding box relative to _pos + _offset
    };

    const LayoutData* layoutData() const;
    LayoutData* mutLayoutData();

    virtual double mag() const;
    virtual Shape shape() const { return Shape(layoutData()->bbox(), this); }
    virtual double baseLine() const { return -height(); }

    mu::RectF abbox(LD_ACCESS mode = LD_ACCESS::CHECK) const { return layoutData()->bbox(mode).translated(pagePos()); }
    mu::RectF pageBoundingRect(LD_ACCESS mode = LD_ACCESS::CHECK) const { return layoutData()->bbox(mode).translated(pagePos()); }
    mu::RectF canvasBoundingRect(LD_ACCESS mode = LD_ACCESS::CHECK) const { return layoutData()->bbox(mode).translated(canvasPos()); }

    //! --- Old Interface ---
    virtual void setbbox(const mu::RectF& r) { mutLayoutData()->setBbox(r); }
    virtual void addbbox(const mu::RectF& r) { mutLayoutData()->addBbox(r); }
    virtual double height() const { return layoutData()->bbox().height(); }
    virtual void setHeight(double v) { mutLayoutData()->setHeight(v); }
    virtual double width(LD_ACCESS mode = LD_ACCESS::CHECK) const { return layoutData()->bbox(mode).width(); }
    virtual void setWidth(double v) { mutLayoutData()->setWidth(v); }

    virtual const PointF pos() const { return layoutData()->pos() + m_offset; }
    virtual double x() const { return layoutData()->pos().x() + m_offset.x(); }
    virtual double y() const { return layoutData()->pos().y() + m_offset.y(); }
    virtual void setPos(double x, double y) { mutLayoutData()->setPos(x, y); }
    virtual void setPos(const PointF& p) { mutLayoutData()->setPos(p.x(), p.y()); }

    virtual void move(const PointF& s) { mutLayoutData()->move(s); }

    void setOffsetChanged(bool val, bool absolute = true, const PointF& diff = PointF());
    //! ---------------------

protected:
    EngravingItem(const ElementType& type, EngravingObject* parent = nullptr, ElementFlags = ElementFlag::NOTHING);
    EngravingItem(const EngravingItem&);

#ifndef ENGRAVING_NO_ACCESSIBILITY
    virtual AccessibleItemPtr createAccessible();
    void notifyAboutNameChanged();
#endif

    virtual LayoutData* createLayoutData() const;

    mutable int m_z = 0;
    mu::draw::Color m_color;                // element color attribute

    track_idx_t m_track = mu::nidx;         // staffIdx * VOICES + voice

private:

    friend class Factory;

#ifndef ENGRAVING_NO_ACCESSIBILITY
    void doInitAccessible();
    AccessibleItemPtr m_accessible;
#endif

    bool m_accessibleEnabled = false;

    PointF m_offset;                    // offset from reference position, set by autoplace or user

    Spatium m_minDistance;              // autoplace min distance
    mutable ElementFlags m_flags;

    bool m_colorsInversionEnabled = true;

    mutable LayoutData* m_layoutData = nullptr;
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
    OBJECT_ALLOCATOR(engraving, ElementEditData)
public:
    EngravingItem* e = nullptr;
    std::list<PropertyData> propertyData;
    PointF initOffset;   ///< for dragging: difference between actual offset and editData.moveDelta

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
    OBJECT_ALLOCATOR(engraving, ElementList)
public:
    ElementList() {}
    bool remove(EngravingItem*);
    void replace(EngravingItem* old, EngravingItem* n);
};

//---------------------------------------------------------
//   @@ Compound
//---------------------------------------------------------

class Compound : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Compound)

    std::list<EngravingItem*> elements;

protected:
    const std::list<EngravingItem*>& getElements() const { return elements; }

public:
    Compound(const ElementType& type, Score*);
    Compound(const Compound&);

    virtual void draw(mu::draw::Painter*) const;
    virtual void addElement(EngravingItem*, double x, double y);
    void clear();
    virtual void setSelected(bool f);
    virtual void setVisible(bool);
    virtual void layout();
};

extern bool elementLessThan(const EngravingItem* const, const EngravingItem* const);
extern void collectElements(void* data, EngravingItem* e);
} // mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::ElementType);
#endif

#endif
