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

#include "exportscoremodel.h"

#include "log.h"
#include "translation.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

using namespace mu;
using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::uicomponents;
using namespace mu::framework;

ExportScoreModel::ExportScoreModel(QObject* parent)
    : QAbstractListModel(parent), m_selectionModel(new ItemMultiSelectionModel(this))
    , m_exportPath(configuration()->defaultExportPath(masterNotation()->metaInfo().title.toStdString()))
{
    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged, this, &ExportScoreModel::selectionChanged);
}

void ExportScoreModel::load()
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

QVariant ExportScoreModel::data(const QModelIndex& index, int role) const
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
    }

    return QVariant();
}

int ExportScoreModel::rowCount(const QModelIndex&) const
{
    return m_notations.size();
}

QHash<int, QByteArray> ExportScoreModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleIsSelected, "isSelected" },
        { RoleIsMain, "isMain" }
    };

    return roles;
}

void ExportScoreModel::toggleSelection(int scoreIndex)
{
    if (!isNotationIndexValid(scoreIndex)) {
        return;
    }

    QModelIndex modelIndex = index(scoreIndex);
    m_selectionModel->QItemSelectionModel::select(modelIndex, QItemSelectionModel::Toggle);

    emit dataChanged(modelIndex, modelIndex);
}

void ExportScoreModel::toggleAllSelections(bool select)
{
    QModelIndexList previousSelectedIndexes = m_selectionModel->selectedIndexes();
    m_selectionModel->QItemSelectionModel::select(QItemSelection(index(0), index(m_notations.size() - 1)),
                                                  select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
    QModelIndexList newSelectedIndexes = m_selectionModel->selectedIndexes();

    QSet<QModelIndex> indexesToUpdate(previousSelectedIndexes.begin(), previousSelectedIndexes.end());
    indexesToUpdate = indexesToUpdate.unite(QSet<QModelIndex>(newSelectedIndexes.begin(), newSelectedIndexes.end()));

    for (const QModelIndex& indexToUpdate : indexesToUpdate) {
        emit dataChanged(indexToUpdate, indexToUpdate);
    }
}

IMasterNotationPtr ExportScoreModel::masterNotation() const
{
    return context()->currentMasterNotation();
}

bool ExportScoreModel::isMainNotation(INotationPtr notation) const
{
    return notation == masterNotation()->notation();
}

bool ExportScoreModel::isNotationIndexValid(int index) const
{
    return index >= 0 && index < m_notations.size();
}

QList<int> ExportScoreModel::selectedRows() const
{
    QList<int> result;

    for (const QModelIndex& index: m_selectionModel->selectedIndexes()) {
        result << index.row();
    }

    return result;
}

int ExportScoreModel::getSelectionLength() const
{
    return selectedRows().size();
}

QString ExportScoreModel::getExportPath() const
{
    return m_exportPath.toQString();
}

void ExportScoreModel::setExportPath(QString path)
{
    m_exportPath = io::path(path);
}

QString ExportScoreModel::chooseExportPath()
{
    io::path requestedPath = interactive()->selectSavingFile(qtrc("userscores", "Export Score"), m_exportPath, getExportFliter());

    if (!requestedPath.empty()) {
        m_exportPath = requestedPath;
    }

    return m_exportPath.toQString();
}

void ExportScoreModel::setExportSuffix(QString suffix)
{
    m_exportPath = dirpath(m_exportPath) + "/" + basename(m_exportPath) + "." + suffix;
}

void ExportScoreModel::exportScores()
{
    QList<INotationPtr> notations;

    for (int i : selectedRows()) {
        notations << m_notations[i];
    }

    exportScoreService()->exportScores(notations, m_exportPath);
}

int ExportScoreModel::pdfResolution() const
{
    return imageExportConfiguration()->exportPdfDpiResolution();
}

void ExportScoreModel::setPdfResolution(const int& resolution)
{
    imageExportConfiguration()->setExportPdfDpiResolution(resolution);
}

int ExportScoreModel::pngResolution() const
{
    return imageExportConfiguration()->exportPngDpiResolution();
}

void ExportScoreModel::setPngResolution(const int& resolution)
{
    imageExportConfiguration()->setExportPngDpiResolution(resolution);
}

bool ExportScoreModel::pngTransparentBackground() const
{
    return imageExportConfiguration()->exportPngWithTransparentBackground();
}

void ExportScoreModel::setPngTransparentBackground(const bool& transparent)
{
    imageExportConfiguration()->setExportPngWithTransparentBackground(transparent);
}

QString ExportScoreModel::getExportFliter() const
{
    io::path suffix = io::syffix(m_exportPath);

    QString filter;

    if (suffix == "pdf") {
        filter = QObject::tr("PDF files") + " (*.pdf)";
    } else if (suffix == "png") {
        filter = QObject::tr("PNG images") + " (*.png)";
    } else if (suffix == "svg") {
        filter = QObject::tr("SVG images") + " (*.svg)";
    } else if (suffix == "mp3") {
        filter = QObject::tr("mp3 audio") + " (*.mp3)";
    } else if (suffix == "midi") {
        filter = QObject::tr("midi file") + " (*.mid, *.midi)";
    }

    return filter;
}
