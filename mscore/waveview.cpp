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

#include "waveview.h"
#include "piano.h"
#include "libmscore/audio.h"
#include "libmscore/score.h"

#include <vorbis/vorbisfile.h>

namespace Ms {


//---------------------------------------------------------
//   VorbisData
//---------------------------------------------------------

struct VorbisData {
      int pos;          // current position in audio->data()
      QByteArray data;
      };

static VorbisData vorbisData;

static size_t ovRead(void* ptr, size_t size, size_t nmemb, void* datasource);
static int ovSeek(void* datasource, ogg_int64_t offset, int whence);
static long ovTell(void* datasource);

static ov_callbacks ovCallbacks = {
      ovRead, ovSeek, 0, ovTell
      };

//---------------------------------------------------------
//   ovRead
//---------------------------------------------------------

static size_t ovRead(void* ptr, size_t size, size_t nmemb, void* datasource)
      {
      VorbisData* vd = (VorbisData*)datasource;
      size_t n = size * nmemb;
      if (vd->data.size() < int(vd->pos + n))
            n = vd->data.size() - vd->pos;
      if (n) {
            const char* src = vd->data.data() + vd->pos;
            memcpy(ptr, src, n);
            vd->pos += int(n);
            }
      return n;
      }

//---------------------------------------------------------
//   ovSeek
//---------------------------------------------------------

static int ovSeek(void* datasource, ogg_int64_t offset, int whence)
      {
      VorbisData* vd = (VorbisData*)datasource;
      switch(whence) {
            case SEEK_SET:
                  vd->pos = offset;
                  break;
            case SEEK_CUR:
                  vd->pos += offset;
                  break;
            case SEEK_END:
                  vd->pos = vd->data.size() - offset;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   ovTell
//---------------------------------------------------------

static long ovTell(void* datasource)
      {
      VorbisData* vd = (VorbisData*)datasource;
      return vd->pos;
      }


//---------------------------------------------------------
//   WaveView
//---------------------------------------------------------

WaveView::WaveView(QWidget* parent)
   : QWidget(parent)
      {
      _xpos   = 0;
      _xmag   = 0.1;
      _timeType = TType::TICKS;      // TType::FRAMES
      setMinimumHeight(50);
      }

static const int WSCALE = 10;

//---------------------------------------------------------
//   setAudio
//---------------------------------------------------------

void WaveView::setAudio(Audio* audio)
      {
      waves.clear();
      OggVorbis_File vf;
      vorbisData.pos  = 0;
      vorbisData.data = audio->data();
      int rv = ov_open_callbacks(&vorbisData, &vf, 0, 0, ovCallbacks);
      if (rv < 0) {
            qDebug("ogg open failed: %d", rv);
            return;
            }
      int rn = 0;
      const int n = 10000 / WSCALE;
      uchar dst[n];
      float* r = 0;
      float* l = 0;
      for (;;) {
            for (int i = 0; i < n; ++i) {
                  float val = 0;
                  for (int k = 0; k < WSCALE; ++k) {
                        if (rn == 0) {
                              float** pcm;
                              int section;
                              rn = ov_read_float(&vf, &pcm, 1000, &section);
                              if (rn == 0) {
                                    for (; i < n; ++i)
                                          dst[i] = 0;
                                    waves.append((char*)dst, n);
                                    ov_clear(&vf);
                                    return;
                                    }
                              r = pcm[0];
                              l = pcm[1];
                              }
                        val += fabsf(*r++) + fabsf(*l++);
                        --rn;
                        }
                  unsigned int v = lrint(val * 128);
                  if (v > 255)
                        v = 255;
                  dst[i] = v;
                  }
            waves.append((char*)dst, n);
            }
      }

//---------------------------------------------------------
//   pegel
//---------------------------------------------------------

int WaveView::pegel(int frame1, int frame2)
      {
      if (frame1 < 0)
            frame1 = 0;
      if (frame1 > frame2)
            return 0;
      frame1 = (frame1 + WSCALE/2) / WSCALE;
      frame2 = (frame2 + WSCALE/2) / WSCALE;
      if (frame2 >= waves.size())
            frame2 = waves.size()-1;
      int p = 0;
      for (int i = frame1; i <= frame2; ++i)
            p += uchar(waves[i]);
      p /= (frame2 - frame1 + 1);
      return p;
      }

//---------------------------------------------------------
//   setXpos
//---------------------------------------------------------

void WaveView::setXpos(int val)
      {
      _xpos = val;
      update();
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void WaveView::setMag(double x, double)
      {
      if (_xmag != x) {
            _xmag = x;
            update();
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void WaveView::moveLocator(int i)
      {
      if (_locator[i].valid()) {
            update();
            // qreal x = qreal(pos2pix(_locator[i]));
            // locatorLines[i]->setPos(QPointF(x, 0.0));
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void WaveView::setScore(Score* s, Pos* lc)
      {
      _score = s;
      _locator = lc;
      _cursor.setContext(s->tempomap(), s->sigmap());
      }

static const int MAP_OFFSET = 5;

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int WaveView::pos2pix(const Pos& p) const
      {
      return lrint((p.time(_timeType) + 480) * _xmag)
         + MAP_OFFSET - _xpos + pianoWidth;
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

Pos WaveView::pix2pos(int x) const
      {
      x -= (pianoWidth + MAP_OFFSET);
      if (x < 0)
            x = 0;
      return Pos(_score->tempomap(), _score->sigmap(), ((x + _xpos) / _xmag) - 480, _timeType);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void WaveView::paintEvent(QPaintEvent* event)
      {
      QPainter p(this);
      int xoffset = pianoWidth + MAP_OFFSET;
      QRect rt(0, 0, xoffset, height());
      QRect r(event->rect());

      p.fillRect(r, Qt::gray);
      if (rt.intersects(r.translated(_xpos, 0)))
            p.fillRect(rt.translated(-_xpos, 0), Qt::lightGray);

      int x1 = r.x();
      int x2 = x1 + r.width();
      if (x1 < pianoWidth)
            x1 = pianoWidth;
      Pos p1 = pix2pos(x1);
      p.setPen(QPen(Qt::blue, 1));
      for (int i = x1+1; i < x2; ++i) {
            Pos p2 = pix2pos(i);
            int val = height() * pegel(p1.frame(), p2.frame()) / 255;
            p1 = p2;
            int y1 = height() - val;
            int y2 = height();
            p.drawLine(i, y1, i, y2);
            }

      p.setPen(QPen(Qt::lightGray, 2));
      int y = height() / 2;
      int x = xoffset - _xpos;
      int w = width() - x;
      if (w > 0)
            p.drawLine(0, y, width(), y);

      static const QColor lcColors[3] = { Qt::red, Qt::blue, Qt::blue };
      for (int i = 0; i < 3; ++i) {
            if (!_locator[i].valid())
                  continue;
            QPen pen(lcColors[i], 3);
            p.setPen(pen);
            int xp      = pos2pix(_locator[i]);
            p.drawLine(xp, 0, xp, height());
            }
      }
}

