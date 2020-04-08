//=============================================================================
//  LyricsEditor
//
//  Copyright (C) 2015-2020 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef LYRICSEDITOR_H
#define LYRICSEDITOR_H

#include "ui_lyricseditor.h"

namespace Ms {

//---------------------------------------------------------
//   LyricsEditor
//---------------------------------------------------------

class LyricsEditor : public QDockWidget {
      Q_OBJECT

      QScrollArea* sa;
      Ui::LyricsEditor le;

   public:
      LyricsEditor(QWidget* parent = 0);
      void setLyrics(QString lyrics)      { le.textEdit->setText(lyrics); }

      };

} // namespace Ms
#endif // LYRICSEDITOR_H
