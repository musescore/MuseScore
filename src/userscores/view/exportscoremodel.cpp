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

#include "translation.h"
#include "uicomponents/view/itemmultiselectionmodel.h"

using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::uicomponents;

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

void ExportScoreModel::setSelected(int scoreIndex, bool selected)
{
    if (!isNotationIndexValid(scoreIndex)) {
        return;
    }

    QModelIndex modelIndex = index(scoreIndex);
    m_selectionModel->QItemSelectionModel::select(modelIndex, selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);

    emit dataChanged(modelIndex, modelIndex);
}

void ExportScoreModel::toggleSelected(int scoreIndex)
{
    if (!isNotationIndexValid(scoreIndex)) {
        return;
    }

    QModelIndex modelIndex = index(scoreIndex);
    m_selectionModel->QItemSelectionModel::select(modelIndex, QItemSelectionModel::Toggle);

    emit dataChanged(modelIndex, modelIndex);
}

void ExportScoreModel::setAllSelected(bool selected)
{
    QModelIndexList previousSelectedIndexes = m_selectionModel->selectedIndexes();
    m_selectionModel->QItemSelectionModel::select(QItemSelection(index(0), index(m_notations.size() - 1)),
                                                  selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
    QModelIndexList newSelectedIndexes = m_selectionModel->selectedIndexes();

    QSet<QModelIndex> indexesToUpdate(previousSelectedIndexes.begin(), previousSelectedIndexes.end());
    indexesToUpdate = indexesToUpdate.unite(QSet<QModelIndex>(newSelectedIndexes.begin(), newSelectedIndexes.end()));

    for (const QModelIndex& indexToUpdate : indexesToUpdate) {
        emit dataChanged(indexToUpdate, indexToUpdate);
    }
}

void ExportScoreModel::selectCurrentNotation()
{
    for (int i = 0; i < m_notations.size(); i++) {
        setSelected(i, m_notations[i] == context()->currentNotation());
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

int ExportScoreModel::selectionLength() const
{
    return m_selectionModel->selectedIndexes().size();
}

QString ExportScoreModel::exportPath() const
{
    return m_exportPath.toQString();
}

void ExportScoreModel::setExportPath(QString path)
{
    m_exportPath = io::path(path);
}

QString ExportScoreModel::chooseExportPath()
{
    io::path requestedPath = interactive()->selectSavingFile(qtrc("userscores", "Export Score"), m_exportPath, exportFliter());

    if (!requestedPath.empty()) {
        m_exportPath = requestedPath;
    }

    return m_exportPath.toQString();
}

void ExportScoreModel::setExportSuffix(QString suffix)
{
    m_exportPath = dirpath(m_exportPath) + "/" + basename(m_exportPath) + "." + suffix;
}

bool ExportScoreModel::exportScores()
{
    INotationPtrList notations;

    for (int i : selectedRows()) {
        notations.push_back(m_notations[i]);
    }

    IF_ASSERT_FAILED(!notations.empty()) {
        return false;
    }

    exportScoreService()->exportScores(notations, m_exportPath);
    return true; // TODO: return false on error/cancel
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

QString ExportScoreModel::exportFliter() const
{
    io::path suffix = io::syffix(m_exportPath);

    QString filter;

    if (suffix == "pdf") {
        filter = qtrc("userscores", "PDF Files") + " (*.pdf)";
    } else if (suffix == "png") {
        filter = qtrc("userscores", "PNG Images") + " (*.png)";
    } else if (suffix == "svg") {
        filter = qtrc("userscores", "SVG Images") + " (*.svg)";
    } else if (suffix == "mp3") {
        filter = qtrc("userscores", "MP3 Audio Files") + " (*.mp3)";
    } else if (suffix == "wav") {
        filter = qtrc("userscores", "WAV Audio Files") + " (*.wav)";
    } else if (suffix == "ogg") {
        filter = qtrc("userscores", "OGG Audio Files") + " (*.ogg)";
    } else if (suffix == "flac") {
        filter = qtrc("userscores", "FLAC Audio Files") + " (*.flac)";
    } else if (suffix == "mid" || suffix == "midi" || suffix == "kar") {
        filter = qtrc("userscores", "MIDI Files") + " (*.mid, *.midi, *.kar)";
    } else if (suffix == "mxl") {
        filter = qtrc("userscores", "Compressed MusicXML Files") + " (*.mxl)";
    } else if (suffix == "musicxml" || suffix == "xml") {
        filter = qtrc("userscores", "Uncompressed MusicXML Files") + " (*.musicxml, *.xml)";
    }

    return filter;
}
