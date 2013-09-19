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

class BarLine;
class MuseScoreView;
class Text;

//---------------------------------------------------------
//   @@ Box
///    virtual base class for frames "boxes"
//---------------------------------------------------------

class Box : public MeasureBase {
      Q_OBJECT

      Spatium _boxWidth;   // only valid for HBox
      Spatium _boxHeight;  // only valid for VBox
      qreal _topGap;       // distance from previous system (left border for hbox)
                           // initialized with ST_systemFrameDistance
      qreal _bottomGap;    // distance to next system (right border for hbox)
                           // initialized with ST_frameSystemDistance
      qreal _leftMargin, _rightMargin;   // inner margins in metric mm
      qreal _topMargin, _bottomMargin;
      bool editMode;
      qreal dragX;            // used during drag of hbox

   public:
      Box(Score*);
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s) override;
      virtual void editDrag(const EditData&) override;
      virtual void endEdit() override;
      virtual void updateGrips(int* grips, QRectF*) const override;
      virtual void layout() override;
      virtual void write(Xml&) const override;
      virtual void write(Xml& xml, int, bool) const override { write(xml); }
      virtual void read(XmlReader&) override;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const override;
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

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };

//---------------------------------------------------------
//   @@ HBox
///   horizontal frame
//---------------------------------------------------------

class HBox : public Box {
      Q_OBJECT

   public:
      HBox(Score* score);
      ~HBox() {}
      virtual HBox* clone() const      { return new HBox(*this); }
      virtual ElementType type() const { return HBOX;       }

      virtual void layout();

      virtual QRectF drag(const EditData& s);
      virtual void endEditDrag();
      void layout2();
      virtual bool isMovable() const;
      };

//---------------------------------------------------------
//   @@ VBox
///   vertical frame
//---------------------------------------------------------

class VBox : public Box {
      Q_OBJECT

   public:
      VBox(Score* score);
      ~VBox() {}
      virtual VBox* clone() const      { return new VBox(*this); }
      virtual ElementType type() const { return VBOX;       }

      virtual void layout();

      virtual QPointF getGrip(int) const;
      virtual void setGrip(int, const QPointF&);
      };

//---------------------------------------------------------
//   @@ FBox
///    frame containing fret diagrams
//---------------------------------------------------------

class FBox : public VBox {
      Q_OBJECT

   public:
      FBox(Score* score) : VBox(score) {}
      ~FBox() {}
      virtual FBox* clone() const      { return new FBox(*this); }
      virtual ElementType type() const { return FBOX;       }

      virtual void layout();
      void add(Element*);
      };


}     // namespace Ms
#endif

