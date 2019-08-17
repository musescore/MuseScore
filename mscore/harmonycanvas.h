//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009-2011 Werner Schweer and others
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

#ifndef __HARMONYCANVAS_H__
#define __HARMONYCANVAS_H__

#include "libmscore/harmony.h"

namespace Ms {

struct TextSegment;
struct ChordDescription;
class ChordList;

//---------------------------------------------------------
//   HarmonyCanvas
//---------------------------------------------------------

class HarmonyCanvas : public QFrame {
      Q_OBJECT

      QList<TextSegment*> textList;       // rendered chord
      ChordDescription* chordDescription;
      ChordList* chordList;
      QTransform _matrix, imatrix;
      double extraMag;
      QPointF startMove;
      TextSegment* moveElement;

      Element* dragElement;

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragLeaveEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);

      void render(const QList<RenderAction>&, double&, double&, int, NoteSpellingType, NoteCaseType);

   private slots:
      void deleteAction();

   public:
      HarmonyCanvas(QWidget* parent = 0);
      void setChordDescription(ChordDescription* sd, ChordList* cl);
      const QList<TextSegment*>& getTextList() const { return textList; }
      void updateChordDescription();
      };


} // namespace Ms
#endif

