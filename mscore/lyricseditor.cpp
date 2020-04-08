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

#include "lyricseditor.h"

namespace Ms {

//---------------------------------------------------------
//   LyricsEditor::LyricsEditor
/// Named "Lyrics Viewer" in the UI since it doesn't yet support editing.
/// When adding ability to edit, remember to rename here and in shortcut.cpp.
/// Also remember to reset the following properties to default values for the
/// textEdit in lyricseditor.ui: readOnly, textInteractionFlags
//---------------------------------------------------------

LyricsEditor::LyricsEditor(QWidget* parent)
   : QDockWidget(tr("Lyrics Viewer"), parent)
      {
      setObjectName("LyricsViewer");
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      sa = new QScrollArea;
      sa->setFrameShape(QFrame::NoFrame);
      sa->setWidgetResizable(true);
      sa->setFocusPolicy(Qt::NoFocus);

      le.setupUi(sa);
      setWidget(sa);
      }

} // namespace Ms

