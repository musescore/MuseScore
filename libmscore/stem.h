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

#ifndef __STEM_H__
#define __STEM_H__

#include "element.h"

namespace Ms {

class Chord;

//---------------------------------------------------------
//   @@ Stem
///    Graphic representation of a note stem.
//---------------------------------------------------------

class Stem final : public Element {
      QLineF line;                  // p1 is attached to notehead
      qreal _lineWidth;
      qreal _userLen;
      qreal _len       { 0.0 };     // always positive

   public:
      Stem(Score* = 0);
      Stem &operator=(const Stem&) = delete;

      virtual Stem* clone() const override        { return new Stem(*this); }
      virtual ElementType type() const override   { return ElementType::STEM; }
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override    { return true; }
      virtual void layout() override;
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual void startEdit(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader& e) override;
      virtual bool readProperties(XmlReader&) override;
      virtual void reset() override;
      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid id) const override;

      Chord* chord() const            { return toChord(parent()); }
      bool up() const;

      qreal userLen() const           { return _userLen; }
      void setUserLen(qreal l)        { _userLen = l; }

      qreal lineWidth() const         { return _lineWidth; }
      void setLineWidth(qreal w)      { _lineWidth = w; }

      void setLen(qreal l);
      qreal len() const               { return _len; }

      QPointF hookPos() const;
      qreal stemLen() const;
      QPointF p2() const              { return line.p2(); }
      };


}     // namespace Ms
#endif

