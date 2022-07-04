/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "partlistmodel.h"

#include "log.h"
#include "translation.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

using namespace mu::notation;
using namespace mu::uicomponents;
using namespace mu::framework;

static QString defaultPartTitle()
{
    return mu::qtrc("notation", "Part");
}

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

    for (IExcerptNotationPtr excerpt : masterNotation->excerpts().val) {
        m_excerpts << excerpt;
    }

    for (IExcerptNotationPtr excerpt : masterNotation->potentialExcerpts()) {
        m_excerpts << excerpt;
    }

    endResetModel();
}

QVariant PartListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    IExcerptNotationPtr excerpt = m_excerpts[index.row()];

    switch (role) {
    case RoleTitle:
        return excerpt->name();
    case RoleIsSelected:
        return m_selectionModel->isSelected(index);
    case RoleIsCustom:
        return excerpt->isCustom();
    }

    return QVariant();
}

int PartListModel::rowCount(const QModelIndex&) const
{
    return m_excerpts.size();
}

QHash<int, QByteArray> PartListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleIsSelected, "isSelected" },
        { RoleIsCustom, "isCustom" }
    };

    return roles;
}

bool PartListModel::hasSelection() const
{
    return m_selectionModel->hasSelection();
}

void PartListModel::createNewPart()
{
    IExcerptNotationPtr excerpt = masterNotation()->createEmptyExcerpt();

    int index = m_excerpts.size();
    insertExcerpt(index, excerpt);
}

void PartListModel::selectPart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    QModelIndex modelIndex = index(partIndex);
    m_selectionModel->select(modelIndex);

    emit dataChanged(index(0), index(rowCount() - 1), { RoleIsSelected });
}

void PartListModel::removePart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    if (!m_excerpts[partIndex]->isEmpty()) {
        std::string question = mu::trc("notation", "Are you sure you want to delete this part?");

        IInteractive::Button btn = interactive()->question("", question, {
            IInteractive::Button::Yes, IInteractive::Button::No
        }).standardButton();

        if (btn != IInteractive::Button::Yes) {
            return;
        }
    }

    doRemovePart(partIndex);
}

void PartListModel::doRemovePart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    bool isCurrentNotation = context()->currentNotation() == m_excerpts[partIndex]->notation();

    beginRemoveRows(QModelIndex(), partIndex, partIndex);

    masterNotation()->removeExcerpts({ m_excerpts[partIndex] });
    m_excerpts.removeAt(partIndex);

    endRemoveRows();

    if (isCurrentNotation) {
        context()->setCurrentNotation(context()->currentMasterNotation()->notation());
    }
}

void PartListModel::setPartTitle(int partIndex, const QString& title)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    IExcerptNotationPtr excerpt = m_excerpts[partIndex];
    if (excerpt->name() == title) {
        return;
    }

    excerpt->setName(title);

    notifyAboutNotationChanged(partIndex);
}

void PartListModel::validatePartTitle(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    IExcerptNotationPtr excerpt = m_excerpts[partIndex];
    if (!excerpt->name().isEmpty()) {
        return;
    }

    excerpt->setName(defaultPartTitle());

    notifyAboutNotationChanged(partIndex);
}

void PartListModel::notifyAboutNotationChanged(int index)
{
    QModelIndex modelIndex = this->index(index);
    emit dataChanged(modelIndex, modelIndex);
}

void PartListModel::copyPart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    IExcerptNotationPtr copy = m_excerpts[partIndex]->clone();
    QString title = copy->name();
    title += qtrc("notation", " (copy)");

    copy->setName(title);

    insertExcerpt(partIndex + 1, copy);
}

void PartListModel::insertExcerpt(int destinationIndex, IExcerptNotationPtr excerpt)
{
    beginInsertRows(QModelIndex(), destinationIndex, destinationIndex);
    m_excerpts.insert(destinationIndex, excerpt);
    masterNotation()->addExcerpts({ excerpt });
    endInsertRows();

    emit partAdded(destinationIndex);
    selectPart(destinationIndex);
}

void PartListModel::openSelectedParts()
{
    QList<int> rows = m_selectionModel->selectedRows();
    std::sort(rows.begin(), rows.end());

    openNotations(rows);
}

void PartListModel::openAllParts()
{
    QList<int> rows;

    for (int i = 0; i < m_excerpts.size(); ++i) {
        rows << i;
    }

    openNotations(rows);
}

void PartListModel::openNotations(const QList<int>& rows) const
{
    if (rows.empty()) {
        return;
    }

    ExcerptNotationList excerpts;
    for (int index : rows) {
        excerpts.push_back(m_excerpts[index]);
    }

    masterNotation()->addExcerpts(excerpts);

    for (int index : rows) {
        masterNotation()->setExcerptIsOpen(m_excerpts[index]->notation(), true);
    }

    context()->setCurrentNotation(m_excerpts[rows.last()]->notation());
}

bool PartListModel::isExcerptIndexValid(int index) const
{
    return index >= 0 && index < m_excerpts.size();
}

IMasterNotationPtr PartListModel::masterNotation() const
{
    return context()->currentMasterNotation();
}
