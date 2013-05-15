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

//---------------------------------------------------------
//   pmKey
//---------------------------------------------------------

static inline int pmKey(const QSize &size, QIcon::Mode mode, QIcon::State state)
      {
      return ((((((size.width()<<11)|size.height())<<11)|mode)<<4)|state);
      }

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
            if (!svgFile.isEmpty())
                  renderer->load(svgFile);
            }
      }

//---------------------------------------------------------
//   pixmap
//---------------------------------------------------------

QPixmap MIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
      {
      QPixmap pm;

      QString pmckey(d->pmcKey(size, mode, state));
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

      QImage img(actualSize, QImage::Format_ARGB32_Premultiplied);
      img.fill(0x00000000);
      QPainter p(&img);
      renderer.render(&p);
      p.end();

      if (mode == QIcon::Disabled) {
            int ww = img.width();
            for (int y = 0; y < img.height(); ++y) {
                  quint32* p = (quint32*)img.scanLine(y);
                  for (int x = 0; x < ww; ++x) {
                        if (*p & 0xff000000) {
                              QColor color(QColor::fromRgba(*p));
                              int alpha = color.alpha() - 64;
                              if (alpha < 0)
                                    alpha = 0;
                              color.setAlpha(alpha);
                              *p = color.rgba();
                              }
                        ++p;
                        }
                  }
            }
      else if (state == QIcon::On) {
            int ww = img.width();
            for (int y = 0; y < img.height(); ++y) {
                  quint32* p = (quint32*)img.scanLine(y);
                  for (int x = 0; x < ww; ++x) {
                        if (*p & 0xff000000) {
                              int d = 0xff - (*p & 0xff);
                              int dd = 50;
                              QColor color(QColor::fromRgba(*p));
                              int r = 70 - d + dd;
                              if (r < 0)
                                    r = 0;
                              int g = 130 - d + dd;
                              if (g < 0)
                                    g = 0;
                              int b = 180 - d + dd;
                              if (b < 0)
                                    b = 0;
                              QColor nc = QColor(r, g, b, color.alpha());
                              *p = nc.rgba();
                              }
                        ++p;
                        }
                  }
            }

      pm = QPixmap::fromImage(img);
      if (qobject_cast<QApplication *>(QCoreApplication::instance())) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();  //  opt.palette = QGuiApplication::palette();
            QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
            if (!generated.isNull())
                  pm = generated;
            }

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
      painter->drawPixmap(rect, pixmap(rect.size(), mode, state));
      }

//---------------------------------------------------------
//   key
//---------------------------------------------------------

QString MIconEngine::key() const
      {
      return QLatin1String("svg");
      }

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

QIconEngine *MIconEngine::clone() const
      {
      return new MIconEngine(*this);
      }

