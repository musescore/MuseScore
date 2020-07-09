//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "style.h"

#include "libmscore/score.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   wrap
//---------------------------------------------------------

MStyle* wrap(Ms::MStyle* style, Ms::Score* score)
      {
      MStyle* st = new MStyle(style, score);
      // All wrapper objects should belong to JavaScript code.
      QQmlEngine::setObjectOwnership(st, QQmlEngine::JavaScriptOwnership);
      return st;
      }

//---------------------------------------------------------
//   MStyle::keyToSid
//---------------------------------------------------------

Sid MStyle::keyToSid(const QString& key) {
      static QMetaEnum sidEnum = QMetaEnum::fromType<Sid>();

      bool ok;
      int val = sidEnum.keyToValue(key.toLatin1().constData(), &ok);

      if (ok) {
            return static_cast<Sid>(val);
            }
      else {
            qWarning("Invalid style key: %s", qPrintable(key));
            return Sid::NOSTYLE;
            }
      }

//---------------------------------------------------------
//   MStyle::value
///   Returns a value of style setting named \p key.
///   Key should be one of \ref Sid values. Type of the
///   returned value depends on type of the corresponding
///   style setting.
//---------------------------------------------------------

QVariant MStyle::value(const QString& key) const {
      const Sid sid = keyToSid(key);

      if (sid == Sid::NOSTYLE)
            return QVariant();

      const QVariant val = _style->value(sid);

      if (!strcmp(Ms::MStyle::valueType(sid), "Ms::Spatium"))
            return val.value<Ms::Spatium>().val();

      return val;
      }

//---------------------------------------------------------
//   MStyle::setValue
///   Sets the value of style setting named \p key to \p value.
///   Key should be one of \ref Sid values.
//---------------------------------------------------------

void MStyle::setValue(const QString& key, QVariant value) {
      const Sid sid = keyToSid(key);

      if (sid == Sid::NOSTYLE)
            return;

      if (!strcmp(Ms::MStyle::valueType(sid), "Ms::Spatium"))
            value = QVariant::fromValue(Ms::Spatium(value.toReal()));

      if (_score) {
            // Style belongs to actual score: change style value in undoable way
            _score->undoChangeStyleVal(sid, value);
            }
      else {
            // Style is not bound to a score: change the value directly
            _style->set(sid, value);
            }
      }

} // namespace PluginAPI
} // namespace Ms
