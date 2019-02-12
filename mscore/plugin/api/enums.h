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

#ifndef __PLUGIN_API_ENUMS_H__
#define __PLUGIN_API_ENUMS_H__

#include <QQmlPropertyMap>

namespace Ms {
namespace PluginAPI {

class Enum : public QQmlPropertyMap {
      Q_OBJECT

   public:
      Enum(const QMetaEnum& _enum, QObject* parent = nullptr);
      };

template <class T>
Enum* wrapEnum(QObject* parent = nullptr)
      {
      return new Enum(QMetaEnum::fromType<T>(), parent);
      }

} // namespace PluginAPI
} // namespace Ms
#endif
