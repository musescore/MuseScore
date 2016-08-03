//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __REST_H__
#define __REST_H__

#include "chordrest.h"

namespace Ms {

class TDuration;
enum class SymId;

//---------------------------------------------------------
//    @@ Rest
///     This class implements a rest.
//    @P isFullMeasure  bool  (read only)
//---------------------------------------------------------

class Rest : public ChordRest {
      Q_OBJECT
      Q_PROPERTY(bool  isFullMeasure  READ isFullMeasureRest)

      // values calculated by layout:
      SymId _sym;
      int dotline    { -1  };       // depends on rest symbol
      qreal _mmWidth { 0.0 };       // width of multi measure rest
      bool _gap;                    ///< invisible and not selectable for user

      virtual QRectF drag(EditData*) override;
      virtual qreal upPos()   const override;
      virtual qreal downPos() const override;
      virtual qreal centerX() const override;
      virtual void setUserOff(const QPointF& o) override;

   protected:
      ElementList _el;              ///< symbols or images

   public:
      Rest(Score* s = 0);
      Rest(Score*, const TDuration&);
      Rest(const Rest&, bool link = false);
      ~Rest();

      virtual Element::Type type() const override { return Element::Type::REST; }
      Rest &operator=(const Rest&) = delete;

      virtual Rest* clone() const override        { return new Rest(*this, false); }
      virtual Element* linkedClone()              { return new Rest(*this, true); }
      virtual Measure* measure() const override   { return parent() ? (Measure*)(parent()->parent()) : 0; }
      virtual qreal mag() const override;
      virtual void draw(QPainter*) const override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual void layout() override;

      bool isGap() const               { return _gap;     }
      virtual void setGap(bool v)      { _gap = v;        }

      virtual void reset() override;

      virtual void add(Element*);
      virtual void remove(Element*);

      virtual void read(XmlReader&) override;
      virtual void write(Xml& xml) const override;

      void setMMWidth(qreal val);
      qreal mmWidth() const        { return _mmWidth; }
      SymId getSymbol(TDuration::DurationType type, int line, int lines,  int* yoffset);

      int getDotline() const   { return dotline; }
      static int getDotline(TDuration::DurationType durationType);
      SymId sym() const        { return _sym;    }
      int computeLineOffset();
      bool accent();
      void setAccent(bool flag);

      virtual int upLine() const;
      virtual int downLine() const;
      virtual QPointF stemPos() const;
      virtual qreal stemPosX() const;
      virtual QPointF stemPosBeam() const;

      ElementList el()                            { return _el; }
      const ElementList el() const                { return _el; }

      virtual bool setProperty(P_ID propertyId, const QVariant& v) override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual QVariant propertyDefault(P_ID) const override;

      virtual QString accessibleInfo() const override;
      virtual QString screenReaderInfo() const override;
      Shape shape() const override;
      };

}     // namespace Ms
#endif

