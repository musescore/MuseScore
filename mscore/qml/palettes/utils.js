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

.pragma library

function stretched(cw, w) {
    return cw + (w % cw) / Math.floor(w / cw);
}

function dropEventMimeData(drag) {
    var formats = drag.formats;
    var mime = {};
    for (var i = 0; i < formats.length; i++) {
        var fmt = formats[i];
        mime[fmt] = drag.getDataAsArrayBuffer(fmt);
    }
    return mime;
}

function removeSelectedItems(paletteModel, paletteController, selectionModel, parentIndex) {
    function findNextSelectedIndex(selectedList) {
        for (var i = 0; i < selectedList.length; i++) {
            var modelIndex = selectedList[i];
            var selParentIdx = paletteModel.parent(modelIndex);
            if ((parentIndex || selParentIdx.valid) && selParentIdx != parentIndex)
                continue;
            return modelIndex;
        }
        return null;
    }

    var selectedIndex = findNextSelectedIndex(selectionModel.selectedIndexes);
    while (selectedIndex) {
        var ok = paletteController.remove(selectedIndex);
        if (!ok)
            return;
        selectedIndex = findNextSelectedIndex(selectionModel.selectedIndexes);
    }
}
