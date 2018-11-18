//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  Copyright (C) 2002-2018 Werner Schweer and others
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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QObject>

namespace Ms {

class stringutils : public QObject
{
      Q_OBJECT

   public:
      static QString removeLigatures(const QString& pre);
      static QString removeDiacritics(const QString& pre);
      static QString convertFileSizeToHumanReadable(const qlonglong & bytes);
};

} // namespace Ms

#endif // STRINGUTILS_H
