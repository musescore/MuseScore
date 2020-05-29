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

#ifndef __PLUGIN_API_STYLE_H__
#define __PLUGIN_API_STYLE_H__

#include "libmscore/style.h"

namespace Ms {
class Score;

namespace PluginAPI {
//---------------------------------------------------------
//   MStyle
///   Provides an access to score style settings.
///   Style settings for a score can be obtained by
///   querying the \ref Score.style property.
///
///   Usage example:
///   \code
///   var style = curScore.style;
///   var genClef = style.value("genClef"); // retrieves style setting on clefs generation, true by default
///   style.setValue("genClef", false); // disables generating clefs for this score
///   \endcode
///   \since MuseScore 3.5
///   \see \ref Sid
//---------------------------------------------------------

class MStyle : public QObject
{
    Q_OBJECT

    Ms::MStyle* _style;
    Ms::Score* _score;

    static Sid keyToSid(const QString& key);

public:
    /// \cond MS_INTERNAL
    MStyle(Ms::MStyle* style, Ms::Score* score)
        : QObject(), _style(style), _score(score) {}
    /// \endcond

    Q_INVOKABLE QVariant value(const QString& key) const;
    Q_INVOKABLE void setValue(const QString& key, QVariant value);
};

extern MStyle* wrap(Ms::MStyle*, Ms::Score*);
} // namespace PluginAPI
} // namespace Ms

#endif
