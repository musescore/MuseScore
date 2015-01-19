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

class Image : public BSymbol {
      union {
            QImage*       rasterDoc;
            QSvgRenderer* svgDoc;
            };
      ImageType imageType;
      Q_OBJECT

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

      virtual bool isEditable() const override { return true; }
      virtual void editDrag(const EditData&) override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 2; }
      virtual QPointF gripAnchor(Grip) const override { return QPointF(); }

   public:
      Image(Score* = 0);
      Image(const Image&);
      ~Image();
      virtual Image* clone() const override       { return new Image(*this); }
      virtual Element::Type type() const override { return Element::Type::IMAGE; }
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      bool load(const QString& s);
      bool loadFromData(const QString&, const QByteArray&);
      virtual void layout() override;
      virtual void draw(QPainter*) const override;

      void setSize(const QSizeF& s)      { _size = s;    }
      QSizeF size() const                { return _size; }
      bool lockAspectRatio() const       { return _lockAspectRatio; }
      void setLockAspectRatio(bool v)    { _lockAspectRatio = v;  }
      bool autoScale() const             { return _autoScale;     }
      void setAutoScale(bool v)          { _autoScale = v;        }
      ImageStoreItem* storeItem() const  { return _storeItem;     }
      bool sizeIsSpatium() const         { return _sizeIsSpatium; }
      void setSizeIsSpatium(bool val)    { _sizeIsSpatium = val;  }

      QSizeF scale() const;
      void setScale(const QSizeF&);
      QSizeF scaleForSize(const QSizeF&) const;
      QSizeF sizeForScale(const QSizeF&) const;

      QVariant getProperty(P_ID ) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;

      QSizeF imageSize() const;
      qreal scaleFactor() const;

      void setImageType(ImageType);
      bool isValid() const           { return rasterDoc || svgDoc; }
      };


}     // namespace Ms
#endif

