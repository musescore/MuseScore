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
#include "instrumentlistmodel.h"

#include "log.h"

using namespace mu::scene::instruments;

InstrumentListModel::InstrumentListModel(QObject* parent)
    : QObject(parent)
{
}

void InstrumentListModel::load()
{
    RetValCh<InstrumentsMeta> instrumentsMeta = repository()->instrumentsMeta();
    if (!instrumentsMeta.ret) {
        LOGE() << instrumentsMeta.ret.toString();
        return;
    }

    instrumentsMeta.ch.onReceive(this, [this](const InstrumentsMeta& newInstrumentsMeta) {
        setInstrumentsMeta(newInstrumentsMeta);
    });

    m_selectedFamilyId = "";
    m_inited = true;

    setInstrumentsMeta(instrumentsMeta.val);
}

QVariantList InstrumentListModel::families()
{
    if (!m_inited) {
        return QVariantList();
    }

    QVariantList result;
    for (const InstrumentGenre& genre: m_instrumentsMeta.genres) {
        QVariantMap obj;
        obj["id"] = genre.id;
        obj["name"] = genre.name;

        result << obj;
    }

    return result;
}

QVariantList InstrumentListModel::groups()
{
    if (!m_inited) {
        return QVariantList();
    }

    QVariantList result;

    if (m_selectedFamilyId.isEmpty()) {
        for (const InstrumentGroup& group: m_instrumentsMeta.groups) {
            QVariantMap obj;
            obj["id"] = group.id;
            obj["name"] = group.name;

            result << obj;
        }

        return result;
    }

    QStringList availableGroups;
    for (const InstrumentTemplate& instrument: m_instrumentsMeta.instrumentTemplates) {
        if (!instrument.genreIds.contains(m_selectedFamilyId)) {
            continue;
        }

        if (!availableGroups.contains(instrument.groupId)) {
            availableGroups << instrument.groupId;
        }
    }

    for (const QString& groupId: availableGroups) {
        InstrumentGroup group = m_instrumentsMeta.groups[groupId];

        QVariantMap obj;
        obj["id"] = group.id;
        obj["name"] = group.name;

        result << obj;
    }

    return result;
}

QVariantList InstrumentListModel::instruments()
{
    if (!m_inited) {
        return QVariantList();
    }

    QVariantList result;
    for (const InstrumentTemplate& instrument: m_instrumentsMeta.instrumentTemplates) {
        if (instrument.groupId != m_selectedGroupId || !instrument.genreIds.contains(m_selectedFamilyId)) {
            continue;
        }

        QVariantMap obj;
        obj["name"] = instrument.trackName;

        result << obj;
    }

    return result;
}

void InstrumentListModel::selectFamily(const QString& family)
{
    if (m_selectedFamilyId == family) {
        return;
    }

    m_selectedFamilyId = family;
    emit familyChanged();
}

void InstrumentListModel::selectGroup(const QString& group)
{
    if (m_selectedGroupId == group) {
        return;
    }

    m_selectedGroupId = group;
    emit groupChanged();
}

void InstrumentListModel::setInstrumentsMeta(const InstrumentsMeta& meta)
{
    m_instrumentsMeta = meta;
    emit familiesChanged();
    emit groupChanged();
}
