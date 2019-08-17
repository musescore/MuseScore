//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "pianoruler.h"
#include "pianokeyboard.h"
#include "libmscore/score.h"

namespace Ms {

#if 0 // yet(?) unused
static const int MAP_OFFSET = 480;
#endif

QPixmap* PianoRuler::markIcon[3];

static const char* rmark_xpm[]={
      "18 18 2 1",
      "# c #0000ff",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "........##########",
      "........#########.",
      "........########..",
      "........#######...",
      "........######....",
      "........#####.....",
      "........####......",
      "........###.......",
      "........##........",
      "........##........",
      "........##........"};
static const char* lmark_xpm[]={
      "18 18 2 1",
      "# c #0000ff",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "##########........",
      ".#########........",
      "..########........",
      "...#######........",
      "....######........",
      ".....#####........",
      "......####........",
      ".......###........",
      "........##........",
      "........##........",
      "........##........"};
static const char* cmark_xpm[]={
      "18 18 2 1",
      "# c #ff0000",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "##################",
      ".################.",
      "..##############..",
      "...############...",
      "....##########....",
      ".....########.....",
      "......######......",
      ".......####.......",
      "........##........",
      "........##........",
      "........##........"};


//---------------------------------------------------------
//   Ruler
//---------------------------------------------------------

PianoRuler::PianoRuler(QWidget* parent)
   : QWidget(parent)
      {
      if (markIcon[0] == 0) {
            markIcon[0] = new QPixmap(cmark_xpm);
            markIcon[1] = new QPixmap(lmark_xpm);
            markIcon[2] = new QPixmap(rmark_xpm);
            }
      setMouseTracking(true);
      _xpos   = 0;
      _xZoom = X_ZOOM_INITIAL;
      _timeType = TType::TICKS;
      _font2.setPixelSize(14);
      _font2.setBold(true);
      _font1.setPixelSize(10);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void PianoRuler::setScore(Score* s, Pos* lc)
      {
      _score = s;
      _locator = lc;
      if (_score)
            _cursor.setContext(_score->tempomap(), _score->sigmap());
      setEnabled(_score != 0);
      }

//---------------------------------------------------------
//   setXpos
//---------------------------------------------------------

void PianoRuler::setXpos(int val)
      {
      _xpos = val;
      update();
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

Pos PianoRuler::pix2pos(int x) const
      {
      int val = (x + _xpos) / _xZoom - MAP_OFFSET;
      
      if (val < 0)
            val = 0;
      return Pos(_score->tempomap(), _score->sigmap(), val, _timeType);
      
      }

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int PianoRuler::pos2pix(const Pos& p) const
      {
      return (p.time(TType::TICKS) + MAP_OFFSET) * _xZoom - _xpos;
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void PianoRuler::paintEvent(QPaintEvent* e)
      {
      QPainter p(this);
      const QRect& r = e->rect();

      int x  = r.x();
      int w  = r.width();
      int y  = pianoRulerHeight - 16;
      int h  = 16; // 14;
      int y1 = r.y();
      int rh = r.height();
      if (y1 < pianoRulerHeight) {
            rh -= pianoRulerHeight - y1;
            y1 = pianoRulerHeight;
            }
      int y2 = y1 + rh;

      if (x < (-_xpos))
            x = -_xpos;

      if (!_score)
            return;

      Pos pos1 = pix2pos(x);
      Pos pos2 = pix2pos(x+w);

      //---------------------------------------------------
      //    draw lines
      //---------------------------------------------------

      int bar1, bar2, beat, tick;

      pos1.mbt(&bar1, &beat, &tick);
      pos2.mbt(&bar2, &beat, &tick);

      const int minBarGapSize = 48;
      const int minBeatGapSize = 30;
      
      //Estimate bar width since changing time signatures can make this inconsistent.
      // Assuming 480 ticks per beat, 4 beats per bar
      qreal pixPerBar = MScore::division * 4 * _xZoom;
      qreal pixPerBeat = MScore::division * _xZoom;
      
      int barSkip = ceil(minBarGapSize / pixPerBar);
      barSkip = (int)pow(2, ceil(log(barSkip)/log(2)));

      int beatSkip = ceil(minBeatGapSize / pixPerBeat);
      beatSkip = (int)pow(2, ceil(log(beatSkip)/log(2)));
      
      //Round down to first bar to be a multiple of barSkip
      bar1 = (bar1 / barSkip) * barSkip;
      
      for (int bar = bar1; bar <= bar2; bar += barSkip) {
            Pos stick(_score->tempomap(), _score->sigmap(), bar, 0, 0);
            
            SigEvent sig = stick.timesig();
            int z = sig.timesig().numerator();
            for (int beat1 = 0; beat1 < z; beat1 += beatSkip) {
                  Pos xx(_score->tempomap(), _score->sigmap(), bar, beat1, 0);
                  int xp = pos2pix(xx);
                  if (xp < 0)
                        continue;
                  QString s;
                  QRect r1(xp+2, y + 1, 1000, h);
                  int y3;
                  int num;
                  if (beat1 == 0) {
                        num = bar + 1;
                        y3  = y + 2;
                        p.setFont(_font2);
                        }
                  else {
                        num = beat1 + 1;
                        y3  = y + 8;
                        p.setFont(_font1);
                        r1.moveTop(r1.top() + 1);
                        }
                  s.setNum(num);
                  p.setPen(Qt::black);
                  p.drawLine(xp, y3, xp, y+h);
                  p.drawText(r1, Qt::AlignLeft | Qt::AlignVCenter, s);
                  p.setPen(beat1 == 0 ? Qt::lightGray : Qt::gray);
                  if (xp > 0)
                        p.drawLine(xp, y1, xp, y2);
                  }
            
            }
      //
      //  draw mouse cursor marker
      //
      p.setPen(Qt::black);
      if (_cursor.valid()) {
            int xp = pos2pix(_cursor);
            if (xp >= x && xp < x+w)
                  p.drawLine(xp, 0, xp, pianoRulerHeight);
            }
      static const QColor lcColors[3] = { Qt::red, Qt::blue, Qt::blue };
      for (int i = 0; i < 3; ++i) {
            if (!_locator[i].valid())
                  continue;
            p.setPen(lcColors[i]);
            int xp      = pos2pix(_locator[i]);
            QPixmap* pm = markIcon[i];
            int pw = pm->width() / 2;
            int x1 = x - pw;
            int x2 = x + w + pw;
            if (xp >= x1 && xp < x2)
                  p.drawPixmap(xp - pw, y-2, *pm);
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void PianoRuler::mousePressEvent(QMouseEvent* e)
      {
      moveLocator(e);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void PianoRuler::mouseMoveEvent(QMouseEvent* e)
      {
      moveLocator(e);
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianoRuler::moveLocator(QMouseEvent* e)
      {
      Pos pos(pix2pos(e->pos().x()));
      if (e->buttons() & Qt::LeftButton)
            emit locatorMoved(0, pos);
      else if (e->buttons() & Qt::MidButton)
            emit locatorMoved(1, pos);
      else if (e->buttons() & Qt::RightButton)
            emit locatorMoved(2, pos);
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void PianoRuler::leaveEvent(QEvent*)
      {
      _cursor.setInvalid();
      emit posChanged(_cursor);
      update();
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void PianoRuler::setPos(const Pos& pos)
      {
      if (_cursor != pos) {
            int x1 = pos2pix(_cursor);
            int x2 = pos2pix(pos);
            if (x1 > x2) {
                  int tmp = x2;
                  x2 = x1;
                  x1 = tmp;
                  }
            update(QRect(x1-1, 0, x2-x1+2, height()));
            _cursor = pos;
            }
      }

//---------------------------------------------------------
//   setXZoom
//---------------------------------------------------------

void PianoRuler::setXZoom(qreal xZoom)
      {
      _xZoom = xZoom;
      update();
      }

}

