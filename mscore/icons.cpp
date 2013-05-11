//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: icons.cpp 5246 2012-01-24 18:48:55Z wschweer $
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

extern QString iconPath, iconGroup;
QIcon* icons[ICONS];

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
      "stem-flip.svg",
      "edit-undo.svg",
      "edit-redo.svg",
      "edit-cut.svg",
      "edit-copy.svg",
      "edit-paste.svg",
      "document-print.svg",
      "clef.svg",
      "toggle-midi.svg",
      "toggle-volume.svg",
      "media-skip-backward.svg",
      "media-playback-start.svg",
      "toggle-play-repeats.svg",
      "toggle-panning.svg",
      "sbeam.svg",
      "mbeam.svg",
      "nbeam.svg",
      "beam32.svg",
      "beam64.svg",
      "abeam.svg",
      "fbeam1.svg",
      "fbeam2.svg",
      "document-open.svg",
      "document-new.svg",
      "document-save.svg",
      "document-save-as.svg",
      "application-exit.svg",
      "mscore.png",
      "acciaccatura.svg",
      "appoggiatura.svg",
      "grace4.svg",
      "grace8b.svg",
      "grace16.svg",
      "grace32.svg",
      "mode-notes.svg",
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
      "align-vertical-middle.svg",
      "align-vertical-baseline.svg",
      "format-text-superscript.svg",
      "format-text-subscript.svg",
      "mode-photo.svg",
      "hraster.svg",
      "vraster.svg",
      "mode-repitch.svg",
      "list-unordered.svg",
      "list-ordered.svg",
      "format-indent-more.svg",
      "format-indent-less.svg",
      "panel-community.svg",
      "toggle-metronome.svg",
      "vframe.svg",
      "hframe.svg",
      "tframe.svg",
      "fframe.svg",
      "measure.svg",
      "checkmark.svg"
      };

void genIcons()
      {
// qDebug("genIcons <%s>\n", qPrintable(iconPath + iconGroup));
      for (int i = 0; i < voice1_ICON; ++i) {
            icons[i] = new QIcon(iconPath + iconGroup + iconNames[i]);
            if (icons[i]->isNull() || icons[i]->pixmap(12).isNull()) {
                  qDebug("cannot load Icon <%s>\n", qPrintable(iconPath + iconGroup + iconNames[i]));
                  }
            }

      static const char* vtext[VOICES] = { "1","2","3","4" };
      int iw = preferences.iconHeight * 2 / 3; // 16;
      int ih = preferences.iconHeight;   // 24;
      for (int i = 0; i < VOICES; ++i) {
            icons[voice1_ICON + i] = new QIcon;
            QPixmap image(iw, ih);
            QColor c(MScore::selectColor[i].light(180));
            image.fill(c);
            QPainter painter(&image);
            painter.setFont(QFont("FreeSans", 8));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, iw, ih), Qt::AlignCenter, vtext[i]);
            painter.end();
            icons[voice1_ICON +i]->addPixmap(image);

            painter.begin(&image);
            c = QColor(MScore::selectColor[i].light(140));
            painter.fillRect(0, 0, iw, ih, c);
            painter.setPen(QPen(Qt::black));
            painter.drawText(QRect(0, 0, iw, ih), Qt::AlignCenter, vtext[i]);
            painter.end();
            icons[voice1_ICON + i]->addPixmap(image, QIcon::Normal, QIcon::On);
            }
      }

