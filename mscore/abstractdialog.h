//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#ifndef __QABSTRACTDIALOG_H__
#define __QABSTRACTDIALOG_H__

namespace Ms {

//---------------------------------------------------------
//   AbstractDialog
//---------------------------------------------------------

class AbstractDialog : public QDialog
      {
      Q_OBJECT

   public:
      AbstractDialog(QWidget * parent = 0, Qt::WindowFlags f = {});

   protected:
      // change language event
      virtual void changeEvent(QEvent *event);

      // translate all strings
      virtual void retranslate() = 0;
      };
}
#endif

