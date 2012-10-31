//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: image.cpp 5568 2012-04-22 10:08:43Z wschweer $
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
      buffer           = img.buffer;
      _size            = img._size;
      _lockAspectRatio = img._lockAspectRatio;
      _autoScale       = img._autoScale;
      _dirty           = img._dirty;
      _storeItem       = img._storeItem;
      _sizeIsSpatium   = img._sizeIsSpatium;
      if(_storeItem)
            _storeItem->reference(this);
      _linkPath        = img._linkPath;
      _linkIsValid     = img._linkIsValid;
      }

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::~Image()
      {
      if (_storeItem)
            _storeItem->dereference(this);
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
//      QSizeF sz = s * (_sizeIsSpatium ? spatium() : MScore::DPMM);
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

void Image::draw(QPainter* painter, QSize size) const
      {
      if (buffer.isNull()) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::black);
            QPointF p[6];
            qreal w = size.width();
            qreal h = size.height();
            p[0] = QPointF(0.0, 0.0);
            p[1] = QPointF(w,   0.0);
            p[2] = QPointF(w,   h);
            p[3] = QPointF(0.0, 0.0);
            p[4] = QPointF(0.0, h);
            p[5] = QPointF(w,  0.0);
            painter->drawPolyline(p, 6);
            p[0] = QPointF(0.0, h);
            p[1] = QPointF(w, h);
            painter->drawPolyline(p, 2);
            }
      else
            painter->drawPixmap(QPointF(0.0, 0.0), buffer);
      if (selected() && !(score() && score()->printing())) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::blue);

            QPointF p[5];
            qreal w = size.width();
            qreal h = size.height();

            p[0] = QPointF(0.0, 0.0);
            p[1] = QPointF(w,   0.0);
            p[2] = QPointF(w,   h);
            p[3] = QPointF(0.0, h);
            p[4] = QPointF(0.0, 0.0);
            painter->drawPolyline(p, 5);
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
      if(!_linkPath.isEmpty() && _linkIsValid) {
            QFileInfo fi(_linkPath);
            // _score->fileInfo()->canonicalPath() would be better
            // but we are saving under a temp file name and the 'final' file
            // might not exist yet, so canonicalFilePath() may return only "/"
            // OTOH, the score 'final' file name is practically always canonical, at this point
            QString scorePath = _score->fileInfo()->absolutePath();
            QString imgFPath  = fi.canonicalFilePath();
            // if imgFPath is in (or below) the directory of scorePath
            if(imgFPath.startsWith(scorePath, Qt::CaseSensitive)) {
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
                  if(imgFPath.startsWith(scorePath, Qt::CaseSensitive)) {
                        // relative img path is the part exceeding new scorePath plus "../"
                        imgFPath.remove(0, scorePath.size());
                        if(!imgFPath.startsWith('/'))
                              imgFPath.prepend('/');
                        imgFPath.prepend("..");
                        relativeFilePath = imgFPath;
                        }
                  }
            }
      // if no match, use full _linkPath
      if(relativeFilePath.isEmpty())
            relativeFilePath = _linkPath;

      xml.stag("Image");
      Element::writeProperties(xml);
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

void Image::read(const QDomElement& de)
      {
      if (score()->mscVersion() <= 123)
            _sizeIsSpatium = false;

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "autoScale")
                  setProperty(P_AUTOSCALE, ::getProperty(P_AUTOSCALE, e));
            else if (tag == "size")
                  setProperty(P_SIZE, ::getProperty(P_SIZE, e));
            else if (tag == "lockAspectRatio")
                  setProperty(P_LOCK_ASPECT_RATIO, ::getProperty(P_LOCK_ASPECT_RATIO, e));
            else if (tag == "sizeIsSpatium")
                  setProperty(P_SIZE_IS_SPATIUM, ::getProperty(P_SIZE_IS_SPATIUM, e));
            else if (tag == "path")
                  _storePath = e.text();
            else if(tag == "linkPath")
                  _linkPath = e.text();
            else if (!Element::readProperties(e))
                  domError(e);
            }

      // once all paths are read, load img or retrieve it from store
      // loading from file is tried first to update the stored image, if necessary
      if(_linkPath.isEmpty() || !load(_linkPath)) {
            // if could not load img from _linkPath, retrieve from store
            _storeItem = imageStore.getImage(_storePath);
            if (_storeItem)
                  _storeItem->reference(this);
            // if not in store, try to load from _storePath for backward compatibility
            else
                  load(_storePath);
            }
      }

//---------------------------------------------------------
//   load
//    load image from file and put into ImageStore
//    return true on success
//---------------------------------------------------------

bool Image::load(const QString& ss)
      {
      QString path(ss);
      // if file path is relative, prepend score path
      QFileInfo fi(path);
      if(fi.isRelative()) {
            path.prepend(_score->fileInfo()->absolutePath() + "/");
            fi.setFile(path);
      }

      _linkIsValid = false;                     // assume link fname is invalid
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly))
            return false;
      QByteArray ba = f.readAll();
      f.close();

      _linkIsValid = true;
      _linkPath = fi.canonicalFilePath();
      _storeItem = imageStore.add(_linkPath, ba);
      _storeItem->reference(this);
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
//   SvgImage
//---------------------------------------------------------

SvgImage::SvgImage(Score* s)
   : Image(s)
      {
      doc = 0;
      }

//---------------------------------------------------------
//   SvgImage
//---------------------------------------------------------

SvgImage::~SvgImage()
      {
      delete doc;
      }

SvgImage* SvgImage::clone() const
      {
      return new SvgImage(*this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SvgImage::draw(QPainter* painter) const
      {
      if (!doc) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::black);
            painter->drawRect(bbox());
            painter->drawLine(0.0, 0.0, bbox().width(), bbox().height());
            painter->drawLine(bbox().width(), 0.0, 0.0, bbox().height());
            }
      else
            doc->render(painter, bbox());
      if (selected() && !(score() && score()->printing())) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::blue);
            painter->drawRect(bbox());
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SvgImage::layout()
      {
      if (!doc) {
            if (_storeItem) {
                  doc = new QSvgRenderer(_storeItem->buffer());
                  if (doc->isValid()) {
                        if (_size.isNull()) {
                              _size = doc->defaultSize();
                              if (_sizeIsSpatium)
                                    _size /= 10.0;    // by convention
                              }
                        }
                  }
            }
      Image::layout();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RasterImage::draw(QPainter* painter) const
      {
      painter->save();
      QSizeF s;
      if (_sizeIsSpatium)
            s = _size * spatium();
      else
            s = _size * MScore::DPMM;
      if (score()->printing()) {
            // use original image size for printing
            painter->scale(s.width() / doc.width(), s.height() / doc.height());
            painter->drawPixmap(QPointF(0, 0), QPixmap::fromImage(doc));
            }
      else {
            QTransform t = painter->transform();
            QSize ss = QSizeF(s.width() * t.m11(), s.height() * t.m22()).toSize();
            t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
            painter->setWorldTransform(t);
            if ((buffer.size() != ss || _dirty) && !doc.isNull()) {
                  buffer = QPixmap::fromImage(doc.scaled(ss, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                  _dirty = false;
                  }
            Image::draw(painter, ss);
            }
      painter->restore();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RasterImage::layout()
      {
      if (doc.isNull()) {
            if (_storeItem) {
                  doc.loadFromData(_storeItem->buffer());
                  if (!doc.isNull()) {
                        if (_size.isNull()) {
                              _size = doc.size() * 0.4;
                              if (_sizeIsSpatium)
                                    _size /= spatium();
                              else
                                    _size /= MScore::DPMM;
                              }
                        _dirty = true;
                        }
                  }
            }
      Image::layout();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Image::layout()
      {
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
      setbbox(QRectF(0.0, 0.0, _size.width() * f, _size.height() * f));
      }

