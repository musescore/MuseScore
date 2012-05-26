//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: voiceselector.h 3445 2010-09-09 08:59:45Z wschweer $
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

#ifndef __VOICESELECTOR_H__
#define __VOICESELECTOR_H__

//---------------------------------------------------------
//   VoiceSelector
//---------------------------------------------------------

class VoiceSelector : public QFrame {
      Q_OBJECT

   signals:
      void triggered(QAction*);

   public:
      VoiceSelector(QWidget* parent = 0);
      };


//---------------------------------------------------------
//   VoiceButton
//---------------------------------------------------------

class VoiceButton : public QToolButton {
      Q_OBJECT
      int voice;

      virtual void paintEvent(QPaintEvent*);

   public:
      VoiceButton(int voice, QWidget* parent = 0);
      virtual QSize sizeHint() const { return QSize(16,8); }
      };

#endif

