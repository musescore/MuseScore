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

#ifndef __BOX_H__
#define __BOX_H__

/**
 \file
 Definition of HBox and VBox classes.
*/

#include "measurebase.h"
class QPainter;

namespace Ms {

class MuseScoreView;

//---------------------------------------------------------
//   @@ Box
///    virtual base class for frames "boxes"
//---------------------------------------------------------

class Box : public MeasureBase {
      Q_OBJECT

      Spatium _boxWidth             { Spatium(0) };  // only valid for HBox
      Spatium _boxHeight            { Spatium(0) };  // only valid for VBox
      qreal _topGap                 { 0.0   };       // distance from previous system (left border for hbox)
                                                     // initialized with StyleIdx::systemFrameDistance
      qreal _bottomGap              { 0.0   };       // distance to next system (right border for hbox)
                                                     // initialized with StyleIdx::frameSystemDistance
      qreal _leftMargin             { 0.0   };
      qreal _rightMargin            { 0.0   };       // inner margins in metric mm
      qreal _topMargin              { 0.0   };
      qreal _bottomMargin           { 0.0   };
      bool editMode                 { false };
      PropertyStyle topGapStyle     { PropertyStyle::STYLED };
      PropertyStyle bottomGapStyle  { PropertyStyle::STYLED };
      qreal dragX;                        // used during drag of hbox

   public:
      Box(Score*);
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual bool edit(MuseScoreView*, Grip grip, int key, Qt::KeyboardModifiers, const QString& s) override;
      virtual void editDrag(const EditData&) override;
      virtual void endEdit() override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 1; }
      virtual void layout() override;
      virtual void write(Xml&) const override;
      virtual void write(Xml& xml, int, bool) const override { write(xml); }
      virtual void writeProperties(Xml&) const override;
      virtual bool readProperties(XmlReader&) override;
      virtual void read(XmlReader&) override;
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual void add(Element* e) override;

      Spatium boxWidth() const        { return _boxWidth;     }
      void setBoxWidth(Spatium val)   { _boxWidth = val;      }
      Spatium boxHeight() const       { return _boxHeight;    }
      void setBoxHeight(Spatium val)  { _boxHeight = val;     }
      qreal leftMargin() const        { return _leftMargin;   }
      qreal rightMargin() const       { return _rightMargin;  }
      qreal topMargin() const         { return _topMargin;    }
      qreal bottomMargin() const      { return _bottomMargin; }
      void setLeftMargin(qreal val)   { _leftMargin = val;    }
      void setRightMargin(qreal val)  { _rightMargin = val;   }
      void setTopMargin(qreal val)    { _topMargin = val;     }
      void setBottomMargin(qreal val) { _bottomMargin = val;  }
      qreal topGap() const            { return _topGap;       }
      void setTopGap(qreal val)       { _topGap = val;        }
      qreal bottomGap() const         { return _bottomGap;    }
      void setBottomGap(qreal val)    { _bottomGap = val;     }
      void copyValues(Box* origin);

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID id) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;
      virtual StyleIdx getPropertyStyle(P_ID id) const override;
      };

//---------------------------------------------------------
//   @@ HBox
///    horizontal frame
//---------------------------------------------------------

class HBox : public Box {
      Q_OBJECT

   public:
      HBox(Score* score);
      virtual ~HBox() {}
      virtual HBox* clone() const override        { return new HBox(*this); }
      virtual Element::Type type() const override { return Element::Type::HBOX;       }

      virtual void layout() override;

      virtual QRectF drag(EditData*) override;
      virtual void endEditDrag() override;
      void layout2();
      virtual bool isMovable() const override;
      };

//---------------------------------------------------------
//   @@ VBox
///    vertical frame
//---------------------------------------------------------

class VBox : public Box {
      Q_OBJECT

   public:
      VBox(Score* score);
      virtual ~VBox() {}
      virtual VBox* clone() const override        { return new VBox(*this);           }
      virtual Element::Type type() const override { return Element::Type::VBOX;       }

      virtual void layout() override;

      virtual QPointF getGrip(Grip) const override;
      virtual void setGrip(Grip, const QPointF&) override;
      };

//---------------------------------------------------------
//   @@ FBox
///    frame containing fret diagrams
//---------------------------------------------------------

class FBox : public VBox {
      Q_OBJECT

   public:
      FBox(Score* score) : VBox(score) {}
      virtual ~FBox() {}
      virtual FBox* clone() const override        { return new FBox(*this); }
      virtual Element::Type type() const override { return Element::Type::FBOX;       }

      virtual void layout() override;
      virtual void add(Element*) override;
      };


}     // namespace Ms
#endif

