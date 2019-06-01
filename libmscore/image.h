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

      virtual bool isEditable() const override { return true; }
      virtual void startEdit(EditData&) override;
      virtual void startEditDrag(EditData&) override;
      virtual void editDrag(EditData& ed) override;
      virtual void endEditDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;
      virtual QPointF gripAnchor(Grip) const override { return QPointF(); }

   public:
      Image(Score* = 0);
      Image(const Image&);
      ~Image();
      virtual Image* clone() const override     { return new Image(*this); }
      virtual ElementType type() const override { return ElementType::IMAGE; }
      virtual void write(XmlWriter& xml) const override;
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

      QVariant getProperty(Pid ) const;
      bool setProperty(Pid propertyId, const QVariant&);
      QVariant propertyDefault(Pid id) const;

      QSizeF imageSize() const;

      void setImageType(ImageType);
      ImageType getImageType() const { return imageType; }
      bool isValid() const           { return rasterDoc || svgDoc; }
      };


}     // namespace Ms
#endif

