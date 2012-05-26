//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textprop.h 5427 2012-03-07 12:41:34Z wschweer $
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

#ifndef __TEXTPROP_H__
#define __TEXTPROP_H__

#include "ui_textproperties.h"
#include "libmscore/style.h"

class Text;
class Score;

//---------------------------------------------------------
//   TextProp
//---------------------------------------------------------

class TextProp : public QWidget, public Ui::TextProperties {
      Q_OBJECT

      int curUnit;
      bool onlyStyle;

   private slots:
      void mmToggled(bool);
      void styledToggled(bool);
      void unstyledToggled(bool);

   public:
      TextProp(QWidget* parent = 0);
      void setScore(bool _onlyStyle, Score*);
      void setTextStyle(const TextStyle&);
      TextStyle textStyle() const;
      bool isStyled() const;
      void setStyled(bool val);
      void setTextStyleType(int);
      int textStyleType() const;
      };

#endif

