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

#ifndef __QMLPRIVATEAPI_H__
#define __QMLPRIVATEAPI_H__

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
///   \cond PLUGIN_API \private \endcond
///   \class QmlPrivateAPI
///   Contains QML bindings used in MuseScore that do not
///   have an interface that is stable enough for reliable
///   usage in plugins.
//---------------------------------------------------------

class QmlPrivateAPI : public QObject {
      Q_OBJECT
   public:
      QmlPrivateAPI(QObject* parent = nullptr) : QObject(parent) {}

      Q_INVOKABLE QStringList getOpenScoreNames(bool singleFile = false);
      };

} // namespace PluginAPI
} // namespace Ms
#endif
