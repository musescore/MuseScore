//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __SQUARECANVAS_H__
#define __SQUARECANVAS_H__

#include "libmscore/pitchvalue.h"

namespace Ms {

//---------------------------------------------------------
//   SqareCanvas
//---------------------------------------------------------

class SquareCanvas : public QFrame {
      Q_OBJECT
      QList<PitchValue> m_points;

      /// The number of rows and columns.
      /// This is in fact the number of lines that are to be drawn.
      int m_rows    = 0;
      int m_columns = 0;
      /// the interval between darker lines.
      /// In the case of rows, since they represent the pitch, the primary lines represent semi-tones.
      /// Moving from a primary line to another means changing the pitch of a semitone.
      int m_primaryColumnsInterval = 0;
      int m_primaryRowsInterval    = 0;

      /// Show negative pitch values. Happens in tremoloBarCanvas.
      bool m_showNegativeRows = false;

      virtual void paintEvent(QPaintEvent*) override;
      virtual void mousePressEvent(QMouseEvent*) override;



   public:
      SquareCanvas(QWidget* parent = nullptr);
      const QList<PitchValue>& points() const { return m_points; }
      QList<PitchValue>& points()             { return m_points; }
      void setPoints(const QList<PitchValue>& p) { m_points = p; }

      int rows() const { return m_rows; }
      /// set the number of lines to draw vertically.
      void setRows(int rows) { m_rows = rows; }
      int columns() const { return m_columns; }
      /// set the number of lines to draw horizontally.
      void setColumns(int columns) { m_columns = columns; }

      int primaryColumnsInterval() const { return m_primaryColumnsInterval; }
      /// set the number of secondary lines between primary ones when drawing the columns.
      void setPrimaryColumnsInterval(int primaryColumnsInterval) { m_primaryColumnsInterval = primaryColumnsInterval; }
      int primaryRowsInterval() const { return m_primaryRowsInterval; }
      /// set the number of secondary lines between primary ones when drawing the rows.
      void setPrimaryRowsInterval(int primaryRowsInterval) { m_primaryRowsInterval = primaryRowsInterval; }

      bool showNegativeRows() const { return m_showNegativeRows; }
      /// sets wheter the canvas should allow negative pitches.
      void setShowNegativeRows(bool showNegativeRows) { m_showNegativeRows = showNegativeRows; }

   signals:
      void canvasChanged();
      };

} // namespace Ms
#endif

