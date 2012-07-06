//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: image.h 5510 2012-03-30 14:51:09Z wschweer $
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

#include "config.h"
#include "bsymbol.h"

class ImageStoreItem;

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

class Image : public BSymbol {
      Q_OBJECT

      void write(Xml& xml, P_ID id) const;

   protected:
      ImageStoreItem* _storeItem;
      QString _path;
      mutable QPixmap buffer;       ///< cached rendering
      QSizeF _size;                 // in mm or spatium units
      bool _lockAspectRatio;
      bool _autoScale;              ///< fill parent frame
      bool _sizeIsSpatium;
      mutable bool _dirty;

      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      void draw(QPainter*, QSize size) const;
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int /*grip*/) const { return QPointF(); }
      virtual QSizeF imageSize() const = 0;

   public:
      Image(Score* = 0);
      Image(const Image&);
      ~Image();
      virtual ElementType type() const { return IMAGE; }
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      bool load(const QString& s);
      virtual void layout();

      void setSize(const QSizeF& s)     { _size = s;    }
      QSizeF size() const               { return _size; }
      bool lockAspectRatio() const      { return _lockAspectRatio; }
      void setLockAspectRatio(bool v)   { _lockAspectRatio = v;  }
      bool autoScale() const            { return _autoScale;     }
      void setAutoScale(bool v)         { _autoScale = v;        }
      ImageStoreItem* storeItem() const { return _storeItem;     }
      bool sizeIsSpatium() const        { return _sizeIsSpatium; }
      void setSizeIsSpatium(bool val)   { _sizeIsSpatium = val;  }

      QSizeF scale() const;
      void setScale(const QSizeF&);
      QSizeF scaleForSize(const QSizeF&) const;
      QSizeF sizeForScale(const QSizeF&) const;

      QVariant getProperty(P_ID ) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };

//---------------------------------------------------------
//   RasterImage
//---------------------------------------------------------

class RasterImage : public Image {
      QImage doc;

   public:
      RasterImage(Score* s) : Image(s) {}
      ~RasterImage() {}
      virtual RasterImage* clone() const { return new RasterImage(*this); }
      virtual void draw(QPainter*) const;
      virtual QSizeF imageSize() const { return doc.size(); }
      virtual void layout();
      };

//---------------------------------------------------------
//   SvgImage
//---------------------------------------------------------

class SvgImage : public Image {
      QSvgRenderer* doc;

   public:
      SvgImage(Score*);
      ~SvgImage();
      virtual SvgImage* clone() const;
      virtual void draw(QPainter*) const;
      virtual QSizeF imageSize() const { return doc->defaultSize(); }
      virtual void layout();
      };

#endif

