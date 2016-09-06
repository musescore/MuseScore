//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2011-2016 Werner Schweer and others
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

#include "pianotools.h"
#include "preferences.h"

namespace Ms {

static const qreal KEY_WIDTH   = 13.0;
static const qreal BKEY_WIDTH  = 8.0;
static const qreal KEY_HEIGHT  = 40.0;
static const qreal BKEY_HEIGHT = 25.0;

//---------------------------------------------------------
//   HPiano
//---------------------------------------------------------

HPiano::HPiano(QWidget* parent)
   : QGraphicsView(parent)
      {
      setLineWidth(0);
      setMidLineWidth(0);

      setScene(new QGraphicsScene);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      setResizeAnchor(QGraphicsView::AnchorUnderMouse);
      setMouseTracking(true);
      setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
      setDragMode(QGraphicsView::RubberBandDrag);
      setScale(1.5);

      scene()->setSceneRect(0.0, 0.0, KEY_WIDTH * 52, KEY_HEIGHT);

      _firstKey   = 21;
      _lastKey    = 108;   // 88 key piano
      qreal x = 0.0;
      for (int i = _firstKey; i <= _lastKey; ++i) {
            PianoKeyItem* k = new PianoKeyItem(this, i);
            switch(i % 12) {
                  case  0:
                  case  5:
                        k->setType(i == _lastKey ? 6 : 0);
                        k->setPos(x, 0);
                        x += KEY_WIDTH;
                        break;

                  case  2:
                        k->setType(1);
                        k->setPos(x, 0);
                        x += KEY_WIDTH;
                        break;
                  case  4:
                  case 11:
                        k->setType(2);
                        k->setPos(x, 0);
                        x += KEY_WIDTH;
                        break;

                  case  7:
                        k->setType(3);
                        k->setPos(x, 0);
                        x += KEY_WIDTH;
                        break;

                  case  9:
                        k->setType(i == _firstKey ? 5 : 4);
                        k->setPos(x, 0);
                        x += KEY_WIDTH;
                        break;

                  case  1:
                  case  6:
                        k->setType(7);
                        k->setPos(x - BKEY_WIDTH * 5/9, 0);
                        break;
                  case  3:
                  case 10:
                        k->setType(7);
                        k->setPos(x - BKEY_WIDTH * 4/9, 0);
                        break;
                  case 8:
                        k->setType(7);
                        k->setPos(x - BKEY_WIDTH / 2.0, 0);
                        break;
                  }
            keys.append(k);
            scene()->addItem(k);
            }
      }

//---------------------------------------------------------
//   setScale
//---------------------------------------------------------

void HPiano::setScale(qreal s)
      {
       scaleVal = s;
      setMaximumSize(QSize((KEY_WIDTH * 52) * scaleVal + 8, KEY_HEIGHT * scaleVal + 8 + 80));
      setMinimumSize(QSize(100, KEY_HEIGHT * scaleVal + 8));
      QTransform t;
      t.scale(scaleVal, scaleVal);
      setTransform(t, false);
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize HPiano::sizeHint() const
      {
      return QSize(KEY_WIDTH * 52 + 1, KEY_HEIGHT+1);
      }

//---------------------------------------------------------
//   pressKeys
//---------------------------------------------------------

void HPiano::setPressedPitches(QSet<int> pitches)
      {
      _pressedPitches = pitches;
      updateAllKeys();
      }

//---------------------------------------------------------
//   pressPitch
//---------------------------------------------------------

void HPiano::pressPitch(int pitch)
      {
      _pressedPitches.insert(pitch);
      updateAllKeys();
      }

//---------------------------------------------------------
//   releasePitch
//---------------------------------------------------------

void HPiano::releasePitch(int pitch)
      {
      _pressedPitches.remove(pitch);
      updateAllKeys();
      }

//---------------------------------------------------------
//   updateAllKeys
//---------------------------------------------------------

void HPiano::updateAllKeys()
      {
      for (PianoKeyItem* key : keys) {
            key->setPressed(_pressedPitches.contains(key->pitch()));
            key->update();
            }
      }

//---------------------------------------------------------
//   PianoKeyItem
//---------------------------------------------------------

PianoKeyItem::PianoKeyItem(HPiano* _piano, int p)
   : QGraphicsPathItem()
      {
      piano = _piano;
      _pitch = p;
      _pressed = false;
      type = -1;
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void PianoKeyItem::setType(int val)
      {
      type = val;
      QPainterPath path;

      switch(type) {
            case 0:
                  path.moveTo(0,0);
                  path.lineTo(0,   KEY_HEIGHT-2);
                  path.lineTo(2.0, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-2, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-2);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 5/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 5/9, 0);
                  break;
            case 1:
                  path.moveTo(BKEY_WIDTH * 4/9, 0);
                  path.lineTo(BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(0, BKEY_HEIGHT);
                  path.lineTo(0,   KEY_HEIGHT-2);
                  path.lineTo(2.0, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-2, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-2);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, 0);
                  break;
            case 2:
                  path.moveTo(BKEY_WIDTH * 5/9, 0);
                  path.lineTo(BKEY_WIDTH * 5/9, BKEY_HEIGHT);
                  path.lineTo(0,   BKEY_HEIGHT);
                  path.lineTo(0,   KEY_HEIGHT-2);
                  path.lineTo(2.0, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-2, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH,  KEY_HEIGHT-2);
                  path.lineTo(KEY_WIDTH,  BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, 0);
                  break;
            case 3:
                  path.moveTo(BKEY_WIDTH * 4/9, 0);
                  path.lineTo(BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(0, BKEY_HEIGHT);
                  path.lineTo(0,   KEY_HEIGHT-2);
                  path.lineTo(2.0, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-2, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-2);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 5/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 5/9, 0);
                  break;
            case 4:
                  path.moveTo(BKEY_WIDTH * 5/9, 0);
                  path.lineTo(BKEY_WIDTH * 5/9, BKEY_HEIGHT);
                  path.lineTo(0, BKEY_HEIGHT);
                  path.lineTo(0,   KEY_HEIGHT-2);
                  path.lineTo(2.0, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-2, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-2);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, 0);
                  break;
            case 5:
                  path.moveTo(0,0);
                  path.lineTo(0,   KEY_HEIGHT-2);
                  path.lineTo(2.0, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-2, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-2);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, 0);
                  break;
            case 6:
                  path.moveTo(0,0);
                  path.lineTo(0,   KEY_HEIGHT-2);
                  path.lineTo(2.0, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-2, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-2);
                  path.lineTo(KEY_WIDTH, 0);
                  break;
            case 7:
                  path.moveTo(0,0);
                  path.lineTo(0,            BKEY_HEIGHT-1);
                  path.lineTo(1.0,          BKEY_HEIGHT);
                  path.lineTo(BKEY_WIDTH-1, BKEY_HEIGHT);
                  path.lineTo(BKEY_WIDTH,   BKEY_HEIGHT-1);
                  path.lineTo(BKEY_WIDTH, 0);
                  break;
            default:
                  break;
            }
      path.closeSubpath();
      setPath(path);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void PianoKeyItem::mousePressEvent(QGraphicsSceneMouseEvent*)
      {
      _pressed = true;
      update();
      bool ctrl = qApp->keyboardModifiers() & Qt::ControlModifier;
      emit piano->keyPressed(_pitch, ctrl, 80);
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void PianoKeyItem::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
      {
      _pressed = false;
      update();
      emit piano->keyReleased(_pitch, false, 0);
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void PianoKeyItem::paint(QPainter* p, const QStyleOptionGraphicsItem* /*o*/, QWidget*)
      {
      p->setRenderHint(QPainter::Antialiasing, true);
      p->setPen(QPen(Qt::black, .8));
      if (_pressed) {
            QColor c(preferences.pianoHlColor);
            c.setAlpha(180);
            p->setBrush(c);
            }
      else
            p->setBrush(type >= 7 ? Qt::black : Qt::white);
      p->drawPath(path());
      if (_pitch == 60) {
            QFont f("FreeSerif", 8);
            p->setFont(f);
            p->drawText(QRectF(KEY_WIDTH / 2, KEY_HEIGHT - 8, 0, 0),
               Qt::AlignCenter | Qt::TextDontClip, "c'");
            }
      }

//---------------------------------------------------------
//   PianoTools
//---------------------------------------------------------

PianoTools::PianoTools(QWidget* parent)
   : QDockWidget(parent)
      {
      setObjectName("piano");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea));

      _piano = new HPiano;
      _piano->setFocusPolicy(Qt::ClickFocus);
      setWidget(_piano);

      connect(_piano, SIGNAL(keyPressed(int, bool, int)), SIGNAL(keyPressed(int, bool, int)));
      connect(_piano, SIGNAL(keyReleased(int, bool, int)), SIGNAL(keyReleased(int, bool, int)));
      retranslate();
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void PianoTools::retranslate()
      {
      setWindowTitle(tr("Piano Keyboard"));
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void PianoTools::heartBeat(QList<const Ms::Note *> notes)
      {
      QSet<int> pitches;
      for (const Note* note : notes) {
          pitches.insert(note->ppitch());
          }
      _piano->setPressedPitches(pitches);
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void PianoTools::changeEvent(QEvent *event)
      {
      QDockWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void HPiano::wheelEvent(QWheelEvent* event)
      {
      static int deltaSum = 0;
      deltaSum += event->delta();
      int step = deltaSum / 120;
      deltaSum %= 120;

      if (event->modifiers() & Qt::ControlModifier) {
            if (step > 0) {
                  for (int i = 0; i < step; ++i) {
                        scaleVal *= 1.1;
                        }
                  }
            else {
                  for (int i = 0; i < -step; ++i) {
                        scaleVal /= 1.1;
                        }
                  }
            if (scaleVal > 4.0)
                  scaleVal = 4.0;
            else if (scaleVal < .5)
                  scaleVal = .5;
            setScale(scaleVal);
            }
      }
}

