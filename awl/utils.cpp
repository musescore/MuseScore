//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
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

#include "utils.h"

static const char* vall[] = {
      QT_TRANSLATE_NOOP("awlutils", "c"),
      QT_TRANSLATE_NOOP("awlutils", "c#"),
      QT_TRANSLATE_NOOP("awlutils", "d"),
      QT_TRANSLATE_NOOP("awlutils", "d#"),
      QT_TRANSLATE_NOOP("awlutils", "e"),
      QT_TRANSLATE_NOOP("awlutils", "f"),
      QT_TRANSLATE_NOOP("awlutils", "f#"),
      QT_TRANSLATE_NOOP("awlutils", "g"),
      QT_TRANSLATE_NOOP("awlutils", "g#"),
      QT_TRANSLATE_NOOP("awlutils", "a"),
      QT_TRANSLATE_NOOP("awlutils", "a#"),
      QT_TRANSLATE_NOOP("awlutils", "b")
      };
static const char* valu[] = {
      QT_TRANSLATE_NOOP("awlutils", "C"),
      QT_TRANSLATE_NOOP("awlutils", "C#"),
      QT_TRANSLATE_NOOP("awlutils", "D"),
      QT_TRANSLATE_NOOP("awlutils", "D#"),
      QT_TRANSLATE_NOOP("awlutils", "E"),
      QT_TRANSLATE_NOOP("awlutils", "F"),
      QT_TRANSLATE_NOOP("awlutils", "F#"),
      QT_TRANSLATE_NOOP("awlutils", "G"),
      QT_TRANSLATE_NOOP("awlutils", "G#"),
      QT_TRANSLATE_NOOP("awlutils", "A"),
      QT_TRANSLATE_NOOP("awlutils", "A#"),
      QT_TRANSLATE_NOOP("awlutils", "B")
      };

namespace Awl {

//---------------------------------------------------------
//   pitch2string
//---------------------------------------------------------

QString pitch2string(int v)
      {
      if (v < 0 || v > 127)
            return QString("----");
      int octave = (v / 12) - 2;
      QString o;
      o.sprintf("%d", octave);
      int i = v % 12;
      return qApp->translate("awlutils", octave < 0 ? valu[i] : vall[i]) + o;
      }
}

