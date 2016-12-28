//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: text.h -1   $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __TEXTPROPERTIES_H__
#define __TEXTPROPERTIES_H__

namespace Ms {

class TextProp;
class Text;

//---------------------------------------------------------
//   TextProperties
//---------------------------------------------------------

class TextProperties : public QDialog {
      Q_OBJECT
      TextProp* tp;
      Text* text;
      QCheckBox* cb;

      virtual void hideEvent(QHideEvent*);
   private slots:
      virtual void accept();
      void resetToStyle();

   public:
      TextProperties(Text*, QWidget* parent = 0);
      bool applyToAll() const { return cb->isChecked(); }
      };
}

#endif
