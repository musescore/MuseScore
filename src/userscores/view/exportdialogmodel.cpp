//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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

#include "exportdialogmodel.h"

#include "translation.h"
#include "uicomponents/view/itemmultiselectionmodel.h"

using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::uicomponents;
using namespace mu::iex::musicxml;

static const QString DEFAULT_EXPORT_SUFFIX = "pdf";

ExportDialogModel::ExportDialogModel(QObject* parent)
    : QAbstractListModel(parent), m_selectionModel(new ItemMultiSelectionModel(this)), m_selectedExportSuffix(DEFAULT_EXPORT_SUFFIX)
{
    connect(m_selectionModel, &ItemMultiSelectionModel::selectionChanged, this, &ExportDialogModel::selectionChanged);
}

void ExportDialogModel::load()
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

QVariant ExportDialogModel::data(const QModelIndex& index, int role) const
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

int ExportDialogModel::rowCount(const QModelIndex&) const
{
    return m_notations.size();
}

QHash<int, QByteArray> ExportDialogModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleIsSelected, "isSelected" },
        { RoleIsMain, "isMain" }
    };

    return roles;
}

void ExportDialogModel::setSelected(int scoreIndex, bool selected)
{
    if (!isNotationIndexValid(scoreIndex)) {
        return;
    }

    QModelIndex modelIndex = index(scoreIndex);
    m_selectionModel->QItemSelectionModel::select(modelIndex, selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);

    emit dataChanged(modelIndex, modelIndex);
}

void ExportDialogModel::toggleSelected(int scoreIndex)
{
    if (!isNotationIndexValid(scoreIndex)) {
        return;
    }

    QModelIndex modelIndex = index(scoreIndex);
    m_selectionModel->QItemSelectionModel::select(modelIndex, QItemSelectionModel::Toggle);

    emit dataChanged(modelIndex, modelIndex);
}

void ExportDialogModel::setAllSelected(bool selected)
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

void ExportDialogModel::selectCurrentNotation()
{
    for (int i = 0; i < m_notations.size(); i++) {
        setSelected(i, m_notations[i] == context()->currentNotation());
    }
}

IMasterNotationPtr ExportDialogModel::masterNotation() const
{
    return context()->currentMasterNotation();
}

bool ExportDialogModel::isMainNotation(INotationPtr notation) const
{
    return notation == masterNotation()->notation();
}

bool ExportDialogModel::isNotationIndexValid(int index) const
{
    return index >= 0 && index < m_notations.size();
}

int ExportDialogModel::selectionLength() const
{
    return m_selectionModel->selectedIndexes().size();
}

QString ExportDialogModel::selectedExportSuffix()
{
    return m_selectedExportSuffix;
}

void ExportDialogModel::setExportSuffix(QString suffix)
{
    if (m_selectedExportSuffix == suffix) {
        return;
    }

    m_selectedExportSuffix = suffix;
    emit selectedExportSuffixChanged(suffix);

    setUnitType(static_cast<int>(exportScoreService()->supportedUnitTypes(suffix.toStdString())[0]));
}

QList<int> ExportDialogModel::availableUnitTypes() const
{
    QList<int> result;
    for (notation::INotationWriter::UnitType t : exportScoreService()->supportedUnitTypes(m_selectedExportSuffix.toStdString())) {
        result << static_cast<int>(t);
    }
    return result;
}

int ExportDialogModel::selectedUnitType() const
{
    return static_cast<int>(m_selectedUnitType);
}

void ExportDialogModel::setUnitType(int unitType)
{
    setUnitType(static_cast<ExportUnitType>(unitType));
}

void ExportDialogModel::setUnitType(ExportUnitType unitType)
{
    if (m_selectedUnitType == unitType) {
        return;
    }

    m_selectedUnitType = unitType;
    emit selectedUnitTypeChanged(unitType);
}

bool ExportDialogModel::exportScores()
{
    INotationPtrList notations;

    for (const QModelIndex& index: m_selectionModel->selectedIndexes()) {
        notations.push_back(m_notations[index.row()]);
    }

    IF_ASSERT_FAILED(!notations.empty()) {
        return false;
    }

    /// If only one file will be created, the filename will be exactly what the user
    /// types in the save dialog and therefore we can put the file dialog in charge of
    /// asking the user whether an existing file should be overridden. Otherwise, we
    /// will take care of that ourselves.
    bool willCreateOneFile = exportScoreService()->willCreateOnlyOneFile(
        static_cast<notation::INotationWriter::UnitType>(m_selectedUnitType), notations);
    bool willExportOnlyOneScore = notations.size() == 1;

    io::path suggestedPath = configuration()->scoresPath().val;
    io::path masterNotationDirPath = io::dirpath(masterNotation()->path());
    if (masterNotationDirPath != "") {
        suggestedPath = masterNotationDirPath;
    }

    suggestedPath += "/" + masterNotation()->metaInfo().title;

    if (m_selectedUnitType == ExportUnitType::MULTI_PART && !willExportOnlyOneScore) {
        bool containsMaster = false;
        for (auto notation : notations) {
            if (isMainNotation(notation)) {
                containsMaster = true;
                break;
            }
        }

        if (containsMaster) {
            suggestedPath += "-" + qtrc("userscores", "Score_and_Parts", "Used in export filename suggestion");
        } else {
            suggestedPath += "-" + qtrc("userscores", "Parts", "Used in export filename suggestion");
        }
    } else if (willExportOnlyOneScore) {
        if (!isMainNotation(notations.front())) {
            suggestedPath += "-" + io::escapeFileName(notations.front()->metaInfo().title);
        }

        if (willCreateOneFile && m_selectedUnitType == ExportUnitType::PER_PAGE) {
            // So there is only one page
            suggestedPath += "-1";
        }
    }

    suggestedPath += "." + m_selectedExportSuffix;

    io::path exportPath = interactive()->selectSavingFile(qtrc("userscores", "Export"),
                                                          suggestedPath, exportFilter(), willCreateOneFile);
    if (exportPath.empty()) {
        return false;
    }

    exportScoreService()->exportScores(notations, exportPath, m_selectedUnitType);
    return true; // TODO: return false on error/cancel
}

int ExportDialogModel::pdfResolution() const
{
    return imageExportConfiguration()->exportPdfDpiResolution();
}

void ExportDialogModel::setPdfResolution(const int& resolution)
{
    if (resolution == pdfResolution()) {
        return;
    }

    imageExportConfiguration()->setExportPdfDpiResolution(resolution);
    emit pdfResolutionChanged(resolution);
}

int ExportDialogModel::pngResolution() const
{
    return imageExportConfiguration()->exportPngDpiResolution();
}

void ExportDialogModel::setPngResolution(const int& resolution)
{
    if (resolution == pngResolution()) {
        return;
    }

    imageExportConfiguration()->setExportPngDpiResolution(resolution);
    emit pngResolutionChanged(resolution);
}

bool ExportDialogModel::pngTransparentBackground() const
{
    return imageExportConfiguration()->exportPngWithTransparentBackground();
}

void ExportDialogModel::setPngTransparentBackground(const bool& transparent)
{
    if (transparent == pngTransparentBackground()) {
        return;
    }

    imageExportConfiguration()->setExportPngWithTransparentBackground(transparent);
    emit pngTransparentBackgroundChanged(transparent);
}

bool ExportDialogModel::normalizeAudio() const
{
    NOT_IMPLEMENTED;
    return true;
}

void ExportDialogModel::setNormalizeAudio(bool normalizeAudio)
{
    if (normalizeAudio == this->normalizeAudio()) {
        return;
    }

    NOT_IMPLEMENTED;
    emit normalizeAudioChanged(normalizeAudio);
}

QList<int> ExportDialogModel::availableSampleRates() const
{
    NOT_IMPLEMENTED; // TODO: move to audio configuration
    return { 32000, 44100, 48000 };
}

int ExportDialogModel::sampleRate() const
{
    NOT_IMPLEMENTED;
    return 44100;
}

void ExportDialogModel::setSampleRate(int sampleRate)
{
    if (sampleRate == this->sampleRate()) {
        return;
    }

    NOT_IMPLEMENTED;
    emit sampleRateChanged(sampleRate);
}

QList<int> ExportDialogModel::availableBitRates() const
{
    NOT_IMPLEMENTED; // TODO: move to audio configuration
    return { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, };
}

int ExportDialogModel::bitRate() const
{
    NOT_IMPLEMENTED;
    return 192;
}

void ExportDialogModel::setBitRate(int bitRate)
{
    if (bitRate == this->bitRate()) {
        return;
    }

    NOT_IMPLEMENTED;
    emit bitRateChanged(bitRate);
}

bool ExportDialogModel::midiExpandRepeats() const
{
    NOT_IMPLEMENTED;
    return true;
}

void ExportDialogModel::setMidiExpandRepeats(bool expandRepeats)
{
    if (expandRepeats == midiExpandRepeats()) {
        return;
    }

    NOT_IMPLEMENTED;
    emit midiExpandRepeatsChanged(expandRepeats);
}

bool ExportDialogModel::midiExportRpns() const
{
    NOT_IMPLEMENTED;
    return true;
}

void ExportDialogModel::setMidiExportRpns(bool exportRpns)
{
    if (exportRpns == midiExportRpns()) {
        return;
    }

    NOT_IMPLEMENTED;
    emit midiExportRpnsChanged(exportRpns);
}

ExportDialogModel::MusicXmlLayoutType ExportDialogModel::musicXmlLayoutType() const
{
    if (musicXmlConfiguration()->musicxmlExportLayout()) {
        return AllLayout;
    }
    switch (musicXmlConfiguration()->musicxmlExportBreaksType()) {
    case IMusicXmlConfiguration::MusicxmlExportBreaksType::All:
        return AllBreaks;
    case IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual:
        return ManualBreaks;
    case IMusicXmlConfiguration::MusicxmlExportBreaksType::No:
        return None;
    }
}

void ExportDialogModel::setMusicXmlLayoutType(MusicXmlLayoutType layoutType)
{
    if (layoutType == musicXmlLayoutType()) {
        return;
    }
    switch (layoutType) {
    case AllLayout:
        musicXmlConfiguration()->setMusicxmlExportLayout(true);
        break;
    case AllBreaks:
        musicXmlConfiguration()->setMusicxmlExportLayout(false);
        musicXmlConfiguration()->setMusicxmlExportBreaksType(IMusicXmlConfiguration::MusicxmlExportBreaksType::All);
        break;
    case ManualBreaks:
        musicXmlConfiguration()->setMusicxmlExportLayout(false);
        musicXmlConfiguration()->setMusicxmlExportBreaksType(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual);
        break;
    case None:
        musicXmlConfiguration()->setMusicxmlExportLayout(false);
        musicXmlConfiguration()->setMusicxmlExportBreaksType(IMusicXmlConfiguration::MusicxmlExportBreaksType::No);
        break;
    }
    emit musicXmlLayoutTypeChanged(layoutType);
}

QString ExportDialogModel::exportFilter() const
{
    QString filter;

    if (m_selectedExportSuffix == "pdf") {
        filter = qtrc("userscores", "PDF Files") + " (*.pdf)";
    } else if (m_selectedExportSuffix == "png") {
        filter = qtrc("userscores", "PNG Images") + " (*.png)";
    } else if (m_selectedExportSuffix == "svg") {
        filter = qtrc("userscores", "SVG Images") + " (*.svg)";
    } else if (m_selectedExportSuffix == "mp3") {
        filter = qtrc("userscores", "MP3 Audio Files") + " (*.mp3)";
    } else if (m_selectedExportSuffix == "wav") {
        filter = qtrc("userscores", "WAV Audio Files") + " (*.wav)";
    } else if (m_selectedExportSuffix == "ogg") {
        filter = qtrc("userscores", "OGG Audio Files") + " (*.ogg)";
    } else if (m_selectedExportSuffix == "flac") {
        filter = qtrc("userscores", "FLAC Audio Files") + " (*.flac)";
    } else if (m_selectedExportSuffix == "mid" || m_selectedExportSuffix == "midi" || m_selectedExportSuffix == "kar") {
        filter = qtrc("userscores", "MIDI Files") + " (*.mid *.midi *.kar)";
    } else if (m_selectedExportSuffix == "mxl") {
        filter = qtrc("userscores", "Compressed MusicXML Files") + " (*.mxl)";
    } else if (m_selectedExportSuffix == "musicxml" || m_selectedExportSuffix == "xml") {
        filter = qtrc("userscores", "Uncompressed MusicXML Files") + " (*.musicxml *.xml)";
    }

    return filter;
}
