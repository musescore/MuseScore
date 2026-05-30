/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 */

#include "musesoundsreassignmodel.h"

#include <algorithm>
#include <QSet>

#include "project/inotationproject.h"
#include "notation/inotationparts.h"
#include "notation/inotationplayback.h"

#include "engraving/dom/instrument.h"
#include "engraving/dom/part.h"

#include "audio/common/audioutils.h"

using namespace mu::playback;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::audio;

static const muse::String MUSICXML_SOUND_ATTRIBUTE(u"musicXmlSound");
static const muse::String MUSE_UID_ATTRIBUTE(u"museUID");

static QString cleanText(QString text)
{
    text = text.toLower();

    for (QChar& ch : text) {
        if (!ch.isLetterOrNumber()) {
            ch = u' ';
        }
    }

    return text.simplified();
}

static QStringList tokensFrom(const QString& text)
{
    static const QSet<QString> ignored {
        QStringLiteral("a"), QStringLiteral("an"), QStringLiteral("and"), QStringLiteral("for"),
        QStringLiteral("in"), QStringLiteral("of"), QStringLiteral("the"), QStringLiteral("to"),
        QStringLiteral("with"), QStringLiteral("solo"), QStringLiteral("primary"),
        QStringLiteral("orchestral"), QStringLiteral("instrument")
    };

    QStringList result;
    for (const QString& token : cleanText(text).split(u' ', Qt::SkipEmptyParts)) {
        if (token.size() < 3 || ignored.contains(token)) {
            continue;
        }

        result.push_back(token);
    }

    return result;
}

static QString resourceTitle(const AudioResourceMeta& resource)
{
    QStringList parts;

    const QString vendor = resource.attributeVal(u"museVendorName").toQString();
    const QString pack = resource.attributeVal(u"musePack").toQString();
    const QString name = resource.attributeVal(u"museName").toQString();

    if (!vendor.isEmpty()) {
        parts << vendor;
    }

    if (!pack.isEmpty() && pack != vendor) {
        parts << pack;
    }

    if (!name.isEmpty()) {
        parts << name;
    } else {
        parts << QString::fromStdString(resource.id);
    }

    return parts.join(u" / ");
}

static bool isUsableMuseSamplerResource(const AudioResourceMeta& resource)
{
    return resource.type == AudioResourceType::MuseSamplerSoundPack
           && resource.isValid()
           && !resource.attributeVal(MUSE_UID_ATTRIBUTE).isEmpty();
}

MuseSoundsReassignModel::MuseSoundsReassignModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void MuseSoundsReassignModel::load()
{
    beginResetModel();
    m_choices.clear();
    endResetModel();
    emit choicesChanged();

    playback()->availableInputResources()
    .onResolve(this, [this](const AudioResourceMetaList& availableResources) {
        buildChoices(availableResources);
    });
}

void MuseSoundsReassignModel::setSelectedCandidate(int row, int candidateIndex)
{
    if (row < 0 || row >= static_cast<int>(m_choices.size())) {
        return;
    }

    TrackChoice& choice = m_choices[row];
    if (candidateIndex < 0 || candidateIndex >= static_cast<int>(choice.candidates.size())) {
        return;
    }

    if (choice.selectedCandidateIndex == candidateIndex) {
        return;
    }

    choice.selectedCandidateIndex = candidateIndex;

    QModelIndex modelIndex = index(row, 0);
    emit dataChanged(modelIndex, modelIndex, { SelectedCandidateIndexRole });
}

void MuseSoundsReassignModel::apply()
{
    for (const TrackChoice& choice : m_choices) {
        if (choice.audioTrackId == INVALID_TRACK_ID) {
            continue;
        }

        if (choice.selectedCandidateIndex < 0 || choice.selectedCandidateIndex >= static_cast<int>(choice.candidates.size())) {
            continue;
        }

        AudioInputParams params;
        params.resourceMeta = choice.candidates[choice.selectedCandidateIndex].resource;
        playback()->setSourceParams(choice.audioTrackId, params);
    }
}

int MuseSoundsReassignModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_choices.size());
}

QVariant MuseSoundsReassignModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_choices.size())) {
        return QVariant();
    }

    const TrackChoice& choice = m_choices[index.row()];

    switch (role) {
    case StaffNameRole:
        return choice.staffName;
    case CurrentSoundRole:
        return choice.currentSound;
    case CandidateTitlesRole:
        return choice.candidateTitles;
    case SelectedCandidateIndexRole:
        return choice.selectedCandidateIndex;
    case CandidateCountRole:
        return static_cast<int>(choice.candidates.size());
    }

    return QVariant();
}

QHash<int, QByteArray> MuseSoundsReassignModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { StaffNameRole, "staffName" },
        { CurrentSoundRole, "currentSound" },
        { CandidateTitlesRole, "candidateTitles" },
        { SelectedCandidateIndexRole, "selectedCandidateIndex" },
        { CandidateCountRole, "candidateCount" }
    };

    return roles;
}

bool MuseSoundsReassignModel::hasChoices() const
{
    return !m_choices.empty();
}

void MuseSoundsReassignModel::buildChoices(const AudioResourceMetaList& availableResources)
{
    AudioResourceMetaList museResources;
    for (const AudioResourceMeta& resource : availableResources) {
        if (isUsableMuseSamplerResource(resource)) {
            museResources.push_back(resource);
        }
    }

    INotationProjectPtr project = context()->currentProject();
    INotationPlaybackPtr notationPlayback = project ? project->masterNotation()->playback() : nullptr;
    IProjectAudioSettingsPtr audioSettings = project ? project->audioSettings() : nullptr;

    if (!notationPlayback || !audioSettings || museResources.empty()) {
        return;
    }

    std::vector<TrackChoice> choices;
    const InstrumentTrackId& metronomeTrackId = notationPlayback->metronomeTrackId();

    for (const auto& pair : controller()->instrumentTrackIdMap()) {
        const InstrumentTrackId& instrumentTrackId = pair.first;
        if (instrumentTrackId == metronomeTrackId || notationPlayback->isChordSymbolsTrack(instrumentTrackId)) {
            continue;
        }

        const mpe::PlaybackData& playbackData = notationPlayback->trackPlaybackData(instrumentTrackId);
        QString staff = trackName(instrumentTrackId);
        std::vector<Candidate> candidates = candidatesForTrack(staff, playbackData.setupData, museResources);
        if (candidates.empty()) {
            continue;
        }

        const AudioInputParams& currentParams = audioSettings->trackInputParams(instrumentTrackId);

        TrackChoice choice;
        choice.instrumentTrackId = instrumentTrackId;
        choice.audioTrackId = pair.second;
        choice.staffName = staff;
        choice.currentSound = audioSourceName(currentParams).toQString();
        choice.candidates = std::move(candidates);

        for (size_t i = 0; i < choice.candidates.size(); ++i) {
            choice.candidateTitles << choice.candidates[i].title;
            if (currentParams.resourceMeta.type == AudioResourceType::MuseSamplerSoundPack
                && currentParams.resourceMeta.id == choice.candidates[i].resource.id) {
                choice.selectedCandidateIndex = static_cast<int>(i);
            }
        }

        choices.push_back(std::move(choice));
    }

    beginResetModel();
    m_choices = std::move(choices);
    endResetModel();
    emit choicesChanged();
}

std::vector<MuseSoundsReassignModel::Candidate> MuseSoundsReassignModel::candidatesForTrack(
    const QString& staffName, const mpe::PlaybackSetupData& setupData, const AudioResourceMetaList& museResources) const
{
    QString setupText = setupData.toString().toQString();
    QString musicXmlSound = setupData.musicXmlSoundId.has_value()
                            ? QString::fromStdString(setupData.musicXmlSoundId.value())
                            : QString();
    QStringList queryTokens = tokensFrom(staffName + u" " + setupText + u" " + musicXmlSound);

    std::vector<Candidate> result;

    for (const AudioResourceMeta& resource : museResources) {
        QString title = resourceTitle(resource);
        QString resourceSetupText = resource.attributeVal(PLAYBACK_SETUP_DATA_ATTRIBUTE).toQString();
        QString resourceMusicXmlSound = resource.attributeVal(MUSICXML_SOUND_ATTRIBUTE).toQString();
        QString haystack = title + u" "
                           + resourceSetupText + u" "
                           + resourceMusicXmlSound + u" "
                           + resource.attributeVal(u"museCategory").toQString();

        QString cleanHaystack = cleanText(haystack);

        int score = 0;
        if (!setupText.isEmpty() && resourceSetupText == setupText) {
            score += 100;
        }

        if (!musicXmlSound.isEmpty() && resourceMusicXmlSound == musicXmlSound) {
            score += 90;
        }

        for (const QString& token : queryTokens) {
            if (cleanHaystack.contains(token)) {
                score += 10;
            }
        }

        if (score == 0) {
            continue;
        }

        result.push_back(Candidate { resource, title, score });
    }

    std::sort(result.begin(), result.end(), [](const Candidate& left, const Candidate& right) {
        if (left.score != right.score) {
            return left.score > right.score;
        }

        return left.title.localeAwareCompare(right.title) < 0;
    });

    if (result.size() > 20) {
        result.resize(20);
    }

    return result;
}

QString MuseSoundsReassignModel::trackName(const InstrumentTrackId& instrumentTrackId) const
{
    INotationProjectPtr project = context()->currentProject();
    INotationPartsPtr parts = project ? project->masterNotation()->parts() : nullptr;

    const Part* part = parts ? parts->part(instrumentTrackId.partId) : nullptr;
    if (!part) {
        return QString();
    }

    const Instrument* instrument = part->instrumentById(instrumentTrackId.instrumentId);
    if (instrument && instrument != part->instrument()) {
        return instrument->trackName().toQString();
    }

    return part->partName().toQString();
}
