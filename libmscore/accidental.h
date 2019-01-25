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

