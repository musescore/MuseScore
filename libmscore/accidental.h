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
      void draw(QPainter*) const override;
      bool isEditable() const override               { return true; }
      void startEdit(EditData&) override { setGenerated(false); }

      SymId symbol() const;
      Note* note() const                        { return (parent() && parent()->isNote()) ? toNote(parent()) : 0; }

      AccidentalBracket bracket() const         { return _bracket;     }
      void setBracket(AccidentalBracket val)    { _bracket = val;      }

      void setRole(AccidentalRole r)            { _role = r;              }

      bool small() const                        { return _small;          }
      void setSmall(bool val)                   { _small = val;           }

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
      static bool isMicrotonal(AccidentalType t)  { return t > AccidentalType::FLAT2; }

      QString accessibleInfo() const override;
      };

extern AccidentalVal sym2accidentalVal(SymId id);

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::AccidentalRole);


#endif

