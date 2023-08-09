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

#include "engraving/libmscore/utils.h"

using namespace mu::notation;
using namespace mu::uicomponents;
using namespace mu::framework;

namespace mu::notation {
static StringList collectExcerptLowerNames(const QList<IExcerptNotationPtr>& allExcerpts)
{
    StringList names;

    for (const IExcerptNotationPtr& excerpt : allExcerpts) {
        names << String::fromQString(excerpt->name()).toLower();
    }

    return names;
}
}

PartListModel::PartListModel(QObject* parent)
    : QAbstractListModel(parent), m_selectionModel(new ItemMultiSelectionModel(this))
{
    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged, this, &PartListModel::selectionChanged);
}

void PartListModel::load()
{
    TRACEFUNC;

    beginResetModel();

    IMasterNotationPtr masterNotation = this->masterNotation();
    if (!masterNotation) {
        endResetModel();
        return;
    }

    ExcerptNotationList excerpts = masterNotation->excerpts().val;
    ExcerptNotationList potentialExcerpts = masterNotation->potentialExcerpts();
    excerpts.insert(excerpts.end(), potentialExcerpts.begin(), potentialExcerpts.end());

    masterNotation->sortExcerpts(excerpts);

    for (IExcerptNotationPtr excerpt : excerpts) {
        m_excerpts.push_back(excerpt);
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
    case RoleIsInited:
        return excerpt->isInited();
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
        { RoleIsInited, "isInited" },
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
    TRACEFUNC;

    QString name = mu::engraving::formatUniqueExcerptName(mtrc("notation", "Part"), collectExcerptLowerNames(m_excerpts)).toQString();
    IExcerptNotationPtr newExcerpt = masterNotation()->createEmptyExcerpt(name);

    int index = m_excerpts.size();
    insertExcerpt(index, newExcerpt);
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

void PartListModel::resetPart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    if (!m_excerpts[partIndex]->isEmpty()) {
        std::string question = mu::trc("notation", "Are you sure you want to reset this part?");

        IInteractive::Button btn = interactive()->question("", question, {
            IInteractive::Button::Yes, IInteractive::Button::No
        }).standardButton();

        if (btn != IInteractive::Button::Yes) {
            return;
        }
    }

    doResetPart(partIndex);
}

void PartListModel::doResetPart(int partIndex)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    masterNotation()->resetExcerpt(m_excerpts[partIndex]);

    emit dataChanged(index(partIndex), index(partIndex));
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

QString PartListModel::validatePartTitle(int partIndex, const QString& title) const
{
    return QString::fromStdString(doValidatePartTitle(partIndex, title.simplified()).text());
}

mu::Ret PartListModel::doValidatePartTitle(int partIndex, const QString& title) const
{
    if (title.isEmpty()) {
        return false;
    }

    QString titleLower = title.toLower();

    for (int i = 0; i < m_excerpts.size(); ++i) {
        if (i == partIndex) {
            continue;
        }

        if (m_excerpts[i]->name().toLower() == titleLower) {
            return make_ret(Ret::Code::UnknownError, trc("notation", "Name already exists"));
        }
    }

    return true;
}

void PartListModel::setPartTitle(int partIndex, const QString& title)
{
    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    QString simplifiedTitle = title.simplified();

    IExcerptNotationPtr excerpt = m_excerpts[partIndex];
    if (excerpt->name() == simplifiedTitle) {
        return;
    }

    if (!doValidatePartTitle(partIndex, simplifiedTitle)) {
        return;
    }

    excerpt->setName(simplifiedTitle);
    notifyAboutNotationChanged(partIndex);
}

void PartListModel::notifyAboutNotationChanged(int index)
{
    QModelIndex modelIndex = this->index(index);
    emit dataChanged(modelIndex, modelIndex);
}

void PartListModel::copyPart(int partIndex)
{
    TRACEFUNC;

    if (!isExcerptIndexValid(partIndex)) {
        return;
    }

    IExcerptNotationPtr copy = m_excerpts[partIndex]->clone();
    String baseName = String::fromQString(copy->name()) + u" " + mtrc("notation", "(copy)");
    copy->setName(mu::engraving::formatUniqueExcerptName(baseName, collectExcerptLowerNames(m_excerpts)));

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
