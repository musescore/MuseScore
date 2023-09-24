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

#include "testutils.h"

void initMuseScoreResources()
      {
#ifdef Q_OS_MAC
      Q_INIT_RESOURCE(musescore);
      Q_INIT_RESOURCE(qml);
      Q_INIT_RESOURCE(musescorefonts_Mac);
      Q_INIT_RESOURCE(shortcut_Mac);
#else
      Q_INIT_RESOURCE(musescore);
      Q_INIT_RESOURCE(qml);
      Q_INIT_RESOURCE(musescorefonts_Leland);
      Q_INIT_RESOURCE(musescorefonts_Edwin);
      Q_INIT_RESOURCE(musescorefonts_MScore);
      Q_INIT_RESOURCE(musescorefonts_Gootville);
      Q_INIT_RESOURCE(musescorefonts_Bravura);
      Q_INIT_RESOURCE(musescorefonts_MuseJazz);
      Q_INIT_RESOURCE(musescorefonts_Campania);
      Q_INIT_RESOURCE(musescorefonts_FreeSerif);
      Q_INIT_RESOURCE(musescorefonts_Free);
      Q_INIT_RESOURCE(musescorefonts_FinaleMaestro);
      Q_INIT_RESOURCE(musescorefonts_FinaleBroadway);
      Q_INIT_RESOURCE(shortcut);
#endif
      }
