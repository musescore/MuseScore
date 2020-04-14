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

var style = {
    popupMargin: 3
}

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

function removeSelectedItems(paletteController, selectionModel, parentIndex) {
    paletteController.removeSelection(selectionModel.selectedIndexes, parentIndex);
}

function setInvisibleRecursive(item) {
    var children = item.children; // list of children if item is Item
    if (!children)
        children = item.contentChildren; // list of children if item is Popup

    for (var i = 0; i < children.length; ++i)
        setInvisibleRecursive(children[i]);

    item.visible = false;
}
