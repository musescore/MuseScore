//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __GLISSANDO_H__
#define __GLISSANDO_H__

#include "element.h"

namespace Ms {

class Note;

//---------------------------------------------------------
//   @@ Glissando
//   @P glissandoType  Ms::Glissando::Type (STRAIGHT, WAVY)
//   @P text           QString
//   @P showText       bool
//---------------------------------------------------------

class Glissando : public Element {
      Q_OBJECT

      Q_PROPERTY(Ms::Glissando::Type glissandoType READ glissandoType  WRITE undoSetGlissandoType)
      Q_PROPERTY(QString text                      READ text     WRITE undoSetText)
      Q_PROPERTY(bool showText                     READ showText WRITE undoSetShowText)
      Q_ENUMS(Type)

  public:
      enum class Type : char {
            STRAIGHT, WAVY
            };

   private:
      Type _glissandoType;
      QLineF line;
      QString _text;
      bool _showText;

   public:
      Glissando(Score* s);
      Glissando(const Glissando&);

      virtual Glissando* clone() const override       { return new Glissando(*this); }
      virtual Element::Type type() const override     { return Element::Type::GLISSANDO; }
      Type glissandoType() const             { return _glissandoType; }
      void setGlissandoType(Type v)          { _glissandoType = v;    }
      virtual Space space() const override;

      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;

      void setSize(const QSizeF&);        // used for palette

      QString text() const           { return _text;     }
      void setText(const QString& t) { _text = t;        }
      bool showText() const          { return _showText; }
      void setShowText(bool v)       { _showText = v;    }

      void undoSetGlissandoType(Type);
      void undoSetText(const QString&);
      void undoSetShowText(bool);

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Glissando::Type);

#endif

