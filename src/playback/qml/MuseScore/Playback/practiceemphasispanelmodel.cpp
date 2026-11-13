/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "practiceemphasispanelmodel.h"

#include <cmath>
#include <unordered_set>

#include "translation.h"

#include "rcommand/commandtypes.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurebase.h"
#include "engraving/playback/practiceemphasisdata.h"

#include "notation/inotation.h"
#include "notation/inotationelements.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationselection.h"

using namespace mu::playback;
using namespace mu::engraving;
using namespace mu::notation;

namespace {
int displayedMeasureNumber(const MeasureBase* measureBase)
{
    if (!measureBase || !measureBase->isMeasure()) {
        return 0;
    }
    const Measure* measure = toMeasure(measureBase);
    return measure->measureNumber() + measure->measureNumberOffset() + 1;
}

std::vector<PracticeEmphasisData::PartRule> visibleParts(const MasterScore* master)
{
    const int staves = master ? static_cast<int>(master->nstaves()) : 2;
    std::vector<PracticeEmphasisData::PartRule> result = PracticeEmphasisData::templatesForStaffCount(staves);
    if (!master) {
        return result;
    }

    std::unordered_set<std::string> seen;
    for (PracticeEmphasisData::PartRule& part : result) {
        seen.insert(part.id);
        for (const PracticeEmphasisData::PartRule& existing : master->practiceEmphasis().document().parts) {
            if (existing.id == part.id) {
                part = existing;
                break;
            }
        }
    }
    for (const PracticeEmphasisData::PartRule& part : master->practiceEmphasis().document().parts) {
        if (seen.insert(part.id).second) {
            result.push_back(part);
        }
    }
    return result;
}

const PracticeEmphasisData::PartRule* findVisible(const std::vector<PracticeEmphasisData::PartRule>& parts, const std::string& id)
{
    for (const PracticeEmphasisData::PartRule& part : parts) {
        if (part.id == id) {
            return &part;
        }
    }
    return nullptr;
}

QString dropdownLabel(const PracticeEmphasisData::PartRule& part)
{
    if (part.builtin) {
        return QString::fromStdString(part.id + "  " + part.name);
    }
    if (!part.name.empty()) {
        return QString::fromStdString(part.name);
    }
    return QString::fromStdString(part.id);
}

QString partCheckboxTooltip(const PracticeEmphasisData::PartRule& part)
{
    if (!part.builtin) {
        return muse::qtrc("playback", "Check to include this custom part in the loud mix.");
    }
    if (part.id == "S1") {
        return muse::qtrc("playback", "1st soprano: top notes on the soprano staff. A unison belongs to both 1st and 2nd.");
    }
    if (part.id == "S2") {
        return muse::qtrc("playback", "2nd soprano: notes below the top on the soprano staff.");
    }
    if (part.id == "A1") {
        return muse::qtrc("playback", "1st alto: top notes on the alto staff. Closed SATB shares the upper staff with soprano.");
    }
    if (part.id == "A2") {
        return muse::qtrc("playback", "2nd alto: notes below the top on the alto staff.");
    }
    if (part.id == "T1") {
        return muse::qtrc("playback", "1st tenor: top notes on the tenor staff. Closed SATB shares the lower staff with bass.");
    }
    if (part.id == "T2") {
        return muse::qtrc("playback", "2nd tenor: notes below the top on the tenor staff.");
    }
    if (part.id == "B1") {
        return muse::qtrc("playback", "1st bass: top notes on the bass staff.");
    }
    if (part.id == "B2") {
        return muse::qtrc("playback", "2nd bass: notes below the top on the bass staff.");
    }
    return dropdownLabel(part);
}
}

PracticeEmphasisPanelModel::PracticeEmphasisPanelModel(QObject* parent)
    : QObject(parent)
    , muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void PracticeEmphasisPanelModel::load()
{
    if (m_loaded) {
        refresh();
        return;
    }
    m_loaded = true;
    if (projectSession()) {
        projectSession()->currentNotationChanged().onNotify(this, [this]() {
            connectScore();
            refresh();
        });
    }
    connectScore();
    refresh();
}

void PracticeEmphasisPanelModel::attachScoreForTesting(MasterScore* score)
{
    m_testScore = score;
    connectScore();
    refresh();
}

void PracticeEmphasisPanelModel::setSelectionMeasuresForTesting(MeasureBase* from, MeasureBase* to)
{
    m_testSelectionFrom = from;
    m_testSelectionTo = to;
    refresh();
}

MasterScore* PracticeEmphasisPanelModel::currentMaster() const
{
    if (m_testScore) {
        return m_testScore;
    }
    if (!projectSession()) {
        return nullptr;
    }
    const INotationPtr notation = projectSession()->currentNotation();
    if (!notation || !notation->elements()) {
        return nullptr;
    }
    Score* score = notation->elements()->msScore();
    return score ? score->masterScore() : nullptr;
}

PracticeEmphasisData* PracticeEmphasisPanelModel::data() const
{
    MasterScore* master = currentMaster();
    return master ? &master->practiceEmphasis() : nullptr;
}

void PracticeEmphasisPanelModel::connectScore()
{
    // SetReplace keeps a single callback on this score. The previous score's channel
    // drops the callback itself when that score is destroyed.
    if (PracticeEmphasisData* emphasis = data()) {
        emphasis->changed().onReceive(this, [this]() {
            refresh();
        }, muse::async::Asyncable::Mode::SetReplace);
    }

    if (!projectSession() || !projectSession()->currentNotation() || !projectSession()->currentNotation()->interaction()) {
        return;
    }
    projectSession()->currentNotation()->interaction()->selectionChanged().onNotify(this, [this]() {
        refresh();
    }, muse::async::Asyncable::Mode::SetReplace);
}

void PracticeEmphasisPanelModel::refresh()
{
    emit stateChanged();
    emit editingPartChanged();
}

bool PracticeEmphasisPanelModel::hasScore() const
{
    return currentMaster() != nullptr;
}

bool PracticeEmphasisPanelModel::playbackActive() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis && emphasis->document().playbackActive;
}

bool PracticeEmphasisPanelModel::emphasisMode() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis && emphasis->document().emphasisMode;
}

int PracticeEmphasisPanelModel::emphasizedDb() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis ? static_cast<int>(std::lround(emphasis->document().emphasizedDb))
           : static_cast<int>(PracticeEmphasisData::DEFAULT_EMPHASIZED_DB);
}

int PracticeEmphasisPanelModel::otherDb() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis ? static_cast<int>(std::lround(emphasis->document().otherDb))
           : static_cast<int>(PracticeEmphasisData::DEFAULT_OTHER_DB);
}

int PracticeEmphasisPanelModel::fromMeasure() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis ? emphasis->document().fromMeasure : 0;
}

int PracticeEmphasisPanelModel::toMeasure() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis ? emphasis->document().toMeasure : 0;
}

QVariantList PracticeEmphasisPanelModel::parts() const
{
    MasterScore* master = currentMaster();
    const int staves = master ? static_cast<int>(master->nstaves()) : 2;
    QVariantList list;
    for (const PracticeEmphasisData::PartRule& part : visibleParts(master)) {
        QVariantMap item;
        item.insert(QStringLiteral("id"), QString::fromStdString(part.id));
        item.insert(QStringLiteral("label"), part.builtin ? QString::fromStdString(part.id)
                    : dropdownLabel(part));
        item.insert(QStringLiteral("tooltip"), partCheckboxTooltip(part));
        item.insert(QStringLiteral("selected"), master && master->practiceEmphasis().partSelected(part.id));
        item.insert(QStringLiteral("available"), master && (part.builtin ? part.staffIndex < staves : true));
        list.push_back(item);
    }
    return list;
}

QVariantList PracticeEmphasisPanelModel::editingPartOptions() const
{
    QVariantList list;
    for (const PracticeEmphasisData::PartRule& part : visibleParts(currentMaster())) {
        QVariantMap item;
        item.insert(QStringLiteral("text"), dropdownLabel(part));
        item.insert(QStringLiteral("value"), QString::fromStdString(part.id));
        list.push_back(item);
    }
    return list;
}

QString PracticeEmphasisPanelModel::editingPartId() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis ? QString::fromStdString(emphasis->document().editingPartId) : QString();
}

QString PracticeEmphasisPanelModel::editingPartName() const
{
    const std::string id = editingPartId().toStdString();
    if (id.empty()) {
        return {};
    }
    const PracticeEmphasisData* emphasis = data();
    if (emphasis) {
        for (const PracticeEmphasisData::PartRule& part : emphasis->document().parts) {
            if (part.id == id) {
                return QString::fromStdString(part.name);
            }
        }
    }
    if (const PracticeEmphasisData::PartRule* part = findVisible(visibleParts(currentMaster()), id)) {
        return QString::fromStdString(part->name);
    }
    return {};
}

bool PracticeEmphasisPanelModel::editingPartBuiltin() const
{
    const std::string id = editingPartId().toStdString();
    if (id.empty()) {
        return true;
    }
    if (const PracticeEmphasisData::PartRule* part = findVisible(visibleParts(currentMaster()), id)) {
        return part->builtin;
    }
    return true;
}

QString PracticeEmphasisPanelModel::editingPartParentId() const
{
    const std::string id = editingPartId().toStdString();
    if (id.empty()) {
        return {};
    }
    const PracticeEmphasisData* emphasis = data();
    if (emphasis) {
        for (const PracticeEmphasisData::PartRule& part : emphasis->document().parts) {
            if (part.id == id) {
                return QString::fromStdString(part.parentId);
            }
        }
    }
    if (const PracticeEmphasisData::PartRule* part = findVisible(visibleParts(currentMaster()), id)) {
        return QString::fromStdString(part->parentId);
    }
    return {};
}

int PracticeEmphasisPanelModel::indexOfOptionValue(const QVariantList& options, const QString& value) const
{
    for (int i = 0; i < options.size(); ++i) {
        const QVariantMap item = options.at(i).toMap();
        if (item.value(QStringLiteral("value")).toString() == value) {
            return i;
        }
    }
    return 0;
}

QString PracticeEmphasisPanelModel::editingValueAt(int index) const
{
    const QVariantList options = editingPartOptions();
    if (index < 0 || index >= options.size()) {
        return {};
    }
    return options.at(index).toMap().value(QStringLiteral("value")).toString();
}

QString PracticeEmphasisPanelModel::parentValueAt(int index) const
{
    const PracticeEmphasisData* emphasis = data();
    if (!emphasis || index < 0) {
        return {};
    }
    const std::vector<std::string> ids = emphasis->parentOptionIds(currentMaster(), editingPartId().toStdString());
    if (static_cast<size_t>(index) >= ids.size()) {
        return {};
    }
    return QString::fromStdString(ids.at(static_cast<size_t>(index)));
}

int PracticeEmphasisPanelModel::editingPartIndex() const
{
    return indexOfOptionValue(editingPartOptions(), editingPartId());
}

int PracticeEmphasisPanelModel::editingPartParentIndex() const
{
    const PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return 0;
    }
    return emphasis->parentOptionIndex(currentMaster(), editingPartId().toStdString(),
                                       editingPartParentId().toStdString());
}

bool PracticeEmphasisPanelModel::solo() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis && emphasis->document().solo;
}

QVariantList PracticeEmphasisPanelModel::parentOptions() const
{
    QVariantList list;
    QVariantMap none;
    none.insert(QStringLiteral("text"), muse::qtrc("playback", "(none)"));
    none.insert(QStringLiteral("value"), QString());
    list.push_back(none);

    const PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return list;
    }

    const std::string editingId = editingPartId().toStdString();
    const std::vector<PracticeEmphasisData::PartRule> parts = visibleParts(currentMaster());
    for (const std::string& id : emphasis->parentOptionIds(currentMaster(), editingId)) {
        if (id.empty()) {
            continue;
        }
        const PracticeEmphasisData::PartRule* part = findVisible(parts, id);
        QVariantMap item;
        item.insert(QStringLiteral("text"), part ? dropdownLabel(*part) : QString::fromStdString(id));
        item.insert(QStringLiteral("value"), QString::fromStdString(id));
        list.push_back(item);
    }
    return list;
}

bool PracticeEmphasisPanelModel::canDeleteEditingPart() const
{
    return hasScore() && !editingPartBuiltin() && !editingPartId().isEmpty();
}

QString PracticeEmphasisPanelModel::selectionButtonText() const
{
    auto labelForRange = [](int from, int to) {
        if (from <= 0) {
            return muse::qtrc("playback", "From Selection");
        }
        if (to <= 0) {
            to = from;
        }
        if (from == to) {
            return muse::qtrc("playback", "From Selection (%1)").arg(from);
        }
        return muse::qtrc("playback", "From Selection (%1–%2)").arg(from).arg(to);
    };

    if (m_testScore) {
        return labelForRange(displayedMeasureNumber(m_testSelectionFrom), displayedMeasureNumber(m_testSelectionTo));
    }

    if (!projectSession() || !projectSession()->currentNotation() || !projectSession()->currentNotation()->interaction()) {
        return muse::qtrc("playback", "From Selection");
    }
    const INotationSelectionPtr selection = projectSession()->currentNotation()->interaction()->selection();
    if (!selection) {
        return muse::qtrc("playback", "From Selection");
    }
    return labelForRange(displayedMeasureNumber(selection->startMeasureBase()),
                         displayedMeasureNumber(selection->endMeasureBase()));
}

void PracticeEmphasisPanelModel::setPlaybackActive(bool active)
{
    PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return;
    }
    if (!active) {
        emphasis->setEmphasisMode(false);
    }
    emphasis->setPlaybackActive(active);
}

void PracticeEmphasisPanelModel::setEmphasisMode(bool enabled)
{
    PracticeEmphasisData* emphasis = data();
    MasterScore* master = currentMaster();
    if (!emphasis || !master) {
        return;
    }
    if (enabled) {
        emphasis->setPlaybackActive(true);
        if (emphasis->document().editingPartId.empty()) {
            emphasis->setEditingPartId(master, "S1");
        }
    }
    emphasis->setEmphasisMode(enabled);
}

void PracticeEmphasisPanelModel::setEmphasizedDb(int db)
{
    PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return;
    }
    emphasis->setGains(static_cast<float>(db), emphasis->document().otherDb);
}

void PracticeEmphasisPanelModel::setOtherDb(int db)
{
    PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return;
    }
    emphasis->setGains(emphasis->document().emphasizedDb, static_cast<float>(db));
}

void PracticeEmphasisPanelModel::setFromMeasure(int measure)
{
    PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return;
    }
    emphasis->setRange(measure, emphasis->document().toMeasure);
}

void PracticeEmphasisPanelModel::setToMeasure(int measure)
{
    PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return;
    }
    emphasis->setRange(emphasis->document().fromMeasure, measure);
}

void PracticeEmphasisPanelModel::setPartSelected(const QString& partId, bool selected)
{
    MasterScore* master = currentMaster();
    if (!master) {
        return;
    }
    master->practiceEmphasis().setPartSelected(master, partId.toStdString(), selected);
}

void PracticeEmphasisPanelModel::setEditingPartId(const QString& partId)
{
    MasterScore* master = currentMaster();
    if (!master) {
        return;
    }
    master->practiceEmphasis().setEditingPartId(master, partId.toStdString());
    m_copySourceId = editingPartParentId();
}

void PracticeEmphasisPanelModel::setSolo(bool enabled)
{
    PracticeEmphasisData* emphasis = data();
    MasterScore* master = currentMaster();
    if (!emphasis || !master) {
        return;
    }
    if (enabled) {
        emphasis->setPlaybackActive(true);
        if (emphasis->document().editingPartId.empty()) {
            emphasis->setEditingPartId(master, "S1");
        }
    }
    emphasis->setSolo(enabled);
}

void PracticeEmphasisPanelModel::togglePanel()
{
    if (!commandDispatcher()) {
        return;
    }
    commandDispatcher()->dispatch(muse::rcommand::Command("command://app/dock/toggle-practice-emphasis"));
}

bool PracticeEmphasisPanelModel::canUndo() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis && emphasis->canUndo();
}

bool PracticeEmphasisPanelModel::canRedo() const
{
    const PracticeEmphasisData* emphasis = data();
    return emphasis && emphasis->canRedo();
}

void PracticeEmphasisPanelModel::undo()
{
    if (PracticeEmphasisData* emphasis = data()) {
        emphasis->undo();
    }
}

void PracticeEmphasisPanelModel::redo()
{
    if (PracticeEmphasisData* emphasis = data()) {
        emphasis->redo();
    }
}

QString PracticeEmphasisPanelModel::copySourceId() const
{
    return resolvedCopySourceId();
}

int PracticeEmphasisPanelModel::copySourceIndex() const
{
    return indexOfOptionValue(copySourceOptions(), resolvedCopySourceId());
}

void PracticeEmphasisPanelModel::setCopySourceId(const QString& sourceId)
{
    if (m_copySourceId == sourceId) {
        return;
    }
    m_copySourceId = sourceId;
    emit stateChanged();
}

QString PracticeEmphasisPanelModel::resolvedCopySourceId() const
{
    const QString editingId = editingPartId();
    const std::vector<PracticeEmphasisData::PartRule> parts = visibleParts(currentMaster());
    auto exists = [&](const QString& id) {
        if (id.isEmpty() || id == editingId) {
            return false;
        }
        for (const PracticeEmphasisData::PartRule& part : parts) {
            if (part.id == id.toStdString()) {
                return true;
            }
        }
        return false;
    };
    if (exists(m_copySourceId)) {
        return m_copySourceId;
    }
    const QString parentId = editingPartParentId();
    if (exists(parentId)) {
        return parentId;
    }
    for (const PracticeEmphasisData::PartRule& part : parts) {
        const QString id = QString::fromStdString(part.id);
        if (id != editingId) {
            return id;
        }
    }
    return {};
}

QVariantList PracticeEmphasisPanelModel::copySourceOptions() const
{
    QVariantList list;
    const std::string editingId = editingPartId().toStdString();
    for (const PracticeEmphasisData::PartRule& part : visibleParts(currentMaster())) {
        if (part.id == editingId) {
            continue;
        }
        QVariantMap item;
        item.insert(QStringLiteral("text"), dropdownLabel(part));
        item.insert(QStringLiteral("value"), QString::fromStdString(part.id));
        list.push_back(item);
    }
    return list;
}

bool PracticeEmphasisPanelModel::canCopyFromPart() const
{
    return hasScore() && !editingPartBuiltin() && !resolvedCopySourceId().isEmpty();
}

void PracticeEmphasisPanelModel::copyFromPart()
{
    MasterScore* master = currentMaster();
    PracticeEmphasisData* emphasis = data();
    const QString sourceId = resolvedCopySourceId();
    if (!master || !emphasis || editingPartBuiltin() || sourceId.isEmpty()) {
        return;
    }
    emphasis->copyFromPart(master, editingPartId().toStdString(), sourceId.toStdString());
}

void PracticeEmphasisPanelModel::applyEditingPartName(const QString& partId, const QString& name)
{
    PracticeEmphasisData* emphasis = data();
    if (!emphasis || partId.isEmpty() || partId != editingPartId()) {
        return;
    }
    emphasis->setPartName(partId.toStdString(), name.toStdString());
}

void PracticeEmphasisPanelModel::applyEditingPartParentId(const QString& partId, const QString& parentId)
{
    MasterScore* master = currentMaster();
    PracticeEmphasisData* emphasis = data();
    if (!master || !emphasis || partId.isEmpty() || partId != editingPartId()) {
        return;
    }
    emphasis->setPartParent(master, partId.toStdString(), parentId.toStdString());
}

void PracticeEmphasisPanelModel::useSelectionRange()
{
    auto applyRange = [this](int from, int to) {
        if (from <= 0) {
            return;
        }
        if (to <= 0) {
            to = from;
        }
        if (PracticeEmphasisData* emphasis = data()) {
            emphasis->setRange(from, to);
        }
    };

    if (m_testScore) {
        applyRange(displayedMeasureNumber(m_testSelectionFrom), displayedMeasureNumber(m_testSelectionTo));
        return;
    }

    if (!projectSession() || !projectSession()->currentNotation()) {
        return;
    }
    const INotationSelectionPtr selection = projectSession()->currentNotation()->interaction()->selection();
    if (!selection) {
        return;
    }
    applyRange(displayedMeasureNumber(selection->startMeasureBase()),
               displayedMeasureNumber(selection->endMeasureBase()));
}

void PracticeEmphasisPanelModel::clearMeasureRange()
{
    PracticeEmphasisData* emphasis = data();
    if (emphasis) {
        emphasis->setRange(0, 0);
    }
}

void PracticeEmphasisPanelModel::addCustomPart()
{
    MasterScore* master = currentMaster();
    PracticeEmphasisData* emphasis = data();
    if (!master || !emphasis) {
        return;
    }
    emphasis->addCustomPart(master, {}, emphasis->document().editingPartId);
}

void PracticeEmphasisPanelModel::deleteEditingPart()
{
    PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return;
    }
    emphasis->deletePart(emphasis->document().editingPartId);
}

void PracticeEmphasisPanelModel::resetEditingPartEdits()
{
    PracticeEmphasisData* emphasis = data();
    if (!emphasis) {
        return;
    }
    emphasis->resetPartEdits(emphasis->document().editingPartId);
}
