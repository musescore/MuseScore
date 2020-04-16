//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "bsymbol.h"

namespace Ms {

class ImageStoreItem;

enum class ImageType : char { NONE, RASTER, SVG };

//---------------------------------------------------------
//   @@ Image
//---------------------------------------------------------

class Image final : public BSymbol {
      union {
            QImage*       rasterDoc;
            QSvgRenderer* svgDoc;
            };
      ImageType imageType;

      QSizeF pixel2size(const QSizeF& s) const;
      QSizeF size2pixel(const QSizeF& s) const;

   protected:
      ImageStoreItem* _storeItem;
      QString _storePath;           // the path of the img in the ImageStore
      QString _linkPath;            // the path of an external linked img
      bool _linkIsValid;            // whether _linkPath file exists or not
      mutable QPixmap buffer;       ///< cached rendering
      QSizeF _size;                 // in mm or spatium units
      bool _lockAspectRatio;
      bool _autoScale;              ///< fill parent frame
      bool _sizeIsSpatium;
      mutable bool _dirty;

      bool isEditable() const override { return true; }
      void startEditDrag(EditData&) override;
      void editDrag(EditData& ed) override;
      QVector<QLineF> gripAnchorLines(Grip) const override { return QVector<QLineF>(); }

   public:
      Image(Score* = 0);
      Image(const Image&);
      ~Image();

      Image* clone() const override     { return new Image(*this); }
      ElementType type() const override { return ElementType::IMAGE; }
      void write(XmlWriter& xml) const override;
      void read(XmlReader&) override;
      bool load(const QString& s);
      bool loadFromData(const QString&, const QByteArray&);
      void layout() override;
      void draw(QPainter*) const override;

      void setSize(const QSizeF& s)      { _size = s;    }
      QSizeF size() const                { return _size; }
      bool lockAspectRatio() const       { return _lockAspectRatio; }
      void setLockAspectRatio(bool v)    { _lockAspectRatio = v;  }
      bool autoScale() const             { return _autoScale;     }
      void setAutoScale(bool v)          { _autoScale = v;        }
      ImageStoreItem* storeItem() const  { return _storeItem;     }
      bool sizeIsSpatium() const         { return _sizeIsSpatium; }
      void setSizeIsSpatium(bool val)    { _sizeIsSpatium = val;  }

      QVariant getProperty(Pid ) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid id) const override;

      QSizeF imageSize() const;

      void setImageType(ImageType);
      ImageType getImageType() const { return imageType; }
      bool isValid() const           { return rasterDoc || svgDoc; }

      Element::EditBehavior normalModeEditBehavior() const override { return Element::EditBehavior::Edit; }
      int gripsCount() const override { return 2; }
      Grip initialEditModeGrip() const override { return Grip(1); }
      Grip defaultGrip() const override { return Grip(1); }
      std::vector<QPointF> gripsPositions(const EditData&) const override;
      };


}     // namespace Ms
#endif

