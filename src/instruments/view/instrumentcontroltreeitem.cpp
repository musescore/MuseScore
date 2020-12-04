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
#include "instrumentcontroltreeitem.h"

#include "log.h"
#include "translation.h"

using namespace mu::instruments;
using namespace mu::notation;

InstrumentControlTreeItem::InstrumentControlTreeItem(INotationPartsPtr notationParts, QObject* parent)
    : AbstractInstrumentPanelTreeItem(InstrumentTreeItemType::ItemType::CONTROL_ADD_DOUBLE_INSTRUMENT, notationParts, parent)
{
}

void InstrumentControlTreeItem::appendNewItem()
{
    QString uri = QString("musescore://instruments/select?canSelectMultipleInstruments=false");
    RetVal<Val> result = interactive()->open(uri.toStdString());

    if (!result.ret) {
        LOGE() << result.ret.toString();
        return;
    }

    Instrument newInstrument = result.val.toQVariant().value<Instrument>();
    if (!newInstrument.isValid()) {
        return;
    }

    notationParts()->appendDoublingInstrument(newInstrument, m_partId);
}

QString InstrumentControlTreeItem::partId() const
{
    return m_partId;
}

void InstrumentControlTreeItem::setPartId(const QString& id)
{
    m_partId = id;
}
