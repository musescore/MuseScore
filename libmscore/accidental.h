//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2004-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ACCIDENTAL_H__
#define __ACCIDENTAL_H__

/**
 \file
 Definition of class Accidental
*/

#include <QString>
#include <QList>
#include <QVariant>

#include "element.h"
#include "sym.h"

namespace Ms {

class Note;
enum class AccidentalVal : signed char;

//---------------------------------------------------------
//   AccidentalRole
//---------------------------------------------------------

enum class AccidentalRole : char {
      AUTO,               // layout created accidental
      USER                // user created accidental
      };

//---------------------------------------------------------
//   AccidentalBracket
//---------------------------------------------------------

enum class AccidentalBracket : char {
      NONE,
      PARENTHESIS,
      BRACKET,
      BRACE,
      };

//---------------------------------------------------------
//   SymElement
//---------------------------------------------------------

struct SymElement {
      SymId sym;
      qreal x;
      qreal y;
      SymElement(SymId _sym, qreal _x, qreal _y) : sym(_sym), x(_x), y(_y) {}
      };

//---------------------------------------------------------
//   @@ Accidental
//   @P role        enum  (Accidental.AUTO, .USER) (read only)
//   @P isSmall     bool
//---------------------------------------------------------

class Accidental final : public Element {
      QList<SymElement> el;
      AccidentalType _accidentalType { AccidentalType::NONE };
      bool m_isSmall                 { false                   };
      AccidentalBracket _bracket     { AccidentalBracket::NONE };
      AccidentalRole _role           { AccidentalRole::AUTO    };

   public:
      Accidental(Score* s = 0);

      Accidental* clone() const override  { return new Accidental(*this); }
      ElementType type() const override   { return ElementType::ACCIDENTAL; }

      QString subtypeUserName() const;
      void setSubtype(const QString& s);
      void setAccidentalType(AccidentalType t)     { _accidentalType = t;    }

      AccidentalType accidentalType() const        { return _accidentalType; }
      AccidentalRole role() const                  { return _role;           }

      int subtype() const override         { return (int)_accidentalType; }
      QString subtypeName() const override { return QString(subtype2name(_accidentalType)); }

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;
      void layout() override;
      void layoutMultiGlyphAccidental();
      void layoutSingleGlyphAccidental();
      void draw(QPainter*) const override;
      bool isEditable() const override               { return true; }
      void startEdit(EditData&) override { setGenerated(false); }

      SymId symbol() const;
      Note* note() const                        { return (parent() && parent()->isNote()) ? toNote(parent()) : 0; }

      AccidentalBracket bracket() const         { return _bracket;     }
      void setBracket(AccidentalBracket val)    { _bracket = val;      }

      void setRole(AccidentalRole r)            { _role = r;              }

      bool isSmall() const                      { return m_isSmall;       }
      void setSmall(bool val)                   { m_isSmall = val;        }

      void undoSetSmall(bool val);

      void read(XmlReader&) override;
      void write(XmlWriter& xml) const override;

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid propertyId) const override;
      Pid propertyId(const QStringRef& xmlName) const override;
      QString propertyUserValue(Pid) const override;

      static AccidentalVal subtype2value(AccidentalType);             // return effective pitch offset
      static SymId subtype2symbol(AccidentalType);
      static const char* subtype2name(AccidentalType);
      static AccidentalType value2subtype(AccidentalVal);
      static AccidentalType name2subtype(const QString&);
      static bool isMicrotonal(AccidentalType t)  { return t > AccidentalType::FLAT3; }

      QString accessibleInfo() const override;
      };

extern AccidentalVal sym2accidentalVal(SymId id);

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::AccidentalRole);


#endif
