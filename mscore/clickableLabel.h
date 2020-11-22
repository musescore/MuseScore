//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#ifndef __CLICKABLE_LABEL_H__
#define __CLICKABLE_LABEL_H__

namespace Ms {

class ClickableLabel : public QLabel {
      Q_OBJECT

   protected:
      void mousePressEvent(QMouseEvent* event);

   public:
      explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
      ~ClickableLabel();

   signals:
      void clicked();
};
}

#endif
