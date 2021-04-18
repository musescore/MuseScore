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

#include "partlistmodel.h"

#include "log.h"
#include "translation.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

using namespace mu::notation;
using namespace mu::uicomponents;
using namespace mu::framework;

PartListModel::PartListModel(QObject* parent)
    : QAbstractListModel(parent), m_selectionModel(new ItemMultiSelectionModel(this))
{
    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged, this, &PartListModel::selectionChanged);
}

void PartListModel::load()
{
    beginResetModel();

    IMasterNotationPtr masterNotation = this->masterNotation();
    if (!masterNotation) {
        endResetModel();
        return;
    }

    m_notations << masterNotation->notation();
    for (IExcerptNotationPtr excerpt : masterNotation->excerpts().val) {
        m_notations << excerpt->notation();
    }

    endResetModel();
}

QVariant PartListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    INotationPtr notation = m_notations[index.row()];

    switch (role) {
    case RoleTitle:
        return notation->metaInfo().title;
    case RoleIsSelected:
        return m_selectionModel->isSelected(index);
    case RoleIsMain:
        return isMainNotation(notation);
    case RoleVoicesVisibility:
        return voicesVisibility(notation);
    case RoleVoicesTitle:
        return formatVoicesTitle(notation);
    }

    return QVariant();
}

bool PartListModel::isMainNotation(INotationPtr notation) const
{
    return notation == masterNotation()->notation();
}

QString PartListModel::formatVoicesTitle(INotationPtr notation) const
{
    QVariantList voicesVisibility = this->voicesVisibility(notation);
    QStringList voices;

    for (int voiceIndex = 0; voiceIndex < voicesVisibility.size(); ++voiceIndex) {
        bool visible = voicesVisibility[voiceIndex].toBool();

        if (visible) {
            voices << QString::number(voiceIndex + 1);
        }
    }

    if (voices.isEmpty()) {
        return qtrc("notation", "None");
    } else if (voices.size() == voicesVisibility.size()) {
        return qtrc("notation", "All");
    }

    return voices.join(", ");
}

QVariantList PartListModel::voicesVisibility(INotationPtr notation) const
{
    QVariantList visibility;

    if (!notation) {
        return visibility;
    }

    for (int voiceIndex = 0; voiceIndex < VOICES; ++voiceIndex) {
        visibility << notation->parts()->voiceVisible(voiceIndex);
    }

    return visibility;
}

int PartListModel::rowCount(const QModelIndex&) const
{
    return m_notations.size();
}

QHash<int, QByteArray> PartListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleIsSelected, "isSelected" },
        { RoleIsMain, "isMain" },
        { RoleVoicesVisibility, "voicesVisibility" },
        { RoleVoicesTitle, "voicesTitle" }
    };

    return roles;
}

bool PartListModel::hasSelection() const
{
    return m_selectionModel->hasSelection();
}

bool PartListModel::isRemovingAvailable() const
{
    if (!m_selectionModel->hasSelection()) {
        return false;
    }

    for (int index : selectedRows()) {
        if (isMainNotation(m_notations[index])) {
            return false;
        }
    }

    return true;
}

void PartListModel::createNewPart()
{
    Meta meta;
    meta.title = qtrc("notation", "Part");

    INotationPtr notation = notationCreator()->newExcerptNotation()->notation();

    notation->setMetaInfo(meta);

    int index = m_notations.size();
    insertNotation(index, notation);
}

void PartListModel::selectPart(int partIndex)
{
    if (!isNotationIndexValid(partIndex)) {
        return;
    }

    QModelIndexList previousSelectedIndexes = m_selectionModel->selectedIndexes();
    QModelIndex modelIndex = index(partIndex);
    m_selectionModel->select(modelIndex);
    QModelIndexList newSelectedIndexes = m_selectionModel->selectedIndexes();

    QSet<QModelIndex> indexesToUpdate(previousSelectedIndexes.begin(), previousSelectedIndexes.end());
    indexesToUpdate = indexesToUpdate.unite(QSet<QModelIndex>(newSelectedIndexes.begin(), newSelectedIndexes.end()));
    indexesToUpdate << modelIndex;

    for (const QModelIndex& indexToUpdate : indexesToUpdate) {
        emit dataChanged(indexToUpdate, indexToUpdate);
    }
}

void PartListModel::removePart(int partIndex)
{
    if (!isNotationIndexValid(partIndex)) {
        return;
    }

    constexpr int partCount = 1;

    if (userAgreesToRemoveParts(partCount)) {
        doRemovePart(partIndex);
    }
}

void PartListModel::doRemovePart(int partIndex)
{
    if (!isNotationIndexValid(partIndex)) {
        return;
    }

    beginRemoveRows(QModelIndex(), partIndex, partIndex);
    m_notations[partIndex]->setOpened(false);
    m_notations.removeAt(partIndex);
    endRemoveRows();
}

void PartListModel::setPartTitle(int partIndex, const QString& title)
{
    if (!isNotationIndexValid(partIndex)) {
        return;
    }

    INotationPtr notation = m_notations[partIndex];
    Meta meta = notation->metaInfo();

    if (meta.title == title) {
        return;
    }

    meta.title = title;
    notation->setMetaInfo(meta);

    notifyAboutNotationChanged(partIndex);
}

void PartListModel::setVoiceVisible(int partIndex, int voiceIndex, bool visible)
{
    if (!isNotationIndexValid(partIndex) || !isVoiceIndexValid(voiceIndex)) {
        return;
    }

    INotationPtr notation = m_notations[partIndex];

    if (notation->parts()->voiceVisible(voiceIndex) == visible) {
        return;
    }

    notation->parts()->setVoiceVisible(voiceIndex, visible);
    notifyAboutNotationChanged(partIndex);
}

void PartListModel::notifyAboutNotationChanged(int index)
{
    QModelIndex modelIndex = this->index(index);
    emit dataChanged(modelIndex, modelIndex);
}

void PartListModel::copyPart(int partIndex)
{
    if (!isNotationIndexValid(partIndex)) {
        return;
    }

    INotationPtr copy = m_notations[partIndex]->clone();
    Meta meta = copy->metaInfo();
    meta.title += qtrc("notation", " (copy)");

    copy->setMetaInfo(meta);

    insertNotation(partIndex + 1, copy);
}

void PartListModel::insertNotation(int destinationIndex, INotationPtr notation)
{
    beginInsertRows(QModelIndex(), destinationIndex, destinationIndex);
    m_notations.insert(destinationIndex, notation);
    endInsertRows();

    emit partAdded(destinationIndex);
    selectPart(destinationIndex);
}

void PartListModel::removeSelectedParts()
{
    QList<int> rows = selectedRows();
    if (rows.empty()) {
        return;
    }

    if (!userAgreesToRemoveParts(rows.count())) {
        return;
    }

    QList<INotationPtr> notationsToRemove;

    for (int row : rows) {
        notationsToRemove.push_back(m_notations[row]);
    }

    for (INotationPtr notation : notationsToRemove) {
        int row = m_notations.indexOf(notation);
        doRemovePart(row);
    }

    m_selectionModel->clear();
}

bool PartListModel::userAgreesToRemoveParts(int partCount) const
{
    QString question = mu::qtrc("notation", "Are you sure you want to delete %1?")
                       .arg(partCount > 1 ? "these parts" : "this part");

    IInteractive::Button button = interactive()->question("", question.toStdString(), {
        IInteractive::Button::Yes, IInteractive::Button::No
    });

    return button == IInteractive::Button::Yes;
}

void PartListModel::openSelectedParts()
{
    QList<int> rows = selectedRows();
    if (rows.empty()) {
        return;
    }

    for (int row : rows) {
        m_notations[row]->setOpened(true);
    }

    m_currentNotation = m_notations[rows.last()];
}

QList<int> PartListModel::selectedRows() const
{
    QList<int> result;

    for (const QModelIndex& index: m_selectionModel->selectedIndexes()) {
        result << index.row();
    }

    return result;
}

void PartListModel::apply()
{
    ExcerptNotationList newExcerpts;

    for (INotationPtr notation : m_notations) {
        IExcerptNotationPtr excerpt = std::dynamic_pointer_cast<IExcerptNotation>(notation);

        if (excerpt) {
            newExcerpts.push_back(excerpt);
        }
    }

    masterNotation()->setExcerpts(newExcerpts);

    if (m_currentNotation) {
        context()->setCurrentNotation(m_currentNotation);
    }
}

bool PartListModel::isNotationIndexValid(int index) const
{
    return index >= 0 && index < m_notations.size();
}

IMasterNotationPtr PartListModel::masterNotation() const
{
    return context()->currentMasterNotation();
}
