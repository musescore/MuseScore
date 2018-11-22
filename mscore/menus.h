//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer and others
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

#ifndef __MENUS_H__
#define __MENUS_H__

namespace Ms {

class IconAction;
class Palette;

extern QMap<QString, QStringList>* smuflRanges();
constexpr const char* SMUFL_ALL_SYMBOLS = "All symbols";
extern void populateIconPalette(Palette* p, const IconAction* a);

} // namespace Ms

#endif
