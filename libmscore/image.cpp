//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2007-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "image.h"
#include "xml.h"
#include "score.h"
#include "undo.h"
#include "mscore.h"
#include "imageStore.h"

namespace Ms {

//---------------------------------------------------------
//   propertyList
//---------------------------------------------------------

static bool defaultAutoScale        = false;
static bool defaultLockAspectRatio  = true;
static bool defaultSizeIsSpatium    = true;

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::Image(Score* s)
   : BSymbol(s)
      {
      imageType        = IMAGE_NONE;
      _size            = QSizeF(0, 0);
      _storeItem       = 0;
      _dirty           = false;
      _lockAspectRatio = defaultLockAspectRatio;
      _autoScale       = defaultAutoScale;
      _sizeIsSpatium   = defaultSizeIsSpatium;
      setZ(IMAGE * 100);
      _linkIsValid     = false;
      }

Image::Image(const Image& img)
   : BSymbol(img)
      {
      imageType        = img.imageType;
      buffer           = img.buffer;
      _size            = img._size;
      _lockAspectRatio = img._lockAspectRatio;
      _autoScale       = img._autoScale;
      _dirty           = img._dirty;
      _storeItem       = img._storeItem;
      _sizeIsSpatium   = img._sizeIsSpatium;
      if (_storeItem)
            _storeItem->reference(this);
      _linkPath        = img._linkPath;
      _linkIsValid     = img._linkIsValid;
      if (imageType == IMAGE_RASTER)
            rasterDoc = img.rasterDoc;
      else if (imageType == IMAGE_SVG)
            svgDoc = img.svgDoc;
      }

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::~Image()
      {
      if (_storeItem)
            _storeItem->dereference(this);
      if (imageType == IMAGE_SVG)
            delete svgDoc;
      else if (imageType == IMAGE_RASTER)
            delete rasterDoc;
      }

//---------------------------------------------------------
//   setImageType
//---------------------------------------------------------

void Image::setImageType(ImageType t)
      {
      imageType = t;
      if (imageType == IMAGE_SVG)
            svgDoc = 0;
      else if (imageType == IMAGE_RASTER)
            rasterDoc = 0;
      else
            qDebug("illegal image type");
      }

//---------------------------------------------------------
//   imageSize
//---------------------------------------------------------

QSizeF Image::imageSize() const
      {
      if (imageType == IMAGE_RASTER)
            return rasterDoc->size();
      else
            return svgDoc->defaultSize();
      }

//---------------------------------------------------------
//   scaleFactor
//---------------------------------------------------------

qreal Image::scaleFactor() const
      {
      if (imageType == IMAGE_RASTER)
            return ( (_sizeIsSpatium ? spatium() : MScore::DPMM) / 0.4 );
      else
            return (_sizeIsSpatium ? 10.0 : MScore::DPMM);
      }

//---------------------------------------------------------
//   scale
//    return image scale in percent
//---------------------------------------------------------

QSizeF Image::scale() const
      {
      return scaleForSize(size());
      }

//---------------------------------------------------------
//   setScale
//---------------------------------------------------------

void Image::setScale(const QSizeF& scale)
      {
      setSize(sizeForScale(scale));
      }

//---------------------------------------------------------
//   scaleForSize
//---------------------------------------------------------

QSizeF Image::scaleForSize(const QSizeF& s) const
      {
      if(!isValid())
            return QSizeF();
      QSizeF sz = s * scaleFactor();
      return QSizeF(
               (sz.width()  * 100.0)/ imageSize().width(),
               (sz.height() * 100.0)/ imageSize().height()
               );
      }

//---------------------------------------------------------
//   sizeForScale
//---------------------------------------------------------

QSizeF Image::sizeForScale(const QSizeF& scale) const
      {
      QSizeF s = scale / 100.0;
//      qreal sz = _sizeIsSpatium ? spatium() : MScore::DPMM;
//      QSizeF oSize = imageSize() / sz;
      QSizeF oSize = imageSize() / scaleFactor();
      return QSizeF(s.width() * oSize.width(), s.height() * oSize.height());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Image::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_AUTOSCALE:
                  return autoScale();
            case P_SIZE:
                  return size();
            case P_SCALE:
                  return scale();
            case P_LOCK_ASPECT_RATIO:
                  return lockAspectRatio();
            case P_SIZE_IS_SPATIUM:
                  return sizeIsSpatium();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Image::setProperty(P_ID propertyId, const QVariant& v)
      {
      bool rv = true;
      score()->addRefresh(canvasBoundingRect());
      switch(propertyId) {
            case P_AUTOSCALE:
                  setAutoScale(v.toBool());
                  break;
            case P_SIZE:
                  setSize(v.toSizeF());
                  break;
            case P_SCALE:
                  setScale(v.toSizeF());
                  break;
            case P_LOCK_ASPECT_RATIO:
                  setLockAspectRatio(v.toBool());
                  break;
            case P_SIZE_IS_SPATIUM:
                  setSizeIsSpatium(v.toBool());
                  break;
            default:
                  rv = Element::setProperty(propertyId, v);
                  break;
            }
      setGenerated(false);
      score()->setLayoutAll(true);
      return rv;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Image::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_AUTOSCALE:             return defaultAutoScale;
            case P_SIZE:                  break;
            case P_LOCK_ASPECT_RATIO:     return defaultLockAspectRatio;
            case P_SIZE_IS_SPATIUM:       return defaultSizeIsSpatium;
            default:                      return Element::propertyDefault(id);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Image::draw(QPainter* painter) const
      {
      bool emptyImage = false;
      if (imageType == IMAGE_SVG) {
            if (!svgDoc)
                  emptyImage = true;
            else
                  svgDoc->render(painter, bbox());
            }
      else if (imageType == IMAGE_RASTER) {
            painter->save();
            QSizeF s;
            if (_sizeIsSpatium)
                  s = _size * spatium();
            else
                  s = _size * MScore::DPMM;
            if (score()->printing()) {
                  // use original image size for printing
                  painter->scale(s.width() / rasterDoc->width(), s.height() / rasterDoc->height());
                  painter->drawPixmap(QPointF(0, 0), QPixmap::fromImage(*rasterDoc));
                  }
            else {
                  QTransform t = painter->transform();
                  QSize ss = QSizeF(s.width() * t.m11(), s.height() * t.m22()).toSize();
                  t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
                  painter->setWorldTransform(t);
                  if ((buffer.size() != ss || _dirty) && rasterDoc && !rasterDoc->isNull()) {
                        buffer = QPixmap::fromImage(rasterDoc->scaled(ss, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                        _dirty = false;
                        }
                  if (buffer.isNull())
                        emptyImage = true;
                  else
                        painter->drawPixmap(QPointF(0.0, 0.0), buffer);
                  }
            painter->restore();
            }
      if (emptyImage) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::black);
            painter->drawRect(bbox());
            painter->drawLine(0.0, 0.0, bbox().width(), bbox().height());
            painter->drawLine(bbox().width(), 0.0, 0.0, bbox().height());
            }
      if (selected() && !(score() && score()->printing())) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(MScore::selectColor[0]);
            painter->drawRect(bbox());
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Image::write(Xml& xml) const
      {
      // attempt to convert the _linkPath to a path relative to the score
      //
      // TODO : on Save As, _score->fileInfo() still contains the old path and fname
      //          if the Save As path is different, image relative path will be wrong!
      //
      QString relativeFilePath= QString();
      if (!_linkPath.isEmpty() && _linkIsValid) {
            QFileInfo fi(_linkPath);
            // _score->fileInfo()->canonicalPath() would be better
            // but we are saving under a temp file name and the 'final' file
            // might not exist yet, so canonicalFilePath() may return only "/"
            // OTOH, the score 'final' file name is practically always canonical, at this point
            QString scorePath = _score->fileInfo()->absolutePath();
            QString imgFPath  = fi.canonicalFilePath();
            // if imgFPath is in (or below) the directory of scorePath
            if (imgFPath.startsWith(scorePath, Qt::CaseSensitive)) {
                  // relative img path is the part exceeding scorePath
                  imgFPath.remove(0, scorePath.size());
                  if(imgFPath.startsWith('/'))
                        imgFPath.remove(0, 1);
                  relativeFilePath = imgFPath;
                  }
            // try 1 level up
            else {
                  // reduce scorePath by one path level
                  fi.setFile(scorePath);
                  scorePath = fi.path();
                  // if imgFPath is in (or below) the directory up the score directory
                  if (imgFPath.startsWith(scorePath, Qt::CaseSensitive)) {
                        // relative img path is the part exceeding new scorePath plus "../"
                        imgFPath.remove(0, scorePath.size());
                        if (!imgFPath.startsWith('/'))
                              imgFPath.prepend('/');
                        imgFPath.prepend("..");
                        relativeFilePath = imgFPath;
                        }
                  }
            }
      // if no match, use full _linkPath
      if (relativeFilePath.isEmpty())
            relativeFilePath = _linkPath;

      xml.stag("Image");
      BSymbol::writeProperties(xml);
      // keep old "path" tag, for backward compatibility and because it is used elsewhere
      // (for instance by Box:read(), Measure:read(), Note:read(), ...)
      xml.tag("path", _storeItem ? _storeItem->hashName() : relativeFilePath);
      xml.tag("linkPath", relativeFilePath);

      writeProperty(xml, P_AUTOSCALE);
      writeProperty(xml, P_SIZE);
      writeProperty(xml, P_LOCK_ASPECT_RATIO);
      writeProperty(xml, P_SIZE_IS_SPATIUM);

      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Image::read(XmlReader& e)
      {
      if (score()->mscVersion() <= 123)
            _sizeIsSpatium = false;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "autoScale")
                  setProperty(P_AUTOSCALE, Ms::getProperty(P_AUTOSCALE, e));
            else if (tag == "size")
                  setProperty(P_SIZE, Ms::getProperty(P_SIZE, e));
            else if (tag == "lockAspectRatio")
                  setProperty(P_LOCK_ASPECT_RATIO, Ms::getProperty(P_LOCK_ASPECT_RATIO, e));
            else if (tag == "sizeIsSpatium")
                  setProperty(P_SIZE_IS_SPATIUM, Ms::getProperty(P_SIZE_IS_SPATIUM, e));
            else if (tag == "path")
                  _storePath = e.readElementText();
            else if (tag == "linkPath")
                  _linkPath = e.readElementText();
            else if (tag == "subtype")    // obsolete
                  e.skipCurrentElement();
            else if (!BSymbol::readProperties(e))
                  e.unknown();
            }

      // once all paths are read, load img or retrieve it from store
      // loading from file is tried first to update the stored image, if necessary

      qDebug("linkPath <%s>", qPrintable(_linkPath));
      qDebug("storePath <%s>", qPrintable(_storePath));

      QString path;
      bool    loaded = false;
      // if a store path is given, attempt to get the image from the store
      if (!_storePath.isEmpty()) {
            _storeItem = imageStore.getImage(_storePath);
            if (_storeItem) {
                  _storeItem->reference(this);
                  loaded = true;
                  }
            // if no image in store, attempt to load from path (for backward compatibility)
            else
                  loaded = load(_storePath);
            path = _storePath;
            }
      // if no succes from store path, attempt loading from link path (for .mscx files)
      if (!loaded) {
            _linkIsValid = load(_linkPath);
            path = _linkPath;
            }

      if (path.endsWith(".svg"))
            setImageType(IMAGE_SVG);
      else
            setImageType(IMAGE_RASTER);
      }

//---------------------------------------------------------
//   load
//    load image from file and put into ImageStore
//    return true on success
//---------------------------------------------------------

bool Image::load(const QString& ss)
      {
      qDebug("Image::load <%s>", qPrintable(ss));
      QString path(ss);
      // if file path is relative, prepend score path
      QFileInfo fi(path);
      if (fi.isRelative()) {
            path.prepend(_score->fileInfo()->absolutePath() + "/");
            fi.setFile(path);
            }

      _linkIsValid = false;                     // assume link fname is invalid
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("Image::load<%s> failed", qPrintable(path));
            return false;
            }
      QByteArray ba = f.readAll();
      f.close();

      _linkIsValid = true;
      _linkPath = fi.canonicalFilePath();
      _storeItem = imageStore.add(_linkPath, ba);
      _storeItem->reference(this);
      if (path.endsWith(".svg"))
            setImageType(IMAGE_SVG);
      else
            setImageType(IMAGE_RASTER);
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Image::editDrag(const EditData& ed)
      {
      qreal ratio = _size.width() / _size.height();
      qreal dx = ed.delta.x();
      qreal dy = ed.delta.y();
      if (_sizeIsSpatium) {
            qreal _spatium = spatium();
            dx /= _spatium;
            dy /= _spatium;
            }
      else {
            dx /= MScore::DPMM;
            dy /= MScore::DPMM;
            }
      if (ed.curGrip == 0) {
            _size.setWidth(_size.width() + dx);
            if (_lockAspectRatio)
                  _size.setHeight(_size.width() / ratio);
            }
      else {
            _size.setHeight(_size.height() + dy);
            if (_lockAspectRatio)
                  _size.setWidth(_size.height() * ratio);
            }
      layout();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Image::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 2;
      QRectF r(pageBoundingRect());
      grip[0].translate(QPointF(r.x() + r.width(), r.y() + r.height() * .5));
      grip[1].translate(QPointF(r.x() + r.width() * .5, r.y() + r.height()));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Image::layout()
      {
      if (imageType == IMAGE_SVG && !svgDoc) {
            if (_storeItem) {
                  svgDoc = new QSvgRenderer(_storeItem->buffer());
                  if (svgDoc->isValid()) {
                        if (_size.isNull()) {
                              _size = svgDoc->defaultSize();
                              if (_sizeIsSpatium)
                                    _size /= 10.0;    // by convention
                              }
                        }
                  }
            }
      else if (imageType == IMAGE_RASTER && !rasterDoc) {
            if (_storeItem) {
                  rasterDoc = new QImage;
                  rasterDoc->loadFromData(_storeItem->buffer());
                  if (!rasterDoc->isNull()) {
                        if (_size.isNull()) {
                              _size = rasterDoc->size() * 0.4;
                              if (_sizeIsSpatium)
                                    _size /= spatium();
                              else
                                    _size /= MScore::DPMM;
                              }
                        _dirty = true;
                        }
                  }
            }

      qreal f = _sizeIsSpatium ? spatium() : MScore::DPMM;
      // if autoscale && inside a box, scale to box relevant size
      if (autoScale() && parent() && ((parent()->type() == HBOX || parent()->type() == VBOX))) {
            if (_lockAspectRatio) {
                  QSizeF size(imageSize());
                  qreal ratio = size.width() / size.height();
                  qreal w = parent()->width();
                  qreal h = parent()->height();
                  if ((w / h) < ratio) {
                        _size.setWidth(w / f);
                        _size.setHeight((w / ratio) / f);
                        }
                  else {
                        _size.setHeight(h / f);
                        _size.setWidth(h * ratio / f);
                        }
                  }
            else
                  _size = parent()->bbox().size() / f;
            }

      // in any case, adjust position relative to parent
      adjustReadPos();
      bbox().setRect(0.0, 0.0, _size.width() * f, _size.height() * f);
      }

}

