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

import QtQuick 2.8
import QtQml.Models 2.2

QtObject {
    property DelegateModel delegateModel: null

    property var placeholder: null

    property bool active: placeholder !== null
    property int index: placeholder ? placeholder.index : -1

    function removePlaceholder() {
        if (placeholder) {
            const idx = placeholder.index;
            placeholder.group.remove(placeholder.index, 1);
            placeholder = null;
        }
    }

    function makePlaceholder(idx, data) {
        // ensure that no placeholder is present currently
        removePlaceholder();
        delegateModel.items.insert(idx, data);
        placeholder = {
            group: delegateModel.items,
            index: idx
        };
    }
}
