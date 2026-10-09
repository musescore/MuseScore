/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <algorithm>
#include <set>

#include <QItemSelectionModel>
#include <QTimer>

#include "async/notifylist.h"
#include "engraving/dom/part.h"
#include "notation/iexcerptnotation.h" // IWYU pragma: keep
#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationparts.h"
#include "notation/inotationselection.h"

#include "translation.h"
#include "log.h"

using namespace muse;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::iex::musicxml;

using UnitType = INotationWriter::UnitType;

static const UnitType DEFAULT_EXPORT_UNITTYPE = UnitType::PER_PART;

ExportDialogModel::ExportDialogModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
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
                                     "WavSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "ogg" },
                                     muse::qtrc("project/export", "OGG audio"),
                                     muse::qtrc("project/export", "OGG audio files"),
                                     "OggSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "flac" },
                                     muse::qtrc("project/export", "FLAC audio"),
                                     muse::qtrc("project/export", "FLAC audio files"),
                                     "FlacSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "aac" },
                                     muse::qtrc("project/export", "AAC audio"),
                                     muse::qtrc("project/export", "AAC audio files"),
                                     "AacSettingsPage.qml"),
#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
        ExportType::makeWithSuffixes({ "mp4" },
                                     muse::qtrc("project/export", "MP4 video"),
                                     muse::qtrc("project/export", "MP4 video"),
                                     "Mp4SettingsPage.qml"),
#endif
        ExportType::makeWithSuffixes({ "mid", "midi", "kar" },
                                     muse::qtrc("project/export", "MIDI file"),
                                     muse::qtrc("project/export", "MIDI files"),
                                     "MidiSettingsPage.qml"),
        ExportType::makeWithSubtypes(musicXmlTypes,
                                     muse::qtrc("project/export", "MusicXML")),
        ExportType::makeWithSuffixes({ "brf" },
                                     //: Meaning like "measure-over-measure", but called "bar-over-bar"
                                     //: even in US English. Not to be confused with "bar-by-bar" format.
                                     //: See https://handbook.musescore.org/file-management/file-export#braille
                                     muse::qtrc("project/export", "Braille (basic bar-over-bar)"),
                                     muse::qtrc("project/export", "Braille files")),
        ExportType::makeWithSuffixes({ "mei" },
                                     muse::qtrc("project/export", "MEI"),
                                     muse::qtrc("project/export", "MEI files"),
                                     "MeiSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "mnx" },
                                     muse::qtrc("project/export", "MNX (experimental)"),
                                     muse::qtrc("project/export", "MNX files (experimental)"),
                                     "MnxSettingsPage.qml"),
        ExportType::makeWithSuffixes({ "lrc" },
                                     muse::qtrc("project/export", "LRC file"),
                                     muse::qtrc("project/export", "LRC files"),
                                     "LrcSettingsPage.qml")
    };
}

ExportDialogModel::~ExportDialogModel()
{
#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
    if (videoEncoderResolver()) {
        disableVideoExportSettingMode();
    }
#endif

    m_selectionModel->deleteLater();
}

void ExportDialogModel::classBegin()
{
    init();
}

void ExportDialogModel::init()
{
    TRACEFUNC;

    const ExportInfo& info = exportProjectScenario()->exportInfo();
    if (info.id.isEmpty()) {
        setExportType(m_exportTypeList.front());
    } else {
        selectExportTypeById(info.id);
    }

    m_exportDirPath = info.exportDirPath;
    setUnitType(info.unitType);
    ensureAudioExportTypeSelected();

    beginResetModel();
    m_notations.clear();

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

    for (const IExcerptNotationPtr& excerpt : excerpts) {
        m_notations << excerpt->notation();
    }

    endResetModel();

    selectCurrentNotation();
    selectSavedNotations();

#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
    videoEncoderResolver()->loadedFFmpegChanged().onNotify(this, [this]() {
        emit isFFmpegAvailableChanged();
        emit ffmpegDirChanged();
    });
    emit isFFmpegAvailableChanged();
    emit ffmpegDirChanged();
#endif
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
    const ExportInfo& info = exportProjectScenario()->exportInfo();
    for (const INotationWeakPtr& notation : info.notations) {
        const INotationPtr ptr = notation.lock();
        if (!ptr) {
            continue;
        }

        auto it = std::find(m_notations.begin(), m_notations.end(), ptr);
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

bool ExportDialogModel::isFormatSelected(const QString& formatSuffix) const
{
    IF_ASSERT_FAILED(!m_selectedExportType.suffixes.empty()) {
        return false;
    }

    return m_selectedExportType.suffixes.contains(formatSuffix);
}

int ExportDialogModel::selectionLength() const
{
    return m_selectionModel->selectedIndexes().size();
}

bool ExportDialogModel::selectionMode() const
{
    return m_selectionMode;
}

void ExportDialogModel::setSelectionMode(bool selectionMode)
{
    if (m_selectionMode == selectionMode) {
        return;
    }

    m_selectionMode = selectionMode;
    emit selectionModeChanged();
    emit exportTypeListChanged();

    if (m_selectionMode) {
        updateSelectionInstrumentInfo();
    }

    ensureAudioExportTypeSelected();
}

int ExportDialogModel::selectionTempoPercentage() const
{
    return m_selectionTempoPercentage;
}

void ExportDialogModel::setSelectionTempoPercentage(int percentage)
{
    percentage = std::clamp(percentage, 10, 300);
    if (m_selectionTempoPercentage == percentage) {
        return;
    }

    m_selectionTempoPercentage = percentage;
    emit selectionTempoPercentageChanged();
}

bool ExportDialogModel::selectionMetronomeEnabled() const
{
    return m_selectionMetronomeEnabled;
}

void ExportDialogModel::setSelectionMetronomeEnabled(bool enabled)
{
    if (m_selectionMetronomeEnabled == enabled) {
        return;
    }

    m_selectionMetronomeEnabled = enabled;
    if (m_selectionMetronomeEnabled) {
        setSelectionFadeInEnabled(false);
    }
    emit selectionMetronomeEnabledChanged();
}

bool ExportDialogModel::selectionFadeInEnabled() const
{
    return m_selectionFadeInEnabled;
}

void ExportDialogModel::setSelectionFadeInEnabled(bool enabled)
{
    if (m_selectionFadeInEnabled == enabled) {
        return;
    }

    m_selectionFadeInEnabled = enabled;
    emit selectionFadeInEnabledChanged();
}

double ExportDialogModel::selectionFadeInDuration() const
{
    return m_selectionFadeInDuration;
}

void ExportDialogModel::setSelectionFadeInDuration(double duration)
{
    duration = std::clamp(duration, 0.1, 30.0);
    if (muse::RealIsEqual(m_selectionFadeInDuration, duration)) {
        return;
    }

    m_selectionFadeInDuration = duration;
    emit selectionFadeInDurationChanged();
}

bool ExportDialogModel::selectionFadeOutEnabled() const
{
    return m_selectionFadeOutEnabled;
}

void ExportDialogModel::setSelectionFadeOutEnabled(bool enabled)
{
    if (m_selectionFadeOutEnabled == enabled) {
        return;
    }

    m_selectionFadeOutEnabled = enabled;
    emit selectionFadeOutEnabledChanged();
}

double ExportDialogModel::selectionFadeOutDuration() const
{
    return m_selectionFadeOutDuration;
}

void ExportDialogModel::setSelectionFadeOutDuration(double duration)
{
    duration = std::clamp(duration, 0.1, 30.0);
    if (muse::RealIsEqual(m_selectionFadeOutDuration, duration)) {
        return;
    }

    m_selectionFadeOutDuration = duration;
    emit selectionFadeOutDurationChanged();
}

int ExportDialogModel::selectionOtherInstrumentsVolume() const
{
    return m_selectionOtherInstrumentsVolume;
}

void ExportDialogModel::setSelectionOtherInstrumentsVolume(int percentage)
{
    percentage = std::clamp(percentage, 0, 100);
    if (m_selectionOtherInstrumentsVolume == percentage) {
        return;
    }

    m_selectionOtherInstrumentsVolume = percentage;
    emit selectionOtherInstrumentsVolumeChanged();

    bool instrumentsChanged = false;
    for (SelectionInstrument& instrument : m_selectionInstruments) {
        if (!instrument.selected && instrument.volume != percentage) {
            instrument.volume = percentage;
            instrumentsChanged = true;
        }
    }

    if (instrumentsChanged) {
        emit selectionInstrumentsChanged();
    }
}

bool ExportDialogModel::selectionOtherInstrumentsMuted() const
{
    return m_selectionOtherInstrumentsMuted;
}

void ExportDialogModel::setSelectionOtherInstrumentsMuted(bool muted)
{
    if (m_selectionOtherInstrumentsMuted == muted) {
        return;
    }

    m_selectionOtherInstrumentsMuted = muted;
    emit selectionOtherInstrumentsMutedChanged();
}

QVariantList ExportDialogModel::selectionInstruments() const
{
    QVariantList result;
    result.reserve(static_cast<qsizetype>(m_selectionInstruments.size()));

    for (size_t i = 0; i < m_selectionInstruments.size(); ++i) {
        const SelectionInstrument& instrument = m_selectionInstruments[i];
        QVariantMap item;
        item["index"] = static_cast<int>(i);
        item["name"] = instrument.name;
        item["selected"] = instrument.selected;
        item["volume"] = instrument.volume;
        result.append(item);
    }

    return result;
}

bool ExportDialogModel::hasOtherInstruments() const
{
    return std::any_of(m_selectionInstruments.cbegin(), m_selectionInstruments.cend(), [](const SelectionInstrument& instrument) {
        return !instrument.selected;
    });
}

void ExportDialogModel::setSelectionInstrumentVolume(int index, int percentage)
{
    if (index < 0 || static_cast<size_t>(index) >= m_selectionInstruments.size()) {
        return;
    }

    percentage = std::clamp(percentage, 0, 200);
    SelectionInstrument& instrument = m_selectionInstruments[static_cast<size_t>(index)];
    if (instrument.volume == percentage) {
        return;
    }

    instrument.volume = percentage;
    emit selectionInstrumentsChanged();
}

void ExportDialogModel::updateSelectionInstrumentInfo()
{
    m_selectionInstruments.clear();

    const INotationPtr notation = context()->currentNotation();
    const INotationSelectionPtr selection = notation ? notation->interaction()->selection() : nullptr;
    const INotationSelectionRangePtr range = selection ? selection->range() : nullptr;
    if (!notation || !notation->parts() || !selection || !selection->isRange() || !range) {
        emit selectionInstrumentsChanged();
        return;
    }

    std::set<muse::ID> selectedPartIds;
    for (const engraving::Part* part : range->selectedParts()) {
        if (part) {
            selectedPartIds.insert(part->id());
        }
    }

    const async::NotifyList<const engraving::Part*> parts = notation->parts()->partList();
    for (const engraving::Part* part : parts) {
        if (!part) {
            continue;
        }

        const bool selected = selectedPartIds.contains(part->id());
        m_selectionInstruments.push_back({ part->id(), part->partName().toQString(), selected,
                                           selected ? 100 : m_selectionOtherInstrumentsVolume });
    }

    const bool shouldMuteOthersByDefault = selectedPartIds.size() == 1 && hasOtherInstruments();
    if (m_selectionOtherInstrumentsMuted != shouldMuteOthersByDefault) {
        m_selectionOtherInstrumentsMuted = shouldMuteOthersByDefault;
        emit selectionOtherInstrumentsMutedChanged();
    }

    emit selectionInstrumentsChanged();
}

QVariantList ExportDialogModel::exportTypeList() const
{
    if (!m_selectionMode) {
        return m_exportTypeList.toVariantList();
    }

    ExportTypeList audioTypes;
    for (const ExportType& type : m_exportTypeList) {
        if (isAudioExportType(type)) {
            audioTypes.push_back(type);
        }
    }

    return audioTypes.toVariantList();
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
    emit availableSampleFormatsChanged();
    emit selectedSampleFormatChanged();

#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
    updateVideoExportSettingMode();
#endif

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
    for (const ExportType& type : std::as_const(m_exportTypeList)) {
        if (m_selectionMode && !isAudioExportType(type)) {
            continue;
        }

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

bool ExportDialogModel::isAudioExportType(const ExportType& type) const
{
    return !type.suffixes.empty() && project::isAudioExport(type.suffixes.front().toStdString());
}

void ExportDialogModel::ensureAudioExportTypeSelected()
{
    if (!m_selectionMode || isAudioExportType(m_selectedExportType)) {
        return;
    }

    const auto it = std::find_if(m_exportTypeList.cbegin(), m_exportTypeList.cend(), [this](const ExportType& type) {
        return isAudioExportType(type);
    });

    IF_ASSERT_FAILED(it != m_exportTypeList.cend()) {
        return;
    }

    setExportType(*it);
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
    for (const QVariant& availabeUnitType : availableUnitTypes()) {
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
    INotationWriter::Options writerOptions;

    if (m_selectionMode) {
        const INotationPtr notation = context()->currentNotation();
        const INotationSelectionPtr selection = notation ? notation->interaction()->selection() : nullptr;
        if (!selection || !selection->isRange() || !selection->range()) {
            return false;
        }

        const INotationSelectionRangePtr range = selection->range();
        writerOptions[INotationWriter::OptionKey::AUDIO_EXPORT_START_TICK]
            = Val(static_cast<int64_t>(range->startTick().ticks()));
        writerOptions[INotationWriter::OptionKey::AUDIO_EXPORT_END_TICK]
            = Val(static_cast<int64_t>(range->endTick().ticks()));
        writerOptions[INotationWriter::OptionKey::AUDIO_EXPORT_TEMPO_PERCENT]
            = Val(m_selectionTempoPercentage);
        writerOptions[INotationWriter::OptionKey::AUDIO_EXPORT_METRONOME_ENABLED]
            = Val(m_selectionMetronomeEnabled);
        if (m_selectionFadeInEnabled) {
            writerOptions[INotationWriter::OptionKey::AUDIO_EXPORT_FADE_IN_SEC]
                = Val(m_selectionFadeInDuration);
        }
        if (m_selectionFadeOutEnabled) {
            writerOptions[INotationWriter::OptionKey::AUDIO_EXPORT_FADE_OUT_SEC]
                = Val(m_selectionFadeOutDuration);
        }
        ValList partVolumes;
        partVolumes.reserve(m_selectionInstruments.size());
        for (const SelectionInstrument& instrument : m_selectionInstruments) {
            const int volume = m_selectionOtherInstrumentsMuted && !instrument.selected ? 0 : instrument.volume;
            ValMap partVolume {
                { "partId", Val(instrument.partId.toStdString()) },
                { "volume", Val(volume) },
            };
            partVolumes.emplace_back(partVolume);
        }
        writerOptions[INotationWriter::OptionKey::AUDIO_EXPORT_PART_VOLUMES] = Val(partVolumes);
        notations.push_back(notation);
    } else {
        for (const QModelIndex& index : m_selectionModel->selectedIndexes()) {
            notations.push_back(m_notations[index.row()]);
        }
    }

    if (notations.empty()) {
        return false;
    }

    std::shared_ptr<IExportProjectScenario> scenario = exportProjectScenario();

    RetVal<muse::io::path_t> exportPath
        = scenario->askExportPath(notations, m_selectedExportType, m_selectedUnitType, m_exportDirPath);
    if (!exportPath.ret) {
        return false;
    }

    m_exportDirPath = io::absoluteDirpath(exportPath.val);

    struct Params {
        INotationPtrList notations;
        muse::io::path_t exportPath;
        project::INotationWriter::UnitType selectedUnitType;
        bool openFolderOnExport = false;
        INotationWriter::Options writerOptions;
    };

    //! NOTE We want to call the scenario method on the next event loop.
    //! But by that point, the dialog may already be destroyed.
    //! Therefore, we save everything we need in variables
    //! and pass them to the lambda.
    //! We can't access the dialog class member in the lambda body.
    Params params{ notations, exportPath.val, m_selectedUnitType,
                   shouldDestinationFolderBeOpenedOnExport(), writerOptions };

    //! NOTE We can't use Async here
    // because the async::processMessages is called deep within the functions,
    // and this can't be done in Async's callback.
    QTimer::singleShot(0, [scenario, params]() {
        scenario->exportScores(params.notations, params.exportPath, params.selectedUnitType, params.openFolderOnExport,
                               params.writerOptions);
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

bool ExportDialogModel::pdfGrayscale() const
{
    return imageExportConfiguration()->exportPdfWithGrayscale();
}

void ExportDialogModel::setPdfGrayscale(const bool& grayscale)
{
    if (grayscale == pdfGrayscale()) {
        return;
    }

    imageExportConfiguration()->setExportPdfWithGrayscale(grayscale);
    emit pdfGrayscaleChanged(grayscale);
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

bool ExportDialogModel::pngGrayscale() const
{
    return imageExportConfiguration()->exportPngWithGrayscale();
}

void ExportDialogModel::setPngGrayscale(const bool& grayscale)
{
    if (grayscale == pngGrayscale()) {
        return;
    }

    imageExportConfiguration()->setExportPngWithGrayscale(grayscale);
    emit pngGrayscaleChanged(grayscale);
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

bool ExportDialogModel::svgIllustratorCompat() const
{
    return imageExportConfiguration()->exportSvgWithIllustratorCompat();
}

void ExportDialogModel::setSvgIllustratorCompat(bool compat)
{
    if (compat == svgIllustratorCompat()) {
        return;
    }

    imageExportConfiguration()->setExportSvgWithIllustratorCompat(compat);
    emit svgIllustratorCompatChanged(compat);
}

QStringList ExportDialogModel::availableVideoResolutions() const
{
    const std::vector<std::string>& resolutions = videoExportConfiguration()->availableResolutions();
    QStringList result;
    for (const std::string& res : resolutions) {
        result << QString::fromStdString(res);
    }
    return result;
}

QString ExportDialogModel::videoResolution() const
{
    return QString::fromStdString(videoExportConfiguration()->resolution());
}

void ExportDialogModel::setVideoResolution(const QString& resolution)
{
    if (resolution == videoResolution()) {
        return;
    }

    videoExportConfiguration()->setResolution(resolution.toStdString());
    emit videoResolutionChanged(resolution);
}

QVariantList ExportDialogModel::availableViewModes() const
{
    std::map<ViewMode, QString> viewModes {
        { ViewMode::PageFull, muse::qtrc("project/export", "Use page layout") },
        { ViewMode::Flexible, muse::qtrc("project/export", "Reflow to fit video resolution") },
    };

    QVariantList result;

    for (const auto& [value, text] : viewModes) {
        QVariantMap obj;
        obj["value"] = value;
        obj["text"] = text;
        result << obj;
    }

    return result;
}

void ExportDialogModel::setViewMode(ViewMode v)
{
    if (v == videoExportConfiguration()->viewMode()) {
        return;
    }

    videoExportConfiguration()->setViewMode(v);
    emit viewModeChanged(v);
}

ExportDialogModel::ViewMode ExportDialogModel::viewMode() const
{
    return videoExportConfiguration()->viewMode();
}

bool ExportDialogModel::isFFmpegAvailable() const
{
#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
    return videoEncoderResolver()->loadedFFmpegVersion() != muse::media::FFMPEG_INVALID_VERSION;
#else
    return false;
#endif
}

QString ExportDialogModel::ffmpegDir() const
{
#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
    return videoEncoderResolver()->loadedFFmpegDir().toQString();
#else
    return QString();
#endif
}

void ExportDialogModel::setFFmpegDir(const QString& dir)
{
#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
    if (ffmpegDir() == dir) {
        return;
    }

    videoEncoderResolver()->loadFFmpeg(dir);
#else
    Q_UNUSED(dir);
#endif
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

bool ExportDialogModel::lrcUseEnhancedFormat() const
{
    return lrcConfiguration()->lrcUseEnhancedFormat();
}

void ExportDialogModel::setLrcUseEnhancedFormat(bool useEnhancedFormat)
{
    if (useEnhancedFormat == lrcUseEnhancedFormat()) {
        return;
    }

    lrcConfiguration()->setLrcUseEnhancedFormat(useEnhancedFormat);
    emit lrcUseEnhancedFormatChanged(useEnhancedFormat);
}

int ExportDialogModel::mnxIndentSpaces() const
{
    return mnxConfiguration()->mnxIndentSpaces();
}

void ExportDialogModel::setMnxIndentSpaces(int spaces)
{
    spaces = std::clamp(spaces, -1, 8);
    if (spaces == mnxIndentSpaces()) {
        return;
    }

    mnxConfiguration()->setMnxIndentSpaces(spaces);
    emit mnxIndentSpacesChanged(spaces);
}

bool ExportDialogModel::mnxExportBeams() const
{
    return mnxConfiguration()->mnxExportBeams();
}

void ExportDialogModel::setMnxExportBeams(bool exportBeams)
{
    if (exportBeams == mnxExportBeams()) {
        return;
    }

    mnxConfiguration()->setMnxExportBeams(exportBeams);
    emit mnxExportBeamsChanged(exportBeams);
}

bool ExportDialogModel::mnxExportRestPositions() const
{
    return mnxConfiguration()->mnxExportRestPositions();
}

void ExportDialogModel::setMnxExportRestPositions(bool exportRestPositions)
{
    if (exportRestPositions == mnxExportRestPositions()) {
        return;
    }

    mnxConfiguration()->setMnxExportRestPositions(exportRestPositions);
    emit mnxExportRestPositionsChanged(exportRestPositions);
}

QVariantList ExportDialogModel::musicXmlLayoutTypes() const
{
    std::map<MusicXmlLayoutType, QString> musicXmlLayoutTypeNames {
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

    for (const auto& [type, name] : musicXmlLayoutTypeNames) {
        QVariantMap obj;
        obj["text"] = name;
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
    info.exportDirPath = m_exportDirPath;
    info.unitType = m_selectedUnitType;

    for (const QModelIndex& index : m_selectionModel->selectedIndexes()) {
        info.notations.emplace_back(m_notations[index.row()]);
    }

    exportProjectScenario()->setExportInfo(info);
}

QVariantList ExportDialogModel::availableSampleFormats() const
{
    std::vector<muse::audio::AudioSampleFormat> formats;

    if (isFormatSelected("wav")) {
        formats = audioExportConfiguration()->availableWavSampleFormats();
    }
    if (isFormatSelected("flac")) {
        formats = audioExportConfiguration()->availableFlacSampleFormats();
    }

    QVariantList result;
    for (const auto& format : formats) {
        QVariantMap obj;
        obj["text"] = audioExportConfiguration()->sampleFormatToString(format);
        obj["value"] = static_cast<int>(format);
        result << obj;
    }
    return result;
}

int ExportDialogModel::selectedSampleFormat() const
{
    if (isFormatSelected("wav")) {
        return static_cast<int>(audioExportConfiguration()->exportWavSampleFormat());
    }
    if (isFormatSelected("flac")) {
        return static_cast<int>(audioExportConfiguration()->exportFlacSampleFormat());
    }
    return static_cast<int>(muse::audio::AudioSampleFormat::Undefined);
}

void ExportDialogModel::setSelectedSampleFormat(int format)
{
    const auto audioFormat = static_cast<muse::audio::AudioSampleFormat>(format);
    if (isFormatSelected("wav")) {
        if (audioFormat == audioExportConfiguration()->exportWavSampleFormat()) {
            return;
        }
        audioExportConfiguration()->setExportWavSampleFormat(audioFormat);
    } else if (isFormatSelected("flac")) {
        if (audioFormat == audioExportConfiguration()->exportFlacSampleFormat()) {
            return;
        }
        audioExportConfiguration()->setExportFlacSampleFormat(audioFormat);
    } else {
        return;
    }
    emit selectedSampleFormatChanged();
}

#ifdef MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE
void ExportDialogModel::updateVideoExportSettingMode()
{
    videoEncoderResolver()->setIsSettingMode(isFormatSelected("mp4"));
}

void ExportDialogModel::disableVideoExportSettingMode()
{
    videoEncoderResolver()->setIsSettingMode(false);
}

#endif
