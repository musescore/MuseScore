//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.h 3734 2010-12-01 10:47:29Z wschweer $
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

#ifndef __TEXTPALETTE_H__
#define __TEXTPALETTE_H__

#include "ui_textpalette.h"

namespace Ms {

class Text;
class Palette;

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

class TextPalette : public QWidget, public Ui::TextPaletteBase {
      Q_OBJECT

      Text* _textElement;
      QFont _font;
      Palette* pCommon;
      Palette* pSmufl;
      Palette* pUnicode;

      QListWidget* lws;
      QListWidget* lwu;

      virtual void hideEvent(QHideEvent*);
      void closeEvent(QCloseEvent* ev);
      void populateCommon();

   private slots:
      void populateSmufl();
      void populateUnicode();

   public:
      TextPalette(QWidget* parent);
      void setText(Text* te);
      Text* text() { return _textElement; }
      void setFont(const QFont& font);
      };
}

#endif

