//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ELEMENTLAYOUT_H__
#define __ELEMENTLAYOUT_H__

namespace Ms {

class Element;
class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   OffsetType
//---------------------------------------------------------

enum class OffsetType : char {
      ABS,       ///< offset in point units
      SPATIUM    ///< offset in staff space units
      };

//---------------------------------------------------------
//   Align
//---------------------------------------------------------

enum class Align : char {
      LEFT     = 0,
      RIGHT    = 1,
      HCENTER  = 2,
      TOP      = 0,
      BOTTOM   = 4,
      VCENTER  = 8,
      BASELINE = 16,
      CENTER = Align::HCENTER | Align::VCENTER,
      HMASK  = Align::LEFT    | Align::RIGHT    | Align::HCENTER,
      VMASK  = Align::TOP     | Align::BOTTOM   | Align::VCENTER | Align::BASELINE
      };

constexpr Align operator| (Align a1, Align a2) {
      return static_cast<Align>(static_cast<char>(a1) | static_cast<char>(a2));
      }
// constexpr Align operator& (Align a1, Align a2) {
//      return static_cast<Align>(static_cast<char>(a1) & static_cast<char>(a2));
//      }
constexpr bool operator& (Align a1, Align a2) {
      return static_cast<char>(a1) & static_cast<char>(a2);
      }
constexpr Align operator~ (Align a) {
      return static_cast<Align>(~static_cast<char>(a));
      }

//---------------------------------------------------------
//   ElementLayout
//---------------------------------------------------------

class ElementLayout {

   protected:
      Align  _align;
      QPointF _offset;              // inch or spatium
      OffsetType _offsetType;

   public:
      ElementLayout();
      ElementLayout(Align a, const QPointF& o, OffsetType ot)
         : _align(a), _offset(o), _offsetType(ot) {}

      Align align() const                 { return _align;        }
      OffsetType offsetType() const       { return _offsetType;   }
      qreal xOffset() const               { return _offset.x();   }
      qreal yOffset() const               { return _offset.y();   }
      const QPointF& offset() const       { return _offset;      }
      QPointF offset(qreal) const;
      void setAlign(Align val)            { _align  = val;        }
      void setXoff(qreal val)             { _offset.rx() = val;   }
      void setYoff(qreal val)             { _offset.ry() = val;        }
      void setOffsetType(OffsetType val)  { _offsetType = val;    }
      void layout(Element*) const;
      void writeProperties(XmlWriter& xml) const;
      void writeProperties(XmlWriter& xml, const ElementLayout&) const;
      bool readProperties(XmlReader& e);
      void restyle(const ElementLayout& ol, const ElementLayout& nl);
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Align);

#endif

