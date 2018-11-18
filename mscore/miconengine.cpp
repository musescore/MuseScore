//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

// this is a modified version of qt QSvgIconEngine

#include "miconengine.h"
#include "preferences.h"

//---------------------------------------------------------
//   MIconEnginePrivate
//---------------------------------------------------------

class MIconEnginePrivate : public QSharedData
      {
   public:
      MIconEnginePrivate() : svgBuffers(0), addedPixmaps(0) { stepSerialNum(); }
      ~MIconEnginePrivate() { delete addedPixmaps; delete svgBuffers; }
      static int hashKey(QIcon::Mode mode, QIcon::State state) { return (((mode)<<4)|state); }

      QString pmcKey(const QSize &size, QIcon::Mode mode, QIcon::State state) {
            return QLatin1String("$qt_svgicon_")
                 + QString::number(serialNum, 16).append(QLatin1Char('_'))
                 + QString::number((((((size.width()<<11)|size.height())<<11)|mode)<<4)|state, 16);
            }

      void stepSerialNum() { serialNum = lastSerialNum.fetchAndAddRelaxed(1); }
      void loadDataForModeAndState(QSvgRenderer *renderer, QIcon::Mode mode, QIcon::State state);

      QHash<int, QString> svgFiles;
      QHash<int, QByteArray> *svgBuffers;
      QHash<int, QPixmap> *addedPixmaps;
      int serialNum;
      static QAtomicInt lastSerialNum;
      };

QAtomicInt MIconEnginePrivate::lastSerialNum;

#if 0 // yet(?) unused
//---------------------------------------------------------
//   pmKey
//---------------------------------------------------------

static inline int pmKey(const QSize &size, QIcon::Mode mode, QIcon::State state)
      {
      return ((((((size.width()<<11)|size.height())<<11)|mode)<<4)|state);
      }
#endif

//---------------------------------------------------------
//   MIconEngine
//---------------------------------------------------------

MIconEngine::MIconEngine()
    : d(new MIconEnginePrivate)
      {
      }

MIconEngine::MIconEngine(const MIconEngine &other)
    : QIconEngine(other), d(new MIconEnginePrivate)
      {
      d->svgFiles = other.d->svgFiles;
      if (other.d->svgBuffers)
            d->svgBuffers = new QHash<int, QByteArray>(*other.d->svgBuffers);
      if (other.d->addedPixmaps)
            d->addedPixmaps = new QHash<int, QPixmap>(*other.d->addedPixmaps);
      }

MIconEngine::~MIconEngine()
      {
      }

//---------------------------------------------------------
//   actualSize
//---------------------------------------------------------

QSize MIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
      {
      if (d->addedPixmaps) {
            QPixmap pm = d->addedPixmaps->value(d->hashKey(mode, state));
            if (!pm.isNull() && pm.size() == size)
                  return size;
            }

      QPixmap pm = pixmap(size, mode, state);
      if (pm.isNull())
            return QSize();
      return pm.size();
      }

//---------------------------------------------------------
//   loadDataForModeAndState
//---------------------------------------------------------

void MIconEnginePrivate::loadDataForModeAndState(QSvgRenderer* renderer, QIcon::Mode mode, QIcon::State state)
      {
      QByteArray buf;
      if (svgBuffers) {
            buf = svgBuffers->value(hashKey(mode, state));
            if (buf.isEmpty())
                  buf = svgBuffers->value(hashKey(QIcon::Normal, QIcon::Off));
            }
      if (!buf.isEmpty()) {
            buf = qUncompress(buf);
            renderer->load(buf);
            }
      else {
            QString svgFile = svgFiles.value(hashKey(mode, state));
            if (svgFile.isEmpty())
                  svgFile = svgFiles.value(hashKey(QIcon::Normal, QIcon::Off));
            if (!svgFile.isEmpty()) {
                  QFile f(svgFile);
                  f.open(QIODevice::ReadOnly);
                  QByteArray ba = f.readAll();
                  if (mode == QIcon::Disabled) {
                        if (Ms::preferences.isThemeDark()) {
                              if (state == QIcon::On)
                                    ba.replace("#3b3f45", "#4171a2").replace("#3B3F45", "#4171a2").replace("rgb(59,63,69)", "#4171a2");
                              else
                                    ba.replace("#3b3f45", "#a0a0a0").replace("#3B3F45", "#a0a0a0").replace("rgb(59,63,69)", "#a0a0a0");
                              }
                        else {
                              if (state == QIcon::On)
                                    ba.replace("#3b3f45", "#8daac7").replace("#3B3F45", "#8daac7").replace("rgb(59,63,69)", "#8daac7");
                              else
                                    ba.replace("#3b3f45", "#a0a0a0").replace("#3B3F45", "#a0a0a0").replace("rgb(59,63,69)", "#a0a0a0");
                              }
                        }
                  else {
                        if (Ms::preferences.isThemeDark()) {
                              if (state == QIcon::On)
                                    ba.replace("#3b3f45", "#78afe6").replace("#3B3F45", "#78afe6").replace("rgb(59,63,69)", "#78afe6");
			            else
                                    ba.replace("#3b3f45", "#eff0f1").replace("#3B3F45", "#eff0f1").replace("rgb(59,63,69)", "#eff0f1");
                              }
                        else {
                              if (state == QIcon::On)
                                    ba.replace("#3b3f45", "#4171a2").replace("#3B3F45", "#4171a2").replace("rgb(59,63,69)", "#4171a2");
                              }
                        }
                  renderer->load(ba);
                  }
            }
      }

static const QRectF getBounds(const QSize outerSize, const QSize innerSize)
    {
    // Horizontal Offset
    qreal hOffset = 0.0;
    if (innerSize.width() < outerSize.width())
        hOffset = (outerSize.width() - innerSize.width()) * 0.5;

    // Vertical Offset
    qreal vOffset = 0.0;
    if (innerSize.height() < outerSize.height())
        vOffset = (outerSize.height() - innerSize.height()) * 0.5;

    return QRectF(hOffset, vOffset, innerSize.width(), innerSize.height());
    }

//---------------------------------------------------------
//   pixmap
//---------------------------------------------------------

QPixmap MIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
      {
      QPixmap pm;

      QString pmckey(d->pmcKey(size, mode, state));
      pmckey.prepend("Ms");

      if (QPixmapCache::find(pmckey, pm))
            return pm;

      if (d->addedPixmaps) {
            pm = d->addedPixmaps->value(d->hashKey(mode, state));
            if (!pm.isNull() && pm.size() == size)
                  return pm;
            }

      QSvgRenderer renderer;
      d->loadDataForModeAndState(&renderer, mode, state);
      if (!renderer.isValid())
            return pm;

      QSize actualSize = renderer.defaultSize();
      if (!actualSize.isNull())
            actualSize.scale(size, Qt::KeepAspectRatio);

      // Generate an image of the requested size, but render the
      // the SVG with the correct aspect ratio centered in the image
      // to prevent scaling issues when setting non square icon size.
      QImage img(size, QImage::Format_ARGB32);
      img.fill(0x00000000);
      QPainter p(&img);
      renderer.render(&p, getBounds(size, actualSize));
      p.end();
      pm = QPixmap::fromImage(img);

      if (!pm.isNull())
            QPixmapCache::insert(pmckey, pm);
      return pm;
      }

//---------------------------------------------------------
//   addPixmap
//---------------------------------------------------------

void MIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
      {
      if (!d->addedPixmaps)
            d->addedPixmaps = new QHash<int, QPixmap>;
      d->stepSerialNum();
      d->addedPixmaps->insert(d->hashKey(mode, state), pixmap);
      }

//---------------------------------------------------------
//   addFile
//---------------------------------------------------------

void MIconEngine::addFile(const QString &fileName, const QSize &, QIcon::Mode mode, QIcon::State state)
      {
      if (!fileName.isEmpty()) {
            QString abs = fileName;
            if (fileName.at(0) != QLatin1Char(':'))
                  abs = QFileInfo(fileName).absoluteFilePath();
            if (abs.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)
                || abs.endsWith(QLatin1String(".svgz"), Qt::CaseInsensitive)
                || abs.endsWith(QLatin1String(".svg.gz"), Qt::CaseInsensitive))
                  {
                  QSvgRenderer renderer(abs);
                  if (renderer.isValid()) {
                        d->stepSerialNum();
                        d->svgFiles.insert(d->hashKey(mode, state), abs);
                        }
                  }
            else {
                  QPixmap pm(abs);
                  if (!pm.isNull())
                        addPixmap(pm, mode, state);
                  }
            }
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void MIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
      {
      QSize pixmapSize = rect.size();
      if (painter->device())
          pixmapSize *= painter->device()->devicePixelRatio();
      painter->drawPixmap(rect, pixmap(pixmapSize, mode, state));
      }

//---------------------------------------------------------
//   key
//---------------------------------------------------------

QString MIconEngine::key() const
      {
      return QLatin1String("micon-svg");
      }

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

QIconEngine *MIconEngine::clone() const
      {
      return new MIconEngine(*this);
      }
