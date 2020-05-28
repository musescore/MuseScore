//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

/**
 * \file
 * Definitions of global variables used in mtests not linked to mscoreapp target
 */

#include "config.h"
#include "mscore/icons.h"
#include "mscore/musescore.h"

Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg)

namespace Ms {
QString revision;

MasterSynthesizer* synti;
QString dataPath;
QIcon* icons[1];
QString mscoreGlobalShare;
}
