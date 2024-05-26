/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "exportdialogmodel.h"

#include <QItemSelectionModel>

#include "async/async.h"
#include "translation.h"
#include "log.h"

using namespace muse;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::iex::musicxml;

using UnitType = INotationWriter::UnitType;

static const UnitType DEFAULT_EXPORT_UNITTYPE = UnitType::PER_PART;

ExportDialogModel::ExportDialogModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_selectionModel(new QItemSelectionModel(this))
    , m_selectedUnitType(DEFAULT_EXPORT_UNITTYPE)
{
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &ExportDialogModel::selectionChanged);

    ExportTypeList musicXmlTypes {
        ExportType::makeWithSuffixes({ "mxl" },
                                     muse::qtrc("project/export", "Compressed") + " (*.mxl)",
                                     muse::qtrc("project/export", "Compressed MusicXML files"),
                                     "MusicXmlSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "musicxml" },
                                     muse::qtrc("project/export", "Uncompressed") + " (*.musicxml)",
                                     muse::qtrc("project/export", "Uncompressed MusicXML files"),
                                     "MusicXmlSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "xml" },
                                     muse::qtrc("project/export", "Uncompressed (outdated)") + " (*.xml)",
                                     muse::qtrc("project/export", "Uncompressed MusicXML files"),
                                     "MusicXmlSettingsPage.qml"),
    };

    m_exportTypeList = {
        ExportType::makeWithSuffixes({ "pdf" },
                                     muse::qtrc("project/export", "PDF file"),
                                     muse::qtrc("project/export", "PDF files"),
                                     "PdfSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "png" },
                                     muse::qtrc("project/export", "PNG images"),
                                     muse::qtrc("project/export", "PNG images"),
                                     "PngSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "svg" },
                                     muse::qtrc("project/export", "SVG images"),
                                     muse::qtrc("project/export", "SVG images"),
                                     "SvgSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "mp3" },
                                     muse::qtrc("project/export", "MP3 audio"),
                                     muse::qtrc("project/export", "MP3 audio files"),
                                     "Mp3SettingsPage.qml"),
        ExportType::makeWithSuffixes({ "wav" },
                                     muse::qtrc("project/export", "WAV audio"),
                                     muse::qtrc("project/export", "WAV audio files"),
                                     "AudioSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "ogg" },
                                     muse::qtrc("project/export", "OGG audio"),
                                     muse::qtrc("project/export", "OGG audio files"),
                                     "OggSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "flac" },
                                     muse::qtrc("project/export", "FLAC audio"),
                                     muse::qtrc("project/export", "FLAC audio files"),
                                     "AudioSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "mid", "midi", "kar" },
                                     muse::qtrc("project/export", "MIDI file"),
                                     muse::qtrc("project/export", "MIDI files"),
                                     "MidiSettingsPage.qml"),
        ExportType::makeWithSubtypes(musicXmlTypes,
                                     muse::qtrc("project/export", "MusicXML")),
        ExportType::makeWithSuffixes({ "brf" },
                                     //: Meaning like "measure-over-measure", but called "bar-over-bar"
                                     //: even in US English. Not to be confused with "bar-by-bar" format.
                                     //: See https://musescore.org/en/handbook/4/file-export#Score_formats
                                     muse::qtrc("project/export", "Braille (basic bar-over-bar)"),
                                     muse::qtrc("project/export", "Braille files")),
        ExportType::makeWithSuffixes({ "mei" },
                                     muse::qtrc("project/export", "MEI"),
                                     muse::qtrc("project/export", "MEI files"),
                                     "MeiSettingsPage.qml")
    };

    ExportInfo info = exportProjectScenario()->exportInfo();
    if (info.id == "") {
        setExportType(m_exportTypeList.front());
    } else {
        selectExportTypeById(info.id);
    }
    m_exportPath = info.exportPath;
    setUnitType(info.unitType);
}

ExportDialogModel::~ExportDialogModel()
{
    m_selectionModel->deleteLater();
}

void ExportDialogModel::load()
{
    TRACEFUNC;

    beginResetModel();

    IMasterNotationPtr masterNotation = this->masterNotation();
    if (!masterNotation) {
        endResetModel();
        return;
    }

    m_notations << masterNotation->notation();

    ExcerptNotationList excerpts = masterNotation->excerpts();
    ExcerptNotationList potentialExcerpts = masterNotation->potentialExcerpts();
    excerpts.insert(excerpts.end(), potentialExcerpts.begin(), potentialExcerpts.end());

    masterNotation->sortExcerpts(excerpts);

    for (IExcerptNotationPtr excerpt : excerpts) {
        m_notations << excerpt->notation();
    }

    endResetModel();

    selectCurrentNotation();
    selectSavedNotations();
}

QVariant ExportDialogModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    INotationPtr notation = m_notations[index.row()];

    switch (role) {
    case RoleTitle:
        return notation->name();
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

void ExportDialogModel::selectSavedNotations()
{
    ExportInfo info = exportProjectScenario()->exportInfo();
    for (INotationPtr notation : info.notations) {
        auto it = std::find(m_notations.begin(), m_notations.end(), notation);
        if (it != m_notations.end()) {
            setSelected(std::distance(m_notations.begin(), it), true);
        }
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

QVariantList ExportDialogModel::exportTypeList() const
{
    return m_exportTypeList.toVariantList();
}

QVariantMap ExportDialogModel::selectedExportType() const
{
    return m_selectedExportType.toMap();
}

void ExportDialogModel::setExportType(const ExportType& type)
{
    if (m_selectedExportType == type) {
        return;
    }

    m_selectedExportType = type;

    emit selectedExportTypeChanged(type.toMap());

    std::vector<UnitType> unitTypes = exportProjectScenario()->supportedUnitTypes(type);

    IF_ASSERT_FAILED(!unitTypes.empty()) {
        return;
    }

    if (std::find(unitTypes.cbegin(), unitTypes.cend(), m_selectedUnitType) != unitTypes.cend()) {
        return;
    }

    //! NOTE if the writer for the newly selected type doesn't support the currently
    //! selected unit type, select the first supported unit type
    setUnitType(unitTypes.front());
}

void ExportDialogModel::selectExportTypeById(const QString& id)
{
    for (const ExportType& type : m_exportTypeList) {
        // First, check if it's a subtype
        if (type.subtypes.contains(id)) {
            setExportType(type.subtypes.getById(id));
            return;
        }

        if (type.id == id) {
            setExportType(type);
            return;
        }
    }

    LOGW() << "Export type id not found: " << id;
    setExportType(m_exportTypeList.front());
}

QVariantList ExportDialogModel::availableUnitTypes() const
{
    QMap<UnitType, QString> unitTypeNames {
        { UnitType::PER_PAGE, muse::qtrc("project/export", "Each page to a separate file") },
        { UnitType::PER_PART, muse::qtrc("project/export", "Each part to a separate file") },
        { UnitType::MULTI_PART, muse::qtrc("project/export", "All parts combined in one file") },
    };

    QVariantList result;

    for (UnitType type : exportProjectScenario()->supportedUnitTypes(m_selectedExportType)) {
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
    setUnitType(static_cast<UnitType>(unitType));
}

void ExportDialogModel::setUnitType(UnitType unitType)
{
    if (m_selectedUnitType == unitType) {
        return;
    }

    bool found = false;
    for (QVariant availabeUnitType : availableUnitTypes()) {
        if (availabeUnitType.value<QVariantMap>()["value"].toInt() == static_cast<int>(unitType)) {
            found = true;
        }
    }

    if (!found) {
        return;
    }

    m_selectedUnitType = unitType;
    emit selectedUnitTypeChanged(unitType);
}

bool ExportDialogModel::exportScores()
{
    INotationPtrList notations;

    for (const QModelIndex& index : m_selectionModel->selectedIndexes()) {
        notations.push_back(m_notations[index.row()]);
    }

    if (notations.empty()) {
        return false;
    }

    INotationProjectPtr project = context()->currentProject();
    muse::io::path_t filename = io::filename(project->path());
    // TODO: only restore filename if exactly the same notations are selected and the same unit type.
    // These settings may namely cause extra things to be added to the name, but these extra things
    // are not applicable to other values of these settings.
    //if (project && project->path() == exportProjectScenario()->exportInfo().projectPath) {
    //    filename = io::filename(m_exportPath);
    //}
    muse::io::path_t defaultPath;
    if (m_exportPath != "" && filename != "") {
        defaultPath = io::absoluteDirpath(m_exportPath)
                      .appendingComponent(io::filename(filename, false))
                      .appendingSuffix(m_selectedExportType.suffixes[0]);
    }

    RetVal<muse::io::path_t> exportPath
        = exportProjectScenario()->askExportPath(notations, m_selectedExportType, m_selectedUnitType, defaultPath);
    if (!exportPath.ret) {
        return false;
    }

    m_exportPath = exportPath.val;

    async::Async::call(this, [this, notations]() {
        exportProjectScenario()->exportScores(notations, m_exportPath, m_selectedUnitType,
                                              shouldDestinationFolderBeOpenedOnExport());
    });

    return true;
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

bool ExportDialogModel::pdfTransparentBackground() const
{
    return imageExportConfiguration()->exportPdfWithTransparentBackground();
}

void ExportDialogModel::setPdfTransparentBackground(const bool& transparent)
{
    if (transparent == pdfTransparentBackground()) {
        return;
    }

    imageExportConfiguration()->setExportPdfWithTransparentBackground(transparent);
    emit pdfTransparentBackgroundChanged(transparent);
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

bool ExportDialogModel::svgTransparentBackground() const
{
    return imageExportConfiguration()->exportSvgWithTransparentBackground();
}

void ExportDialogModel::setSvgTransparentBackground(const bool& transparent)
{
    if (transparent == svgTransparentBackground()) {
        return;
    }

    imageExportConfiguration()->setExportSvgWithTransparentBackground(transparent);
    emit svgTransparentBackgroundChanged(transparent);
}

QList<int> ExportDialogModel::availableSampleRates() const
{
    const std::vector<int>& rates = audioExportConfiguration()->availableSampleRates();
    return QList<int>(rates.cbegin(), rates.cend());
}

int ExportDialogModel::sampleRate() const
{
    return audioExportConfiguration()->exportSampleRate();
}

void ExportDialogModel::setSampleRate(int rate)
{
    if (rate == sampleRate()) {
        return;
    }

    audioExportConfiguration()->setExportSampleRate(rate);
    emit sampleRateChanged(rate);
}

QList<int> ExportDialogModel::availableBitRates() const
{
    const std::vector<int>& rates = audioExportConfiguration()->availableMp3BitRates();
    return QList<int>(rates.cbegin(), rates.cend());
}

int ExportDialogModel::bitRate() const
{
    return audioExportConfiguration()->exportMp3Bitrate();
}

void ExportDialogModel::setBitRate(int rate)
{
    if (rate == bitRate()) {
        return;
    }

    audioExportConfiguration()->setExportMp3Bitrate(rate);
    emit bitRateChanged(rate);
}

bool ExportDialogModel::midiExpandRepeats() const
{
    return midiImportExportConfiguration()->isExpandRepeats();
}

void ExportDialogModel::setMidiExpandRepeats(bool expandRepeats)
{
    if (expandRepeats == midiExpandRepeats()) {
        return;
    }

    midiImportExportConfiguration()->setExpandRepeats(expandRepeats);
    emit midiExpandRepeatsChanged(expandRepeats);
}

bool ExportDialogModel::midiExportRpns() const
{
    return midiImportExportConfiguration()->isMidiExportRpns();
}

void ExportDialogModel::setMidiExportRpns(bool exportRpns)
{
    if (exportRpns == midiExportRpns()) {
        return;
    }

    midiImportExportConfiguration()->setIsMidiExportRpns(exportRpns);
    emit midiExportRpnsChanged(exportRpns);
}

bool ExportDialogModel::meiExportLayout() const
{
    return meiConfiguration()->meiExportLayout();
}

void ExportDialogModel::setMeiExportLayout(bool exportLayout)
{
    if (exportLayout == meiExportLayout()) {
        return;
    }

    meiConfiguration()->setMeiExportLayout(exportLayout);
    emit meiExportLayoutChanged(exportLayout);
}

bool ExportDialogModel::meiUseMuseScoreIds() const
{
    return meiConfiguration()->meiUseMuseScoreIds();
}

void ExportDialogModel::setMeiUseMuseScoreIds(bool useMuseScoreIds)
{
    if (useMuseScoreIds == meiUseMuseScoreIds()) {
        return;
    }

    meiConfiguration()->setMeiUseMuseScoreIds(useMuseScoreIds);
    emit meiUseMuseScoreIdsChanged(useMuseScoreIds);
}

QVariantList ExportDialogModel::musicXmlLayoutTypes() const
{
    QMap<MusicXmlLayoutType, QString> musicXmlLayoutTypeNames {
        //: Specifies to which extent layout customizations should be exported to MusicXML.
        { MusicXmlLayoutType::AllLayout, muse::qtrc("project/export", "All layout") },
        //: Specifies to which extent layout customizations should be exported to MusicXML.
        { MusicXmlLayoutType::AllBreaks, muse::qtrc("project/export", "System and page breaks") },
        //: Specifies to which extent layout customizations should be exported to MusicXML.
        { MusicXmlLayoutType::ManualBreaks, muse::qtrc("project/export", "Manually added system and page breaks only") },
        //: Specifies to which extent layout customizations should be exported to MusicXML.
        { MusicXmlLayoutType::None, muse::qtrc("project/export", "No system or page breaks") },
    };

    QVariantList result;

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
    if (musicXmlConfiguration()->exportLayout()) {
        return MusicXmlLayoutType::AllLayout;
    }
    switch (musicXmlConfiguration()->exportBreaksType()) {
    case IMusicXmlConfiguration::MusicXmlExportBreaksType::All:
        return MusicXmlLayoutType::AllBreaks;
    case IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual:
        return MusicXmlLayoutType::ManualBreaks;
    case IMusicXmlConfiguration::MusicXmlExportBreaksType::No:
        return MusicXmlLayoutType::None;
    }

    return MusicXmlLayoutType::AllLayout;
}

void ExportDialogModel::setMusicXmlLayoutType(MusicXmlLayoutType layoutType)
{
    if (layoutType == musicXmlLayoutType()) {
        return;
    }
    switch (layoutType) {
    case MusicXmlLayoutType::AllLayout:
        musicXmlConfiguration()->setExportLayout(true);
        break;
    case MusicXmlLayoutType::AllBreaks:
        musicXmlConfiguration()->setExportLayout(false);
        musicXmlConfiguration()->setExportBreaksType(IMusicXmlConfiguration::MusicXmlExportBreaksType::All);
        break;
    case MusicXmlLayoutType::ManualBreaks:
        musicXmlConfiguration()->setExportLayout(false);
        musicXmlConfiguration()->setExportBreaksType(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual);
        break;
    case MusicXmlLayoutType::None:
        musicXmlConfiguration()->setExportLayout(false);
        musicXmlConfiguration()->setExportBreaksType(IMusicXmlConfiguration::MusicXmlExportBreaksType::No);
        break;
    }
    emit musicXmlLayoutTypeChanged(layoutType);
}

bool ExportDialogModel::shouldDestinationFolderBeOpenedOnExport() const
{
    return configuration()->shouldDestinationFolderBeOpenedOnExport();
}

void ExportDialogModel::setShouldDestinationFolderBeOpenedOnExport(bool enabled)
{
    if (enabled == shouldDestinationFolderBeOpenedOnExport()) {
        return;
    }

    configuration()->setShouldDestinationFolderBeOpenedOnExport(enabled);
    emit shouldDestinationFolderBeOpenedOnExportChanged(enabled);
}

void ExportDialogModel::updateExportInfo()
{
    ExportInfo info;
    info.id = m_selectedExportType.id;
    info.exportPath = m_exportPath;
    info.unitType = m_selectedUnitType;

    INotationProjectPtr project = context()->currentProject();
    info.projectPath = project ? project->path() : "";

    std::vector<INotationPtr> notations;
    for (QModelIndex index : m_selectionModel->selectedIndexes()) {
        notations.emplace_back(m_notations[index.row()]);
    }
    info.notations = notations;

    exportProjectScenario()->setExportInfo(info);
}
