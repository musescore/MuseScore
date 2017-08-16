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
#include "property.h"

namespace Ms {

class MuseScoreView;

//---------------------------------------------------------
//   @@ Box
///    virtual base class for frames "boxes"
//---------------------------------------------------------

class Box : public MeasureBase {
      Q_GADGET

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
      PropertyFlags topGapStyle     { PropertyFlags::STYLED };
      PropertyFlags bottomGapStyle  { PropertyFlags::STYLED };
      qreal dragX;                        // used during drag of hbox

   public:
      Box(Score*);
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override { return true; }

      virtual void startEdit(EditData&) override;
      virtual bool edit(EditData&) override;
      virtual void startEditDrag(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual void endEdit(EditData&) override;

      virtual void updateGrips(EditData&) const override;
      virtual void layout() override;
      virtual void write(XmlWriter&) const override;
      virtual void write(XmlWriter& xml, int, bool, bool) const override { write(xml); }
      virtual void writeProperties(XmlWriter&) const override;
      virtual bool readProperties(XmlReader&) override;
      virtual void read(XmlReader&) override;
      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;
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
      virtual PropertyFlags propertyFlags(P_ID id) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;
      virtual StyleIdx getPropertyStyle(P_ID id) const override;
      };

//---------------------------------------------------------
//   @@ HBox
///    horizontal frame
//---------------------------------------------------------

class HBox : public Box {
      Q_GADGET

      bool _createSystemHeader { true };

   public:
      HBox(Score* score);
      virtual ~HBox() {}
      virtual HBox* clone() const override        { return new HBox(*this); }
      virtual ElementType type() const override { return ElementType::HBOX;       }

      virtual void layout() override;

      virtual QRectF drag(EditData&) override;
      virtual void endEditDrag(EditData&) override;
      void layout2();
      virtual bool isMovable() const override;
      virtual void computeMinWidth();

      bool createSystemHeader() const      { return _createSystemHeader; }
      void setCreateSystemHeader(bool val) { _createSystemHeader = val;  }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };

//---------------------------------------------------------
//   @@ VBox
///    vertical frame
//---------------------------------------------------------

class VBox : public Box {
      Q_GADGET

   public:
      VBox(Score* score);
      virtual ~VBox() {}
      virtual VBox* clone() const override        { return new VBox(*this);           }
      virtual ElementType type() const override { return ElementType::VBOX;       }

      virtual void layout() override;

      };

//---------------------------------------------------------
//   @@ FBox
///    frame containing fret diagrams
//---------------------------------------------------------

class FBox : public VBox {
      Q_GADGET

   public:
      FBox(Score* score) : VBox(score) {}
      virtual ~FBox() {}
      virtual FBox* clone() const override        { return new FBox(*this); }
      virtual ElementType type() const override { return ElementType::FBOX;       }

      virtual void layout() override;
      virtual void add(Element*) override;
      };


}     // namespace Ms
#endif

