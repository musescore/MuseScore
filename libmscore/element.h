//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "spatium.h"
#include "fraction.h"
#include "scoreElement.h"
#include "shape.h"

namespace Ms {

#ifdef Q_OS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#endif

#ifndef VOICES
#define VOICES 4
#endif

class ConnectorInfoReader;
class XmlReader;
class XmlWriter;
enum class SymId;
enum class Pid;
enum class OffsetType : char;

//---------------------------------------------------------
//   Grip
//---------------------------------------------------------

enum class Grip {
      NO_GRIP = -1,
      START = 0, END = 1,                         // arpeggio etc.
          MIDDLE = 2, APERTURE = 3,               // Line
      /*START, END , */
          BEZIER1 = 2, SHOULDER = 3, BEZIER2 = 4, DRAG = 5, // Slur
      GRIPS = 6                     // number of grips for slur
      };

//---------------------------------------------------------
//   ElementFlag
//---------------------------------------------------------

enum class ElementFlag {
      NOTHING                = 0x00000000,
      DROP_TARGET            = 0x00000001,
      NOT_SELECTABLE         = 0x00000002,
      MOVABLE                = 0x00000004,
      COMPOSITION            = 0x00000008,       // true if element is part of another element
      HAS_TAG                = 0x00000010,       // true if this is a layered element
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
      TRAILER                = 0x00400000,    // also used in segment
      KEYSIG                 = 0x00800000,

      // segment flags
      ENABLED                = 0x01000000,    // used for segments
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

class EditData {
      QList<ElementEditData*> data;

   public:
      MuseScoreView* view              { 0       };

      QVector<QRectF> grip;
      int grips                        { 0       };         // number of grips
      Grip curGrip                     { Grip(0) };

      QPointF pos;
      QPointF startMove;
      QPoint  startMovePixel;
      QPointF lastPos;
      QPointF delta;
      bool hRaster                     { false };
      bool vRaster                     { false };

      int key                          { 0     };
      Qt::KeyboardModifiers modifiers  { 0     };
      QString s;

      Qt::MouseButtons buttons         { Qt::NoButton };

      // drop data:
      QPointF dragOffset;
      Element* element                 { 0     };
      Element* dropElement             { 0     };
      Fraction duration                { Fraction(1,4) };

      EditData(MuseScoreView* v) : view(v) {}
      void init();
      void clearData();

      ElementEditData* getData(const Element*) const;
      void addData(ElementEditData*);
      bool control(bool textEditing = false) const;
      bool shift() const    { return modifiers & Qt::ShiftModifier; }
      bool isStartEndGrip() { return curGrip == Grip::START || curGrip == Grip::END; }
      };

//-------------------------------------------------------------------
//    @@ Element
///     \brief Base class of score layout elements
///
///     The Element class is the virtual base class of all
///     score layout elements.
//-------------------------------------------------------------------

class Element : public ScoreElement {
      Element* _parent { 0 };
      mutable QRectF _bbox;       ///< Bounding box relative to _pos + _offset
      qreal _mag;                 ///< standard magnification (derived value)
      QPointF _pos;               ///< Reference position, relative to _parent.
      QPointF _offset;            ///< offset from reference position, set by autoplace or user
      int _track;                 ///< staffIdx * VOICES + voice
      mutable ElementFlags _flags;
                                  ///< valid after call to layout()
      uint _tag;                  ///< tag bitmask

  protected:
      mutable int _z;
      QColor _color;              ///< element color attribute

   public:
      Element(Score* = 0, ElementFlags = ElementFlag::NOTHING);
      Element(const Element&);
      virtual ~Element();

      Element &operator=(const Element&) = delete;
      //@ create a copy of the element
      Q_INVOKABLE virtual Ms::Element* clone() const = 0;
      virtual Element* linkedClone();

      Element* parent() const                 { return _parent;     }
      void setParent(Element* e)              { _parent = e;        }
      Measure* findMeasure();
      const Measure* findMeasure() const;
      MeasureBase* findMeasureBase();
      const MeasureBase* findMeasureBase() const;

      virtual bool isElement() const override { return true;        }

      qreal spatium() const;

      inline void setFlag(ElementFlag f, bool v)       { if (v) _flags |= f; else _flags &= ~ElementFlags(f); }
      inline void setFlag(ElementFlag f, bool v) const { if (v) _flags |= f; else _flags &= ~ElementFlags(f); }
      inline bool flag(ElementFlag f) const            { return _flags & f; }

      bool selected() const                   { return flag(ElementFlag::SELECTED); }
      virtual void setSelected(bool f)        { setFlag(ElementFlag::SELECTED, f);  }

      bool visible() const                    { return !flag(ElementFlag::INVISIBLE);  }
      virtual void setVisible(bool f)         { setFlag(ElementFlag::INVISIBLE, !f);   }

      virtual bool sizeIsSpatiumDependent() const override { return !flag(ElementFlag::SIZE_SPATIUM_DEPENDENT); }
      void setSizeIsSpatiumDependent(bool v)  { setFlag(ElementFlag::SIZE_SPATIUM_DEPENDENT, !v); }

      Placement placement() const             { return Placement(!flag(ElementFlag::PLACE_ABOVE));  }
      void setPlacement(Placement val)        { setFlag(ElementFlag::PLACE_ABOVE, !bool(val)); }
      bool placeAbove() const                 { return placement() == Placement::ABOVE; }
      bool placeBelow() const                 { return placement() == Placement::BELOW; }

      bool generated() const                  { return flag(ElementFlag::GENERATED);  }
      void setGenerated(bool val)             { setFlag(ElementFlag::GENERATED, val);   }

      //TODO: rename to pos()
      const QPointF& ipos() const             { return _pos;                    }
      //TODO: rename to posWithUserOffset()
      virtual const QPointF pos() const       { return _pos + _offset;          }
      virtual qreal x() const                 { return _pos.x() + _offset.x(); }
      virtual qreal y() const                 { return _pos.y() + _offset.y(); }
      void setPos(qreal x, qreal y)           { _pos.rx() = x, _pos.ry() = y;   }
      void setPos(const QPointF& p)           { _pos = p;                }
      QPointF& rpos()                         { return _pos;                    }
      qreal& rxpos()                          { return _pos.rx();        }
      qreal& rypos()                          { return _pos.ry();        }
      virtual void move(const QPointF& s)     { _pos += s;               }

      virtual QPointF pagePos() const;          ///< position in page coordinates
      virtual QPointF canvasPos() const;        ///< position in canvas coordinates
      qreal pageX() const;
      qreal canvasX() const;

      const QPointF& offset() const            { return _offset;  }
      virtual void setOffset(const QPointF& o) { _offset = o;     }
      void setOffset(qreal x, qreal y) { _offset.rx() = x, _offset.ry() = y; }
      QPointF& roffset()                       { return _offset; }
      qreal& rxoffset()                        { return _offset.rx(); }
      qreal& ryoffset()                        { return _offset.ry(); }

      bool isNudged() const                       { return !_offset.isNull(); }

      virtual const QRectF& bbox() const          { return _bbox;              }
      virtual QRectF& bbox()                      { return _bbox;              }
      virtual qreal height() const                { return bbox().height();    }
      virtual void setHeight(qreal v)             { _bbox.setHeight(v);        }
      virtual qreal width() const                 { return bbox().width();     }
      virtual void setWidth(qreal v)              { _bbox.setWidth(v);         }
      QRectF abbox() const                        { return bbox().translated(pagePos());   }
      QRectF pageBoundingRect() const             { return bbox().translated(pagePos());   }
      QRectF canvasBoundingRect() const           { return bbox().translated(canvasPos()); }
      virtual void setbbox(const QRectF& r) const { _bbox = r;           }
      virtual void addbbox(const QRectF& r) const { _bbox |= r;          }
      bool contains(const QPointF& p) const;
      bool intersects(const QRectF& r) const;
#ifndef NDEBUG
      virtual Shape shape() const                 { return Shape(bbox(), name());   }
#else
      virtual Shape shape() const                 { return Shape(bbox());   }
#endif
      virtual qreal baseLine() const              { return -height();       }

      virtual int subtype() const                 { return -1; }  // for select gui

      virtual void draw(QPainter*) const {}
      void drawAt(QPainter*p, const QPointF& pt) const { p->translate(pt); draw(p); p->translate(-pt);}

      virtual void writeProperties(XmlWriter& xml) const;
      virtual bool readProperties(XmlReader&);

      virtual void write(XmlWriter&) const;
      virtual void read(XmlReader&);

      virtual void startDrag(EditData&);
      virtual QRectF drag(EditData&);
      virtual void endDrag(EditData&);
      virtual QLineF dragAnchor() const       { return QLineF(); }

      virtual bool isEditable() const         { return !flag(ElementFlag::GENERATED); }

      virtual void startEdit(EditData&);
      virtual bool edit(EditData&);
      virtual void startEditDrag(EditData&);
      virtual void editDrag(EditData&);
      virtual void endEditDrag(EditData&);
      virtual void endEdit(EditData&);

      virtual void editCut(EditData&)            {}
      virtual void editCopy(EditData&)           {}

      virtual void updateGrips(EditData&) const  {}
      virtual bool nextGrip(EditData&) const;
      virtual bool prevGrip(EditData&) const;
      virtual QPointF gripAnchor(Grip) const     { return QPointF(); }

      int track() const                       { return _track; }
      virtual void setTrack(int val)          { _track = val;  }

      int z() const;
      void setZ(int val)                      { _z = val;  }

      int staffIdx() const                    { return _track >> 2;        }
      virtual int vStaffIdx() const           { return staffIdx();         }
      int voice() const                       { return _track & 3;         }
      void setVoice(int v)                    { _track = (_track / VOICES) * VOICES + v; }
      Staff* staff() const;
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
      void dumpQPointF(const char*) const;

      virtual QColor color() const             { return _color; }
      QColor curColor() const;
      QColor curColor(bool isVisible) const;
      QColor curColor(bool isVisible, QColor normalColor) const;
      virtual void setColor(const QColor& c)     { _color = c;    }
      void undoSetColor(const QColor& c);
      void undoSetVisible(bool v);

      static ElementType readType(XmlReader& node, QPointF*, Fraction*);

      QByteArray mimeData(const QPointF&) const;
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
      virtual Element* drop(EditData&) { return 0;}

/**
 delivers mouseEvent to element in edit mode
 returns true if mouse event is accepted by element
 */
      virtual bool mousePress(EditData&) { return false; }

      mutable bool itemDiscovered      { false };     ///< helper flag for bsp

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      virtual void reset() override;         // reset all properties & position to default

      virtual qreal mag() const        { return _mag;   }
      void setMag(qreal val)           { _mag = val;    }
      qreal magS() const;

      bool isPrintable() const;
      qreal point(const Spatium sp) const { return sp.val() * spatium(); }

      virtual int tick() const;       // utility, searches for segment / segment parent
      virtual int rtick() const;      // utility, searches for segment / segment parent
      virtual Fraction rfrac() const; // utility, searches for segment / segment parent
      virtual Fraction afrac() const; // utility, searches for segment / segment parent

      //
      // check element for consistency; return false if element
      // is not valid
      //
      virtual bool check() const { return true; }

      static Ms::Element* create(Ms::ElementType type, Score*);
      static Element* name2Element(const QStringRef&, Score*);

      bool systemFlag() const          { return flag(ElementFlag::SYSTEM);  }
      void setSystemFlag(bool v) const { setFlag(ElementFlag::SYSTEM, v);  }

      bool header() const              { return flag(ElementFlag::HEADER);        }
      void setHeader(bool v)           { setFlag(ElementFlag::HEADER, v);         }

      bool trailer() const             { return flag(ElementFlag::TRAILER); }
      void setTrailer(bool val)        { setFlag(ElementFlag::TRAILER, val); }

      bool selectable() const          { return !flag(ElementFlag::NOT_SELECTABLE);  }
      void setSelectable(bool val)     { setFlag(ElementFlag::NOT_SELECTABLE, !val); }

      bool dropTarget() const          { return flag(ElementFlag::DROP_TARGET); }
      void setDropTarget(bool v) const { setFlag(ElementFlag::DROP_TARGET, v);  }

      bool composition() const          { return flag(ElementFlag::COMPOSITION); }
      void setComposition(bool v) const { setFlag(ElementFlag::COMPOSITION, v);  }

      virtual bool isMovable() const   { return flag(ElementFlag::MOVABLE);     }

      bool enabled() const             { return flag(ElementFlag::ENABLED); }
      void setEnabled(bool val)        { setFlag(ElementFlag::ENABLED, val); }

      uint tag() const                 { return _tag;                      }
      void setTag(uint val)            { _tag = val;                       }

      bool autoplace() const           { return !flag(ElementFlag::NO_AUTOPLACE); }
      void setAutoplace(bool v)        { setFlag(ElementFlag::NO_AUTOPLACE, !v); }

      virtual QVariant getProperty(Pid) const override;
      virtual bool setProperty(Pid, const QVariant&) override;
      virtual void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;
      using ScoreElement::undoChangeProperty;
      virtual QVariant propertyDefault(Pid) const override;
      virtual Pid propertyId(const QStringRef& xmlName) const override;
      virtual QString propertyUserValue(Pid) const override;
      virtual Element* propertyDelegate(Pid) { return 0; }  // return Spanner for SpannerSegment for some properties

      bool custom(Pid) const;
      virtual bool isUserModified() const;

      void drawSymbol(SymId id, QPainter* p, const QPointF& o = QPointF(), qreal scale = 1.0) const;
      void drawSymbol(SymId id, QPainter* p, const QPointF& o, int n) const;
      void drawSymbols(const std::vector<SymId>&, QPainter* p, const QPointF& o = QPointF(), qreal scale = 1.0) const;
      void drawSymbols(const std::vector<SymId>&, QPainter* p, const QPointF& o, const QSizeF& scale) const;
      qreal symHeight(SymId id) const;
      qreal symWidth(SymId id) const;
      qreal symWidth(const std::vector<SymId>&) const;
      QRectF symBbox(SymId id) const;
      QRectF symBbox(const std::vector<SymId>&) const;
      QPointF symStemDownNW(SymId id) const;
      QPointF symStemUpSE(SymId id) const;
      QPointF symCutOutNE(SymId id) const;
      QPointF symCutOutNW(SymId id) const;
      QPointF symCutOutSE(SymId id) const;
      QPointF symCutOutSW(SymId id) const;
      qreal symAdvance(SymId id) const;
      bool symIsValid(SymId id) const;

      bool concertPitch() const;
      virtual Element* nextElement(); // selects the next score element, (notes, rests etc. as well as articulation etc.)
      virtual Element* prevElement(); // selects the next score element, (notes, rests etc. as well as articulation etc.)
      virtual Element* nextSegmentElement();  //< Used for navigation
      virtual Element* prevSegmentElement();  //< next-element and prev-element command

      virtual QString accessibleInfo() const;         //< used to populate the status bar
      virtual QString screenReaderInfo() const  {     //< by default returns accessibleInfo, but can be overridden
            return accessibleInfo();
            }
                                                       //  if the screen-reader needs a special string (see note for example)
      virtual QString accessibleExtraInfo() const {    // used to return info that will be appended to accessibleInfo
            return QString();                          // and passed only to the screen-reader
            }

      virtual void triggerLayout() const;
      virtual void drawEditMode(QPainter*, EditData&);

      void autoplaceSegmentElement(qreal minDistance);      // helper function
      void autoplaceMeasureElement(qreal minDistance);
      qreal styleP(Sid idx) const;
      };

//-----------------------------------------------------------------------------
//   ElementEditData
//    holds element specific data during element editing:
//
//    startEditDrag(EditData&)    creates data and attaches it to EditData
//       editDrag(EditData&)
//    endEditDrag(EditData&)      use data to create undo records
//-----------------------------------------------------------------------------

struct PropertyData {
      Pid id;
      QVariant data;
      };

class ElementEditData {
   public:
      Element* e;
      QList<PropertyData> propertyData;

      void pushProperty(Pid pid) { propertyData.push_back(PropertyData({pid, e->getProperty(pid) })); }
      };

//---------------------------------------------------------
//   ElementList
//---------------------------------------------------------

class ElementList : public std::vector<Element*> {
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

class Compound : public Element {
      QList<Element*> elements;

   protected:
      const QList<Element*>& getElements() const { return elements; }

   public:
      Compound(Score*);
      Compound(const Compound&);
      virtual ElementType type() const = 0;

      virtual void draw(QPainter*) const;
      virtual void addElement(Element*, qreal x, qreal y);
      void clear();
      virtual void setSelected(bool f);
      virtual void setVisible(bool);
      virtual void layout();
      };

extern bool elementLessThan(const Element* const, const Element* const);
extern void collectElements(void* data, Element* e);


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::ElementType);

#endif

