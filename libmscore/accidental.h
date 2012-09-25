//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: accidental.h 5242 2012-01-23 17:25:56Z wschweer $
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
#include "mscore.h"

class Note;
class QPainter;

//---------------------------------------------------------
//   SymElement
//---------------------------------------------------------

struct SymElement {
      int sym;
      qreal x;
      SymElement(int _sym, qreal _x) : sym(_sym), x(_x) {}
      };

//---------------------------------------------------------
//   AccidentalRole
//---------------------------------------------------------

enum AccidentalRole {
      ACC_AUTO,               // layout created accidental
      ACC_USER                // user created accidental
      };

//---------------------------------------------------------
//   @@ Accidental
//    @P hasBracket bool
//    @P small      bool
//---------------------------------------------------------

class Accidental : public Element {
      Q_OBJECT
      Q_PROPERTY(bool hasBracket READ hasBracket WRITE undoSetHasBracket)
      Q_PROPERTY(bool small      READ small      WRITE undoSetSmall)

      QList<SymElement> el;
      AccidentalType _subtype;
      bool _hasBracket;
      bool _small;
      AccidentalRole _role;

   public:
      Accidental(Score* s = 0);
      virtual Accidental* clone() const     { return new Accidental(*this); }
      virtual ElementType type() const      { return ACCIDENTAL; }

      const char* subtypeUserName() const;
      void setSubtype(const QString& s);
      void setSubtype(AccidentalType t)     { _subtype = t;    }
      AccidentalType subtype() const        { return _subtype; }

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const                    { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) { setGenerated(false); }

      int symbol() const;
      Note* note() const                  { return (Note*)parent(); }

      bool hasBracket() const             { return _hasBracket;     }
      void setHasBracket(bool val)        { _hasBracket = val;      }
      void undoSetHasBracket(bool val);

      AccidentalRole role() const         { return _role;           }
      void setRole(AccidentalRole r)      { _role = r;              }

      bool small() const                  { return _small;          }
      void setSmall(bool val)             { _small = val;           }
      void undoSetSmall(bool val);

      virtual void read(const QDomElement&);
      virtual void write(Xml& xml) const;

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);

      static AccidentalVal subtype2value(AccidentalType);             // return effective pitch offset
      static const char* subtype2name(AccidentalType);
      static AccidentalType value2subtype(AccidentalVal);
      static AccidentalType name2subtype(const QString&);
      };

//---------------------------------------------------------
//   @@ AccidentalBracket
//    used as icon in palette
//---------------------------------------------------------

class AccidentalBracket : public Compound {
      Q_OBJECT

   public:
      AccidentalBracket(Score*);
      virtual AccidentalBracket* clone() const { return new AccidentalBracket(*this); }
      virtual ElementType type() const         { return ACCIDENTAL_BRACKET; }
      };

#endif

