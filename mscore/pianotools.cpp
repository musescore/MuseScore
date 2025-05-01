//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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
#include "libmscore/chord.h"

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
      scaleVal = 1.0;
      setLineWidth(0);
      setMidLineWidth(0);

      setScene(new QGraphicsScene);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      setResizeAnchor(QGraphicsView::AnchorUnderMouse);
      setMouseTracking(true);
      setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
      setDragMode(QGraphicsView::RubberBandDrag);
      setScale(2.5);

      grabGesture(Qt::PinchGesture);      // laptop pad (Mac) and touchscreen

      scene()->setSceneRect(0.0, 0.0, KEY_WIDTH * 52, KEY_HEIGHT);
      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      setSizePolicy(policy);
      int margin = 16;
      setMaximumSize(QSize((KEY_WIDTH * 52 + margin/2) * scaleVal, 1000));

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
      if (s > 16.0)
            s = 16.0;
      else if (s < .5)
            s = .5;
      if (s != scaleVal) {
            scaleVal = s;
            int margin = 16;
            QDockWidget* par = static_cast<QDockWidget*>(parent());
            if (par) {
                  if (!par->isFloating())
                        setMaximumSize(QSize((KEY_WIDTH * 52 + margin/2) * scaleVal, 1000));
                  else
                        setMaximumSize(QSize((KEY_WIDTH * 52 + margin/2) * scaleVal, (KEY_HEIGHT + margin) * scaleVal));
                  }
            else
                  setMaximumSize(QSize((KEY_WIDTH * 52 + margin/2) * scaleVal, (KEY_HEIGHT + margin) * scaleVal));
            setMinimumSize(QSize(100 * scaleVal, (KEY_HEIGHT + margin) * scaleVal));
            QTransform t;
            t.scale(scaleVal, scaleVal);
            setTransform(t, false);
            }
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

void HPiano::setPressedPlaybackPitches(QSet<int> pitches)
      {
      _pressedPlaybackPitches = pitches;
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
//   pressPlaybackPitch
//---------------------------------------------------------

void HPiano::pressPlaybackPitch(int pitch)
      {
      _pressedPlaybackPitches.insert(pitch);
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
//   releasePlaybackPitch
//---------------------------------------------------------

void HPiano::releasePlaybackPitch(int pitch)
      {
      _pressedPlaybackPitches.remove(pitch);
      updateAllKeys();
      }

//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void HPiano::changeSelection(const Selection& selection)
      {
      for (PianoKeyItem* key : qAsConst(keys)) {
            key->setHighlighted(false);
            key->setSelected(false);
            }
      for (Note* n : selection.noteList()) {
            if (n->epitch() >= _firstKey && n->epitch() <= _lastKey)
                  keys[n->epitch() - _firstKey]->setSelected(true);
            for (Note* other : n->chord()->notes())
                  if (other->epitch() >= _firstKey && other->epitch() <= _lastKey)
                        keys[other->epitch() - _firstKey]->setHighlighted(true);
            }
      for (PianoKeyItem* key : qAsConst(keys))
            key->update();
      // Force redraw
      scene()->invalidate();
      }

// used when currentScore() is NULL; same as above except the for loop
void HPiano::clearSelection()
      {
      for (PianoKeyItem* key : qAsConst(keys)) {
            key->setHighlighted(false);
            key->setSelected(false);
            key->update();
            }
      // Force redraw
      scene()->invalidate();
      }

//---------------------------------------------------------
//   updateAllKeys
//---------------------------------------------------------

void HPiano::updateAllKeys()
      {
      for (PianoKeyItem* key : qAsConst(keys)) {
            key->setPressed(_pressedPitches.contains(key->pitch())
                            || _pressedPlaybackPitches.contains(key->pitch()));
            key->update();
            }
      // Force redraw
      scene()->invalidate();
      }

void HPiano::setMaximum(bool top_level) {
      int margin = 16;
      if (!top_level)
            setMaximumSize(QSize((KEY_WIDTH * 52 + margin/2) * scaleVal, 1000));
      else
            setMaximumSize(QSize((KEY_WIDTH * 52 + margin/2) * scaleVal, (KEY_HEIGHT + margin) * scaleVal));
      updateAllKeys();
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
      _selected = false;
      _highlighted = false;
      type = -1;

      if (preferences.getBool(PREF_UI_PIANO_SHOWPITCHHELP)) { // changes to that setting take effect only after restarting MuseScore though
            const char* pitchNames[] = {"C", "C♯", "D", "D♯", "E", "F", "F♯", "G", "G♯", "A", "A♯", "B"}; // keep in sync with `valu` in limbscore/utils.cpp
            QString text = qApp->translate("utils", pitchNames[_pitch % 12]) + QString::number((_pitch / 12) - 1);
            setToolTip(text);
            }
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void PianoKeyItem::setType(int val)
      {
      type = val;
      QPainterPath path;
      qreal triangle = 1.0;
      qreal htriangle = triangle/2;
      switch(type) {
            case 0:
                  path.moveTo(0,0);
                  path.lineTo(0,   KEY_HEIGHT-triangle);
                  path.lineTo(triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-triangle);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 5/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 5/9, 0);
                  break;
            case 1:
                  path.moveTo(BKEY_WIDTH * 4/9, 0);
                  path.lineTo(BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(0, BKEY_HEIGHT);
                  path.lineTo(0,   KEY_HEIGHT-triangle);
                  path.lineTo(triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-triangle);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, 0);
                  break;
            case 2:
                  path.moveTo(BKEY_WIDTH * 5/9, 0);
                  path.lineTo(BKEY_WIDTH * 5/9, BKEY_HEIGHT);
                  path.lineTo(0,   BKEY_HEIGHT);
                  path.lineTo(0,   KEY_HEIGHT-triangle);
                  path.lineTo(triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH,  KEY_HEIGHT-triangle);
                  path.lineTo(KEY_WIDTH,  BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, 0);
                  break;
            case 3:
                  path.moveTo(BKEY_WIDTH * 4/9, 0);
                  path.lineTo(BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(0, BKEY_HEIGHT);
                  path.lineTo(0,   KEY_HEIGHT-triangle);
                  path.lineTo(triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-triangle);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 5/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 5/9, 0);
                  break;
            case 4:
                  path.moveTo(BKEY_WIDTH * 5/9, 0);
                  path.lineTo(BKEY_WIDTH * 5/9, BKEY_HEIGHT);
                  path.lineTo(0, BKEY_HEIGHT);
                  path.lineTo(0,   KEY_HEIGHT-triangle);
                  path.lineTo(triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-triangle);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, 0);
                  break;
            case 5:
                  path.moveTo(0,0);
                  path.lineTo(0,   KEY_HEIGHT-triangle);
                  path.lineTo(triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-triangle);
                  path.lineTo(KEY_WIDTH, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, BKEY_HEIGHT);
                  path.lineTo(KEY_WIDTH - BKEY_WIDTH * 4/9, 0);
                  break;
            case 6:
                  path.moveTo(0,0);
                  path.lineTo(0,   KEY_HEIGHT-triangle);
                  path.lineTo(triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH-triangle, KEY_HEIGHT);
                  path.lineTo(KEY_WIDTH, KEY_HEIGHT-triangle);
                  path.lineTo(KEY_WIDTH, 0);
                  break;
            case 7:
                  path.moveTo(0,0);
                  path.lineTo(0,           BKEY_HEIGHT-htriangle);
                  path.lineTo(htriangle,   BKEY_HEIGHT);
                  path.lineTo(BKEY_WIDTH-htriangle, BKEY_HEIGHT);
                  path.lineTo(BKEY_WIDTH,  BKEY_HEIGHT-htriangle);
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
      bool chord = qApp->keyboardModifiers() & Qt::ShiftModifier;
      emit piano->keyPressed(_pitch, chord, 80);
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
            QColor c(preferences.getColor(PREF_UI_PIANO_HIGHLIGHTCOLOR));
            c.setAlpha(180);
            p->setBrush(c);
            }
      else if (_selected) {
            QColor c(preferences.getColor(PREF_UI_PIANO_HIGHLIGHTCOLOR));
            c.setAlpha(100);
            p->setBrush(c);
            }
      else if (_highlighted)
            p->setBrush(type >= 7 ? QColor(125, 125, 125) : QColor(200, 200, 200));
      else
            p->setBrush(type >= 7 ? Qt::black : Qt::white);
      p->drawPath(path());
      if (preferences.getBool(PREF_UI_PIANO_SHOWPITCHHELP) && _pitch % 12 == 0) {
            QFont f("Edwin", 6);
            p->setFont(f);
            QString text = "C" + QString::number((_pitch / 12) - 1);
            p->drawText(QRectF(KEY_WIDTH / 2, KEY_HEIGHT - 8, 0, 0),
               Qt::AlignCenter | Qt::TextDontClip, text);
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

      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      setSizePolicy(policy);

      connect(_piano, SIGNAL(keyPressed(int, bool, int)), SIGNAL(keyPressed(int, bool, int)));
      connect(_piano, SIGNAL(keyReleased(int, bool, int)), SIGNAL(keyReleased(int, bool, int)));
      connect(this, SIGNAL(topLevelChanged(bool)), _piano, SLOT(setMaximum(bool)));
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

void PianoTools::setPlaybackNotes(QList<const Ms::Note *> notes)
      {
      QSet<int> pitches;
      for (const Note* note : notes) {
            pitches.insert(note->ppitch());
            }
      _piano->setPressedPlaybackPitches(pitches);
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
      deltaSum += event->angleDelta().y();
      int step = deltaSum / 120;
      deltaSum %= 120;
      qreal mag = scaleVal;
      if (event->modifiers() & Qt::ControlModifier) {
            if (step > 0) {
                  for (int i = 0; i < step; ++i)
                        mag *= 1.1;
                  }
            else {
                  for (int i = 0; i < -step; ++i)
                        mag /= 1.1;
                  }
            setScale(mag);
            }
      }

//---------------------------------------------------------
//   gestureEvent
//    fired on touchscreen gestures as well as Mac touchpad gestures
//---------------------------------------------------------

bool HPiano::event(QEvent* event)
      {
      if (event->type() == QEvent::Gesture) {
            return gestureEvent(static_cast<QGestureEvent*>(event));
            }
      return QGraphicsView::event(event);
      }

//---------------------------------------------------------
//   gestureEvent
//    fired on touchscreen gestures as well as Mac touchpad gestures
//---------------------------------------------------------

bool HPiano::gestureEvent(QGestureEvent *event)
      {
      if (QGesture *gesture = event->gesture(Qt::PinchGesture)) {
            // Zoom in/out when receiving a pinch gesture
            QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);

            static qreal magStart = 1.0;
            if (pinch->state() == Qt::GestureStarted) {
                  magStart = scaleVal;
                  }
            if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged) {
                  // On Windows, totalScaleFactor() contains the net magnification.
                  // On OS X, totalScaleFactor() is 1, and scaleFactor() contains the net magnification.
                  qreal value = pinch->totalScaleFactor();
                  if (value == 1) {
                        value = pinch->scaleFactor();
                        }
                  // Qt 5.4 doesn't report pinch->centerPoint() correctly
                  setScale(magStart*value);
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void PianoTools::changeSelection(const Selection& selection)
      {
      _piano->changeSelection(selection);
      }

void PianoTools::clearSelection()
      {
      _piano->clearSelection();
      }
}
