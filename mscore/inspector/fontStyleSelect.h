//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __FONT_STYLE_SELECT_H__
#define __FONT_STYLE_SELECT_H__

#include "ui_font_style_select.h"
#include "libmscore/types.h"

namespace Ms {

enum class Align : char;

//---------------------------------------------------------
//   FontStyleSelect
//---------------------------------------------------------

class FontStyleSelect : public QWidget, public Ui::FontStyleSelect {
      Q_OBJECT

   private slots:
      void _fontStyleChanged();

   signals:
      void fontStyleChanged(FontStyle);

   public:
      FontStyleSelect(QWidget* parent);
      FontStyle fontStyle() const;
      void setFontStyle(FontStyle);
      };

}

#endif

