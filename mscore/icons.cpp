//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "globals.h"
#include "icons.h"
#include "libmscore/score.h"
#include "libmscore/style.h"
#include "preferences.h"
#include "libmscore/sym.h"
#include "libmscore/mscore.h"
#include "miconengine.h"

namespace Ms {

extern QString iconPath;
QIcon* icons[int(Icons::ICONS)];

//---------------------------------------------------------
//   genIcons
//    create some icons
//---------------------------------------------------------

static const char* iconNames[] = {
      "note-longa.svg",
      "note-breve.svg",
      "note-1.svg",
      "note-2.svg",
      "note-4.svg",
      "note-8.svg",
      "note-16.svg",
      "note-32.svg",
      "note-64.svg",
      "note-128.svg",
      "note-natural.svg",
      "note-sharp.svg",
      "note-double-sharp.svg",
      "note-flat.svg",
      "note-double-flat.svg",
      "rest.svg",
      "note-dot.svg",
      "note-double-dot.svg",
      "note-dot3.svg",
      "note-dot4.svg",
      "stem-flip.svg",
      "edit-undo.svg",
      "edit-redo.svg",
      "edit-cut.svg",
      "edit-copy.svg",
      "edit-paste.svg",
      "edit-swap.svg",
      "document-print.svg",
      "clef.svg",
      "midi-input.svg",
      "sound-while-editing.svg",
      "media-skip-backward.svg",
      "media-playback-start.svg",
      "media-playback-repeats.svg",
      "media-playback-pan.svg",
      "sbeam.svg",
      "mbeam.svg",
      "nbeam.svg",
      "beam32.svg",
      "beam64.svg",
      "default.svg",
      "fbeam1.svg",
      "fbeam2.svg",
      "document.svg",
      "document-open.svg",
      "document-new.svg",
      "document-save.svg",
      "document-save-as.svg",
      "mscore.svg",
      "acciaccatura.svg",
      "appoggiatura.svg",
      "grace4.svg",
      "grace16.svg",
      "grace32.svg",
      "grace8after.svg",
      "grace16after.svg",
      "grace32after.svg",
      "mode-notes.svg",
      // "mode-notes-steptime.svg", (using normal icon for the time being.)
      "mode-notes-repitch.svg",
      "mode-notes-rhythm.svg",
      "mode-notes-realtime-auto.svg",
      "mode-notes-realtime-manual.svg",
      "insert-symbol.svg",
      "note-tie.svg",
      "format-text-bold.svg",
      "format-text-italic.svg",
      "format-text-underline.svg",
      "format-justify-left.svg",
      "format-justify-center.svg",
      "format-justify-right.svg",
      "align-vertical-top.svg",
      "align-vertical-bottom.svg",
      "align-vertical-center.svg",
      "align-vertical-baseline.svg",
      "format-text-superscript.svg",
      "format-text-subscript.svg",
      "mode-photo.svg",
      "raster-horizontal.svg",
      "raster-vertical.svg",
      "list-unordered.svg",
      "list-ordered.svg",
      "format-indent-more.svg",
      "format-indent-less.svg",
      "media-playback-loop.svg",
      "media-playback-loop-in.svg",
      "media-playback-loop-out.svg",
      "media-playback-metronome.svg",
      "media-playback-countin.svg",
      "frame-vertical.svg",
      "frame-horizontal.svg",
      "frame-text.svg",
      "frame-fretboard.svg",
      "measure.svg",
      "object-select.svg",
      "help-contents.svg",
      "go-home.svg",
      "go-previous.svg",
      "go-next.svg",
      "view-refresh.svg",
      "parentheses.svg",
      "brackets.svg",
      "timesig_allabreve.svg",
      "timesig_common.svg",
      "timesig_prolatio01.svg",
      "timesig_prolatio02.svg",
      "timesig_prolatio03.svg",
      "timesig_prolatio04.svg",
      "timesig_prolatio05.svg",
      "timesig_prolatio07.svg",
      "timesig_prolatio08.svg",
      "timesig_prolatio10.svg",
      "timesig_prolatio11.svg",
      "edit.svg",
      "edit-reset.svg",
      "window-close.svg",
      "arrow_up.svg",
      "arrow_down.svg",
      "mail.svg",
      "bug.svg",
      "note_timewise.svg"
      };

//---------------------------------------------------------
//   genIcons
//---------------------------------------------------------

void genIcons()
      {
      for (int i = 0; i < int(Icons::voice1_ICON); ++i) {
            QIcon* icon = new QIcon(new MIconEngine);
            icon->addFile(iconPath + iconNames[i]);
            icons[i] = icon;
            if (icon->isNull() || icon->pixmap(12).isNull()) {
                  qDebug("cannot load Icon <%s>", qPrintable(iconPath + iconNames[i]));
                  }
            }

      static const char* vtext[VOICES] = { "1","2","3","4" };
      int iw = preferences.getInt(PREF_UI_THEME_ICONHEIGHT) * 2 / 3; // 16;
      int ih = preferences.getInt(PREF_UI_THEME_ICONHEIGHT);   // 24;
      for (int i = 0; i < VOICES; ++i) {
            icons[int(Icons::voice1_ICON) + i] = new QIcon;
            QPixmap image(iw, ih);
            QColor c(MScore::selectColor[i].lighter(180));
            image.fill(c);
            QPainter painter(&image);
            painter.setFont(QFont("FreeSans", 8));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, iw, ih), Qt::AlignCenter, vtext[i]);
            painter.end();
            icons[int(Icons::voice1_ICON) +i]->addPixmap(image);

            painter.begin(&image);
            c = QColor(MScore::selectColor[i].lighter(140));
            painter.fillRect(0, 0, iw, ih, c);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, iw, ih), Qt::AlignCenter, vtext[i]);
            painter.end();
            icons[int(Icons::voice1_ICON) + i]->addPixmap(image, QIcon::Normal, QIcon::On);
            }
      }
}

