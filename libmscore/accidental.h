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

#include "element.h"

class QPainter;

namespace Ms {

class Note;
enum class SymId;

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
//   @P hasBracket  bool
//   @P small       bool
//   @P acctype     Ms::Accidental::Type  (NONE, SHARP, FLAT, SHARP2, FLAT2, NATURAL, ...) (read only)
//   @P role        Ms::Accidental::Role  (AUTO, USER) (read only)
//---------------------------------------------------------

class Accidental : public Element {
   public:
      enum class Role : char {
            AUTO,               // layout created accidental
            USER                // user created accidental
            };
      enum class Type : char {
            NONE,
            SHARP,
            FLAT,
            SHARP2,
            FLAT2,
            NATURAL,

            FLAT_SLASH,
            FLAT_SLASH2,
            MIRRORED_FLAT2,
            MIRRORED_FLAT,
            MIRRORED_FLAT_SLASH,
            FLAT_FLAT_SLASH,

            SHARP_SLASH,
            SHARP_SLASH2,
            SHARP_SLASH3,
            SHARP_SLASH4,

            SHARP_ARROW_UP,
            SHARP_ARROW_DOWN,
            SHARP_ARROW_BOTH,
            FLAT_ARROW_UP,
            FLAT_ARROW_DOWN,
            FLAT_ARROW_BOTH,
            NATURAL_ARROW_UP,
            NATURAL_ARROW_DOWN,
            NATURAL_ARROW_BOTH,
            SORI,
            KORON,
            END
            };

   private:
      Q_OBJECT
      Q_PROPERTY(bool                 hasBracket  READ hasBracket  WRITE undoSetHasBracket)
      Q_PROPERTY(bool                 small       READ small       WRITE undoSetSmall)
      Q_PROPERTY(Ms::Accidental::Type accType     READ accidentalType)
      Q_PROPERTY(Ms::Accidental::Role role        READ role)
      Q_ENUMS(Type)
      Q_ENUMS(Role)

      QList<SymElement> el;
      Type _accidentalType;
      bool _hasBracket;
      bool _small;
      Role _role;

   public:
      Accidental(Score* s = 0);
      virtual Accidental* clone() const override  { return new Accidental(*this); }
      virtual Element::Type type() const override { return Element::Type::ACCIDENTAL; }

      const char* subtypeUserName() const;
      void setSubtype(const QString& s);
      void setAccidentalType(Type t)               { _accidentalType = t;    }
      Type accidentalType() const                  { return _accidentalType; }
      virtual int subtype() const override         { return (int)_accidentalType; }
      virtual QString subtypeName() const override { return QString(subtype2name(_accidentalType)); }

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override               { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) override { setGenerated(false); }

      SymId symbol() const;
      Note* note() const  { return (parent() && parent()->type() == Element::Type::NOTE) ? (Note*)parent() : 0; }

      bool hasBracket() const             { return _hasBracket;     }
      void setHasBracket(bool val)        { _hasBracket = val;      }
      void undoSetHasBracket(bool val);

      Role role() const                   { return _role;           }
      void setRole(Role r)                { _role = r;              }

      bool small() const                  { return _small;          }
      void setSmall(bool val)             { _small = val;           }
      void undoSetSmall(bool val);

      virtual void read(XmlReader&) override;
      virtual void write(Xml& xml) const override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID propertyId) const override;

      static AccidentalVal subtype2value(Type);             // return effective pitch offset
      static const char* subtype2name(Type);
      static Type value2subtype(AccidentalVal);
      static Type name2subtype(const QString&);

      QString accessibleInfo() override;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Accidental::Role);
Q_DECLARE_METATYPE(Ms::Accidental::Type);

#endif

