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

//---------------------------------------------------------
//   AccidentalRole
//---------------------------------------------------------

enum class AccidentalRole : char {
      AUTO,               // layout created accidental
      USER                // user created accidental
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

      //Arel-Ezgi-Uzdilek (AEU)
      FLAT_SLASH2,
      FLAT_SLASH,
      SHARP_SLASH3,
      SHARP_SLASH2,

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
//   @P accType     enum  (Accidental.NONE, .SHARP, .FLAT, .SHARP2, .FLAT2, .NATURAL, .FLAT_SLASH, .FLAT_SLASH2, .MIRRORED_FLAT2, .MIRRORED_FLAT, .SHARP_SLASH, .SHARP_SLASH2, .SHARP_SLASH3, .SHARP_SLASH4, .SHARP_ARROW_UP, .SHARP_ARROW_DOWN, .FLAT_ARROW_UP, .FLAT_ARROW_DOWN, .NATURAL_ARROW_UP, .NATURAL_ARROW_DOWN, .SORI, .KORON) (read only)
//   @P hasBracket  bool
//   @P role        enum  (Accidental.AUTO, .USER) (read only)
//   @P small       bool
//---------------------------------------------------------

class Accidental : public Element {

#ifdef SCRIPT_INTERFACE
      Q_OBJECT
      Q_PROPERTY(int  accType     READ qmlAccidentalType)
      Q_PROPERTY(bool hasBracket  READ hasBracket  WRITE undoSetHasBracket)
      Q_PROPERTY(int  role        READ qmlRole)
      Q_PROPERTY(bool small       READ small       WRITE undoSetSmall)

   public:
      enum QmlAccidentalRole { AUTO, USER };
      enum QmlAccidentalType {
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

            //Arel-Ezgi-Uzdilek (AEU)
            FLAT_SLASH2,
            FLAT_SLASH,
            SHARP_SLASH3,
            SHARP_SLASH2,

            // Persian
            SORI,
            KORON,
            END
            };
      Q_ENUMS(QmlAccidentalRole QmlAccidentalType)
      int qmlAccidentalType() const { return int(_accidentalType); }
      int qmlRole() const           { return int(_role);           }
   private:
#endif

      QList<SymElement> el;
      AccidentalType _accidentalType;
      bool _hasBracket;
      bool _small;
      AccidentalRole _role;

   public:
      Accidental(Score* s = 0);
      virtual Accidental* clone() const override  { return new Accidental(*this); }
      virtual Element::Type type() const override { return Element::Type::ACCIDENTAL; }

      QString subtypeUserName() const;
      void setSubtype(const QString& s);
      void setAccidentalType(AccidentalType t)     { _accidentalType = t;    }

      AccidentalType accidentalType() const        { return _accidentalType; }
      AccidentalRole role() const                  { return _role;           }

      virtual int subtype() const override         { return (int)_accidentalType; }
      virtual QString subtypeName() const override { return QString(subtype2name(_accidentalType)); }

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override               { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) override { setGenerated(false); }

      SymId symbol() const;
      Note* note() const                  { return (parent() && parent()->isNote()) ? toNote(parent()) : 0; }

      bool hasBracket() const             { return _hasBracket;     }
      void setHasBracket(bool val)        { _hasBracket = val;      }

      void setRole(AccidentalRole r)      { _role = r;              }

      bool small() const                  { return _small;          }
      void setSmall(bool val)             { _small = val;           }

      void undoSetHasBracket(bool val);
      void undoSetSmall(bool val);

      virtual void read(XmlReader&) override;
      virtual void write(Xml& xml) const override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID propertyId) const override;

      static AccidentalVal subtype2value(AccidentalType);             // return effective pitch offset
      static const char* subtype2name(AccidentalType);
      static AccidentalType value2subtype(AccidentalVal);
      static AccidentalType name2subtype(const QString&);
      static bool isMicrotonal(AccidentalType t)  { return t > AccidentalType::SHARP2; }

      QString accessibleInfo() const override;
      };

}     // namespace Ms

#ifdef SCRIPT_INTERFACE
Q_DECLARE_METATYPE(Ms::Accidental::QmlAccidentalRole);
Q_DECLARE_METATYPE(Ms::Accidental::QmlAccidentalType);
#endif // SCRIPT_INTERFACE

Q_DECLARE_METATYPE(Ms::AccidentalRole);
Q_DECLARE_METATYPE(Ms::AccidentalType);


#endif

