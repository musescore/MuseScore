//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#include "enums.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Enum::Enum
//---------------------------------------------------------

Enum::Enum(const QMetaEnum& _enum, QObject* parent)
   : QQmlPropertyMap(this, parent)
      {
      const int nkeys = _enum.keyCount();
      for (int i = 0; i < nkeys; ++i)
            insert(_enum.key(i), _enum.value(i));
      }

} // namespace PluginAPI
} // namespace Ms
