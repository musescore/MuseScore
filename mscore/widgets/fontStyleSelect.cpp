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

#include "fontStyleSelect.h"
#include "inspectoriconloader.h"

namespace Ms {

//---------------------------------------------------------
//    FontStyleSelect
//---------------------------------------------------------

FontStyleSelect::FontStyleSelect(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);

      bold->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_FORMAT_BOLD_ICON));
      italic->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_FORMAT_ITALIC_ICON));
      underline->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_FORMAT_UNDERLINE_ICON));

      connect(bold, SIGNAL(toggled(bool)), SLOT(_fontStyleChanged()));
      connect(italic, SIGNAL(toggled(bool)), SLOT(_fontStyleChanged()));
      connect(underline, SIGNAL(toggled(bool)), SLOT(_fontStyleChanged()));
      }

//---------------------------------------------------------
//   _fontStyleChanged
//---------------------------------------------------------

void FontStyleSelect::_fontStyleChanged()
      {
      emit fontStyleChanged(fontStyle());
      }

//---------------------------------------------------------
//   fontStyle
//---------------------------------------------------------

FontStyle FontStyleSelect::fontStyle() const
      {
      FontStyle fs = FontStyle::Normal;

      if (bold->isChecked())
            fs = fs + FontStyle::Bold;
      if (italic->isChecked())
            fs = fs + FontStyle::Italic;
      if (underline->isChecked())
            fs = fs + FontStyle::Underline;

      return fs;
      }

//---------------------------------------------------------
//   setFontStyle
//---------------------------------------------------------

void FontStyleSelect::setFontStyle(FontStyle fs)
      {
      bold->setChecked(fs & FontStyle::Bold);
      italic->setChecked(fs & FontStyle::Italic);
      underline->setChecked(fs & FontStyle::Underline);
      }

}

