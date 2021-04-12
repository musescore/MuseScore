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

#include <QItemSelectionModel>

using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::iex::musicxml;

static const QString DEFAULT_EXPORT_SUFFIX = "pdf";
static const ExportUnitType DEFAULT_EXPORT_UNITTYPE = ExportUnitType::PER_PART;

ExportDialogModel::ExportDialogModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_selectionModel(new QItemSelectionModel(this))
    , m_selectedExportSuffix(DEFAULT_EXPORT_SUFFIX)
    , m_selectedUnitType(DEFAULT_EXPORT_UNITTYPE)
{
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &ExportDialogModel::selectionChanged);
}

ExportDialogModel::~ExportDialogModel()
{
    m_selectionModel->deleteLater();
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

    selectCurrentNotation();

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
    if (!isIndexValid(scoreIndex)) {
        return;
    }

    QModelIndex modelIndex = index(scoreIndex);
    m_selectionModel->select(modelIndex, selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);

    emit dataChanged(modelIndex, modelIndex, { RoleIsSelected });
}

void ExportDialogModel::toggleSelected(int scoreIndex)
{
    if (!isIndexValid(scoreIndex)) {
        return;
    }

    QModelIndex modelIndex = index(scoreIndex);
    m_selectionModel->select(modelIndex, QItemSelectionModel::Toggle);

    emit dataChanged(modelIndex, modelIndex, { RoleIsSelected });
}

void ExportDialogModel::setAllSelected(bool selected)
{
    for (int i = 0; i < rowCount(); i++) {
        setSelected(i, selected);
    }
}

void ExportDialogModel::selectCurrentNotation()
{
    for (int i = 0; i < rowCount(); i++) {
        setSelected(i, m_notations[i] == context()->currentNotation());
    }
}

IMasterNotationPtr ExportDialogModel::masterNotation() const
{
    return context()->currentMasterNotation();
}

bool ExportDialogModel::isMainNotation(INotationPtr notation) const
{
    return masterNotation() && masterNotation()->notation() == notation;
}

bool ExportDialogModel::isIndexValid(int index) const
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

    auto unitTypes = exportScoreScenario()->supportedUnitTypes(suffix.toStdString());
    IF_ASSERT_FAILED(!unitTypes.empty()) {
        return;
    }

    setUnitType(unitTypes.front());
}

QList<QVariantMap> ExportDialogModel::availableUnitTypes() const
{
    QMap<ExportUnitType, QString> unitTypeNames {
        { ExportUnitType::PER_PAGE, qtrc("userscores", "Each page to a separate file") },
        { ExportUnitType::PER_PART, qtrc("userscores", "Each part to a separate file") },
        { ExportUnitType::MULTI_PART, qtrc("userscores", "All parts combined in one file") },
    };

    QList<QVariantMap> result;

    for (ExportUnitType type : exportScoreScenario()->supportedUnitTypes(m_selectedExportSuffix.toStdString())) {
        QVariantMap obj;
        obj["text"] = unitTypeNames[type];
        obj["value"] = static_cast<int>(type);
        result << obj;
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

    if (notations.empty()) {
        return false;
    }

    return exportScoreScenario()->exportScores(notations, m_selectedExportSuffix.toStdString(), m_selectedUnitType);
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

QList<QVariantMap> ExportDialogModel::musicXmlLayoutTypes() const
{
    QMap<MusicXmlLayoutType, QString> musicXmlLayoutTypeNames {
        { MusicXmlLayoutType::AllLayout, qtrc("userscores", "All layout") },
        { MusicXmlLayoutType::AllBreaks, qtrc("userscores", "System and page breaks") },
        { MusicXmlLayoutType::ManualBreaks, qtrc("userscores", "Manually added system and page breaks only") },
        { MusicXmlLayoutType::None, qtrc("userscores", "No system or page breaks") },
    };

    QList<QVariantMap> result;

    for (MusicXmlLayoutType type : musicXmlLayoutTypeNames.keys()) {
        QVariantMap obj;
        obj["text"] = musicXmlLayoutTypeNames[type];
        obj["value"] = static_cast<int>(type);
        result << obj;
    }

    return result;
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
