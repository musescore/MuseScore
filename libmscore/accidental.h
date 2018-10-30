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

#include "config.h"
#include "element.h"

namespace Ms {

class Note;
enum class SymId;
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
      BRACKET
      };

//---------------------------------------------------------
//   AccidentalType
//---------------------------------------------------------
// NOTE: keep this in sync with with accList array
enum class AccidentalType : char {
      NONE,
      FLAT,
      NATURAL,
      SHARP,
      SHARP2,
      FLAT2,
      //SHARP3,
      //FLAT3,
      NATURAL_FLAT,
      NATURAL_SHARP,
      SHARP_SHARP,

      // Gould arrow quartertone
      FLAT_ARROW_UP,
      FLAT_ARROW_DOWN,
      NATURAL_ARROW_UP,
      NATURAL_ARROW_DOWN,
      SHARP_ARROW_UP,
      SHARP_ARROW_DOWN,
      SHARP2_ARROW_UP,
      SHARP2_ARROW_DOWN,
      FLAT2_ARROW_UP,
      FLAT2_ARROW_DOWN,

      // Stein-Zimmermann
      MIRRORED_FLAT,
      MIRRORED_FLAT2,
      SHARP_SLASH,
      SHARP_SLASH4,

      // Arel-Ezgi-Uzdilek (AEU)
      FLAT_SLASH2,
      FLAT_SLASH,
      SHARP_SLASH3,
      SHARP_SLASH2,

      // Extended Helmholtz-Ellis accidentals (just intonation)
      DOUBLE_FLAT_ONE_ARROW_DOWN,
      FLAT_ONE_ARROW_DOWN,
      NATURAL_ONE_ARROW_DOWN,
      SHARP_ONE_ARROW_DOWN,
      DOUBLE_SHARP_ONE_ARROW_DOWN,
      DOUBLE_FLAT_ONE_ARROW_UP,

      FLAT_ONE_ARROW_UP,
      NATURAL_ONE_ARROW_UP,
      SHARP_ONE_ARROW_UP,
      DOUBLE_SHARP_ONE_ARROW_UP,
      DOUBLE_FLAT_TWO_ARROWS_DOWN,
      FLAT_TWO_ARROWS_DOWN,

      NATURAL_TWO_ARROWS_DOWN,
      SHARP_TWO_ARROWS_DOWN,
      DOUBLE_SHARP_TWO_ARROWS_DOWN,
      DOUBLE_FLAT_TWO_ARROWS_UP,
      FLAT_TWO_ARROWS_UP,
      NATURAL_TWO_ARROWS_UP,

      SHARP_TWO_ARROWS_UP,
      DOUBLE_SHARP_TWO_ARROWS_UP,
      DOUBLE_FLAT_THREE_ARROWS_DOWN,
      FLAT_THREE_ARROWS_DOWN,
      NATURAL_THREE_ARROWS_DOWN,
      SHARP_THREE_ARROWS_DOWN,

      DOUBLE_SHARP_THREE_ARROWS_DOWN,
      DOUBLE_FLAT_THREE_ARROWS_UP,
      FLAT_THREE_ARROWS_UP,
      NATURAL_THREE_ARROWS_UP,
      SHARP_THREE_ARROWS_UP,
      DOUBLE_SHARP_THREE_ARROWS_UP,

      LOWER_ONE_SEPTIMAL_COMMA,
      RAISE_ONE_SEPTIMAL_COMMA,
      LOWER_TWO_SEPTIMAL_COMMAS,
      RAISE_TWO_SEPTIMAL_COMMAS,
      LOWER_ONE_UNDECIMAL_QUARTERTONE,
      RAISE_ONE_UNDECIMAL_QUARTERTONE,

      LOWER_ONE_TRIDECIMAL_QUARTERTONE,
      RAISE_ONE_TRIDECIMAL_QUARTERTONE,

      DOUBLE_FLAT_EQUAL_TEMPERED,
      FLAT_EQUAL_TEMPERED,
      NATURAL_EQUAL_TEMPERED,
      SHARP_EQUAL_TEMPERED,
      DOUBLE_SHARP_EQUAL_TEMPERED,
      QUARTER_FLAT_EQUAL_TEMPERED,
      QUARTER_SHARP_EQUAL_TEMPERED,

      // Persian
      SORI,
      KORON,
      END
      };

//---------------------------------------------------------
//   SymElement
//---------------------------------------------------------

struct SymElement {
      SymId sym;
      qreal x;
      SymElement(SymId _sym, qreal _x) : sym(_sym), x(_x) {}
      };

//---------------------------------------------------------
//   @@ Accidental
//   @P role        enum  (Accidental.AUTO, .USER) (read only)
//   @P small       bool
//---------------------------------------------------------

class Accidental final : public Element {
      QList<SymElement> el;
      AccidentalType _accidentalType { AccidentalType::NONE };
      bool _small                    { false                   };
      AccidentalBracket _bracket     { AccidentalBracket::NONE };
      AccidentalRole _role           { AccidentalRole::AUTO    };

   public:
      Accidental(Score* s = 0);
      virtual Accidental* clone() const override  { return new Accidental(*this); }
      virtual ElementType type() const override   { return ElementType::ACCIDENTAL; }

      QString subtypeUserName() const;
      void setSubtype(const QString& s);
      void setAccidentalType(AccidentalType t)     { _accidentalType = t;    }

      AccidentalType accidentalType() const        { return _accidentalType; }
      AccidentalRole role() const                  { return _role;           }

      virtual int subtype() const override         { return (int)_accidentalType; }
      virtual QString subtypeName() const override { return QString(subtype2name(_accidentalType)); }

      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override               { return true; }
      virtual void startEdit(EditData&) override { setGenerated(false); }

      SymId symbol() const;
      Note* note() const                        { return (parent() && parent()->isNote()) ? toNote(parent()) : 0; }

      AccidentalBracket bracket() const         { return _bracket;     }
      void setBracket(AccidentalBracket val)    { _bracket = val;      }

      void setRole(AccidentalRole r)            { _role = r;              }

      bool small() const                        { return _small;          }
      void setSmall(bool val)                   { _small = val;           }

      void undoSetSmall(bool val);

      virtual void read(XmlReader&) override;
      virtual void write(XmlWriter& xml) const override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid propertyId) const override;
      virtual QString propertyUserValue(Pid) const;

      static AccidentalVal subtype2value(AccidentalType);             // return effective pitch offset
      static const char* subtype2name(AccidentalType);
      static AccidentalType value2subtype(AccidentalVal);
      static AccidentalType name2subtype(const QString&);
      static bool isMicrotonal(AccidentalType t)  { return t > AccidentalType::FLAT2; }

      QString accessibleInfo() const override;
      };

extern AccidentalVal sym2accidentalVal(SymId id);

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::AccidentalRole);
Q_DECLARE_METATYPE(Ms::AccidentalType);


#endif

