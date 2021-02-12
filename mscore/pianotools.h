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

#ifndef __PIANOTOOLS_H__
#define __PIANOTOOLS_H__

#include "libmscore/note.h"
#include "libmscore/select.h"

namespace Ms {

class HPiano;

//---------------------------------------------------------
//   PianoKeyItem
//---------------------------------------------------------

class PianoKeyItem : public QGraphicsPathItem {
      int type;
      int _pitch;
      bool _pressed;
      bool _highlighted;
      bool _selected;
      HPiano* piano;

      virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
      virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
      virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent*);

   public:
      PianoKeyItem(HPiano* , int p);
      void setType(int val);
      int pitch() { return _pitch; }
      void setPressed(bool p) { _pressed = p; }
      void setHighlighted(bool h) { _highlighted = h; }
      void setSelected(bool s) { _selected = s; }
      };

//---------------------------------------------------------
//   HPiano
//---------------------------------------------------------

class HPiano : public QGraphicsView {
      Q_OBJECT
      int _firstKey;
      int _lastKey;
      //Pitches pressed due to playback
      QSet<int> _pressedPlaybackPitches;
      //Pitches pressed due to user interaction
      QSet<int> _pressedPitches;
      QList<PianoKeyItem*> keys;
      qreal scaleVal;
      virtual void wheelEvent(QWheelEvent*);
      virtual bool event(QEvent* event);
      bool gestureEvent(QGestureEvent *event);
      void setScale(qreal);

   signals:
      void keyPressed(int pitch, bool chord, int velo);
      void keyReleased(int pitch, bool chord, int velo);

   public:
      HPiano(QWidget* parent = 0);
      friend class PianoKeyItem;
      void setPressedPlaybackPitches(QSet<int> pitches);
      // User interaction
      void pressPitch(int pitch);
      void releasePitch(int pitch);
      // Playback
      void pressPlaybackPitch(int pitch);
      void releasePlaybackPitch(int pitch);
      void clearSelection();
      void changeSelection(const Selection& selection);
      void updateAllKeys();
      QSet<int>& pressedPlaybackPitches() { return _pressedPlaybackPitches; }
      virtual QSize sizeHint() const;

   public slots:
      void setMaximum(bool top_level);
      };

//---------------------------------------------------------
//   PianoTools
//---------------------------------------------------------

class PianoTools : public QDockWidget {
      Q_OBJECT

      HPiano* _piano;

   signals:
      void keyPressed(int pitch, bool chord, int vel);
      void keyReleased(int pitch, bool chord, int vel);

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate();

   public:
      PianoTools(QWidget* parent = 0);
      // User Interaction
      void pressPitch(int pitch)    { _piano->pressPitch(pitch);   }
      void releasePitch(int pitch)  { _piano->releasePitch(pitch); }
      // Playback
      void pressPlaybackPitch(int pitch)   { _piano->pressPlaybackPitch(pitch);   }
      void releasePlaybackPitch(int pitch) { _piano->releasePlaybackPitch(pitch); }
      QSet<int> pressedPlaybackPitches()   { return _piano->pressedPlaybackPitches(); }
      void setPlaybackNotes(QList<const Note*> notes);
      void clearSelection();
      void changeSelection(const Selection& selection);
      void updateAllKeys() { _piano->updateAllKeys(); }
      };


} // namespace Ms
#endif

