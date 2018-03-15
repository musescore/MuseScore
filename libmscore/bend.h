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

#ifndef __BEND_H__
#define __BEND_H__

#include "element.h"
#include "pitchvalue.h"
#include "property.h"
#include "style.h"

namespace Ms {

//---------------------------------------------------------
//   @@ Bend
//---------------------------------------------------------

class Bend final : public Element {
      QString _fontFace;
      qreal _fontSize;
      bool _fontBold;
      bool _fontItalic;
      bool _fontUnderline;

      static constexpr std::array<StyledProperty,6> _styledProperties {{
            { StyleIdx::bendFontFace,      P_ID::FONT_FACE },
            { StyleIdx::bendFontSize,      P_ID::FONT_SIZE },
            { StyleIdx::bendFontBold,      P_ID::FONT_BOLD },
            { StyleIdx::bendFontItalic,    P_ID::FONT_ITALIC },
            { StyleIdx::bendFontUnderline, P_ID::FONT_UNDERLINE },
            { StyleIdx::NOSTYLE,           P_ID::END }      // end of list marker
            }};

      PropertyFlags _propertyFlagsList[5] = {
            PropertyFlags::STYLED,
            PropertyFlags::STYLED,
            PropertyFlags::STYLED,
            PropertyFlags::STYLED,
            PropertyFlags::STYLED,
            };

      bool _playBend     { true };
      QList<PitchValue> _points;
      qreal _lw;

      QPointF notePos;
      qreal noteWidth;

      QFont font(qreal) const;
      bool readStyledProperty(XmlReader& e, const QStringRef& tag);     // helper function

   protected:
      virtual const StyledProperty* styledProperties() const override { return _styledProperties.data(); }
      virtual PropertyFlags* propertyFlagsList()       override       { return _propertyFlagsList; }

   public:
      Bend(Score* s);
      virtual Bend* clone() const override        { return new Bend(*this); }
      virtual ElementType type() const override   { return ElementType::BEND; }
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader& e) override;
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }
      bool playBend() const                      { return _playBend; }
      void setPlayBend(bool v)                   { _playBend = v;    }

      // property methods
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      virtual void reset() override;
      };


}     // namespace Ms
#endif

