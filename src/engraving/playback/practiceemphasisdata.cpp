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

#include "practiceemphasisdata.h"

#include <algorithm>
#include <functional>
#include <unordered_map>

#include "global/serialization/json.h"

#include "dom/chord.h"
#include "dom/engravingitem.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/tie.h"

#include "editing/editpracticeemphasis.h"
#include "editing/transaction/undostack.h"

#include "log.h"

using namespace mu::engraving;

namespace {
constexpr const char* GENERATOR = "popatr-practice-emphasis";

bool isBuiltinPartId(const std::string& id)
{
    return id == "S1" || id == "S2" || id == "A1" || id == "A2"
           || id == "T1" || id == "T2" || id == "B1" || id == "B2";
}

const PracticeEmphasisData::PartRule* findRuleById(const std::vector<PracticeEmphasisData::PartRule>& parts,
                                                   const std::string& id)
{
    for (const PracticeEmphasisData::PartRule& part : parts) {
        if (part.id == id) {
            return &part;
        }
    }
    return nullptr;
}

const char* roleToString(PracticeEmphasisData::Role role)
{
    switch (role) {
    case PracticeEmphasisData::Role::Any:
        return "any";
    case PracticeEmphasisData::Role::Lower:
        return "lower";
    case PracticeEmphasisData::Role::Top:
        break;
    }
    return "top";
}

PracticeEmphasisData::Role roleFromString(const muse::String& text)
{
    if (text == u"lower") {
        return PracticeEmphasisData::Role::Lower;
    }
    if (text == u"any") {
        return PracticeEmphasisData::Role::Any;
    }
    return PracticeEmphasisData::Role::Top;
}

int displayedMeasureNumber(const Measure* measure)
{
    if (!measure) {
        return 0;
    }
    return measure->measureNumber() + measure->measureNumberOffset() + 1;
}

bool samePitchIsTop(const Chord* chord, const Note* note)
{
    const std::vector<Note*>& notes = chord->notes();
    if (notes.size() <= 1) {
        return true;
    }

    int bestPitch = notes.front()->pitch();
    size_t topIndex = 0;
    size_t noteIndex = 0;
    for (size_t i = 0; i < notes.size(); ++i) {
        if (notes[i]->pitch() > bestPitch) {
            bestPitch = notes[i]->pitch();
            topIndex = i;
        }
        if (notes[i] == note) {
            noteIndex = i;
        }
    }
    return noteIndex == topIndex;
}

void eachTiedNote(Note* note, const std::function<void(Note*)>& visit)
{
    if (!note) {
        return;
    }

    Note* start = note;
    while (Tie* back = start->tieBack()) {
        Note* previous = back->startNote();
        if (!previous || previous == start) {
            break;
        }
        start = previous;
    }

    Note* current = start;
    while (current) {
        visit(current);
        Tie* forward = current->tieFor();
        Note* next = forward ? forward->endNote() : nullptr;
        if (!next || next == current) {
            break;
        }
        current = next;
    }
}

void visitChordNotes(const Chord* chord, const std::function<void(const Note*)>& visit)
{
    if (!chord) {
        return;
    }
    for (const Note* note : chord->notes()) {
        visit(note);
    }
    for (const Chord* grace : chord->graceNotes()) {
        visitChordNotes(grace, visit);
    }
}

PracticeEmphasisData::PartRule makePart(const char* id, const char* name, int staff, int voice, PracticeEmphasisData::Role role)
{
    PracticeEmphasisData::PartRule part;
    part.id = id;
    part.name = name;
    part.staffIndex = staff;
    part.voice = voice;
    part.role = role;
    part.builtin = true;
    return part;
}

std::string noteEid(const Note* note)
{
    if (!note || !note->eid().isValid()) {
        return {};
    }
    return note->eid().toStdString();
}

std::string assignNoteEid(Note* note)
{
    if (!note) {
        return {};
    }
    if (!note->eid().isValid()) {
        note->assignNewEID();
    }
    return note->eid().toStdString();
}

bool containsEid(const std::vector<std::string>& list, const std::string& eid)
{
    return !eid.empty() && std::binary_search(list.begin(), list.end(), eid);
}

void insertEid(std::vector<std::string>& list, const std::string& eid)
{
    if (eid.empty()) {
        return;
    }
    const auto it = std::lower_bound(list.begin(), list.end(), eid);
    if (it == list.end() || *it != eid) {
        list.insert(it, eid);
    }
}

void eraseEid(std::vector<std::string>& list, const std::string& eid)
{
    if (eid.empty()) {
        return;
    }
    const auto it = std::lower_bound(list.begin(), list.end(), eid);
    if (it != list.end() && *it == eid) {
        list.erase(it);
    }
}

void normalizeEidList(std::vector<std::string>& list)
{
    list.erase(std::remove_if(list.begin(), list.end(), [](const std::string& value) {
        return value.empty();
    }), list.end());
    std::sort(list.begin(), list.end());
    list.erase(std::unique(list.begin(), list.end()), list.end());
}

std::vector<std::string> readEidArray(const muse::JsonObject& item, const char* key)
{
    std::vector<std::string> values;
    const muse::JsonValue value = item.value(key);
    if (!value.isArray()) {
        return values;
    }
    const muse::JsonArray array = value.toArray();
    for (size_t i = 0; i < array.size(); ++i) {
        const std::string eid = array.at(i).toStdString();
        if (!eid.empty()) {
            values.push_back(eid);
        }
    }
    normalizeEidList(values);
    return values;
}

const Note* masterNote(const Note* note)
{
    if (!note) {
        return nullptr;
    }
    const Score* master = note->score() ? note->score()->masterScore() : nullptr;
    if (!master || note->score() == master) {
        return note;
    }
    const EngravingItem* linked = note->findLinkedInScore(master);
    return (linked && linked->isNote()) ? toNote(linked) : note;
}

Note* writableMasterNote(Note* note)
{
    return const_cast<Note*>(masterNote(note));
}

const std::vector<PracticeEmphasisData::PartRule>& cachedTemplates(int staffCount)
{
    static std::unordered_map<int, std::vector<PracticeEmphasisData::PartRule> > cache;
    auto it = cache.find(staffCount);
    if (it == cache.end()) {
        it = cache.emplace(staffCount, PracticeEmphasisData::templatesForStaffCount(staffCount)).first;
    }
    return it->second;
}

muse::JsonArray writeEidArray(const std::vector<std::string>& values)
{
    muse::JsonArray array;
    for (const std::string& eid : values) {
        array.append(eid);
    }
    return array;
}
}

bool PracticeEmphasisData::PartRule::operator==(const PartRule& other) const
{
    return id == other.id
           && name == other.name
           && staffEid == other.staffEid
           && staffIndex == other.staffIndex
           && voice == other.voice
           && role == other.role
           && selected == other.selected
           && builtin == other.builtin
           && parentId == other.parentId
           && added == other.added
           && removed == other.removed;
}

bool PracticeEmphasisData::Document::operator==(const Document& other) const
{
    return version == other.version
           && playbackActive == other.playbackActive
           && emphasisMode == other.emphasisMode
           && emphasizedDb == other.emphasizedDb
           && otherDb == other.otherDb
           && fromMeasure == other.fromMeasure
           && toMeasure == other.toMeasure
           && editingPartId == other.editingPartId
           && solo == other.solo
           && parts == other.parts;
}

std::vector<PracticeEmphasisData::PartRule> PracticeEmphasisData::templatesForStaffCount(int staffCount)
{
    std::vector<PartRule> parts;
    if (staffCount <= 0) {
        return parts;
    }

    if (staffCount >= 4) {
        parts.push_back(makePart("S1", "1st soprano", 0, 0, Role::Top));
        parts.push_back(makePart("S2", "2nd soprano", 0, 0, Role::Lower));
        parts.push_back(makePart("A1", "1st alto", 1, 0, Role::Top));
        parts.push_back(makePart("A2", "2nd alto", 1, 0, Role::Lower));
        parts.push_back(makePart("T1", "1st tenor", 2, 0, Role::Top));
        parts.push_back(makePart("T2", "2nd tenor", 2, 0, Role::Lower));
        parts.push_back(makePart("B1", "1st bass", 3, 0, Role::Top));
        parts.push_back(makePart("B2", "2nd bass", 3, 0, Role::Lower));
        return parts;
    }

    if (staffCount == 1) {
        parts.push_back(makePart("S1", "1st soprano", 0, 0, Role::Top));
        parts.push_back(makePart("S2", "2nd soprano", 0, 0, Role::Lower));
        parts.push_back(makePart("A1", "1st alto", 0, 1, Role::Top));
        parts.push_back(makePart("A2", "2nd alto", 0, 1, Role::Lower));
        parts.push_back(makePart("T1", "1st tenor", 0, 2, Role::Top));
        parts.push_back(makePart("T2", "2nd tenor", 0, 2, Role::Lower));
        parts.push_back(makePart("B1", "1st bass", 0, 3, Role::Top));
        parts.push_back(makePart("B2", "2nd bass", 0, 3, Role::Lower));
        return parts;
    }

    // Closed SATB: soprano/alto share the upper staff, tenor/bass the lower.
    parts.push_back(makePart("S1", "1st soprano", 0, 0, Role::Top));
    parts.push_back(makePart("S2", "2nd soprano", 0, 0, Role::Lower));
    parts.push_back(makePart("A1", "1st alto", 0, 1, Role::Top));
    parts.push_back(makePart("A2", "2nd alto", 0, 1, Role::Lower));
    parts.push_back(makePart("T1", "1st tenor", 1, 0, Role::Top));
    parts.push_back(makePart("T2", "2nd tenor", 1, 0, Role::Lower));
    parts.push_back(makePart("B1", "1st bass", 1, 1, Role::Top));
    parts.push_back(makePart("B2", "2nd bass", 1, 1, Role::Lower));
    return parts;
}

void PracticeEmphasisData::setDocument(Document document, bool notify)
{
    m_document = std::move(document);
    m_unreadableSidecar = {};
    if (notify) {
        notifyListeners();
    }
}

void PracticeEmphasisData::applyUndoableSnapshot(Document snapshot)
{
    projectNonUndoableFields(snapshot);
    m_document = std::move(snapshot);
    m_unreadableSidecar = {};
    notifyListeners();
}

bool PracticeEmphasisData::isDefault() const
{
    return m_unreadableSidecar.empty()
           && !m_document.playbackActive
           && !m_document.emphasisMode
           && m_document.emphasizedDb == DEFAULT_EMPHASIZED_DB
           && m_document.otherDb == DEFAULT_OTHER_DB
           && m_document.fromMeasure == 0
           && m_document.toMeasure == 0
           && m_document.editingPartId.empty()
           && !m_document.solo
           && m_document.parts.empty();
}

muse::ByteArray PracticeEmphasisData::toJson() const
{
    if (!m_unreadableSidecar.empty()
        && m_document == Document()) {
        return m_unreadableSidecar;
    }

    muse::JsonObject root;
    root.set("version", CURRENT_VERSION);
    root.set("generator", GENERATOR);
    root.set("playbackActive", m_document.playbackActive);
    root.set("emphasisMode", m_document.emphasisMode);
    root.set("emphasizedGainDb", static_cast<double>(m_document.emphasizedDb));
    root.set("otherGainDb", static_cast<double>(m_document.otherDb));
    root.set("otherMode", "attenuate");
    root.set("fromMeasure", m_document.fromMeasure);
    root.set("toMeasure", m_document.toMeasure);
    root.set("editingPartId", m_document.editingPartId);
    root.set("solo", m_document.solo);

    muse::JsonArray parts;
    for (const PartRule& part : m_document.parts) {
        muse::JsonObject item;
        item.set("id", part.id);
        item.set("name", part.name);
        item.set("staffEid", part.staffEid);
        item.set("staffIndex", part.staffIndex);
        item.set("voice", part.voice);
        item.set("role", roleToString(part.role));
        item.set("selected", part.selected);
        item.set("builtin", part.builtin);
        item.set("parentId", part.parentId);
        item.set("added", writeEidArray(part.added));
        item.set("removed", writeEidArray(part.removed));
        parts.append(item);
    }
    root.set("parts", parts);

    return muse::JsonDocument(root).toJson();
}

bool PracticeEmphasisData::fromJson(const muse::ByteArray& json)
{
    if (json.empty()) {
        m_document = Document();
        m_unreadableSidecar = {};
        notifyListeners();
        return true;
    }

    std::string error;
    const muse::JsonDocument doc = muse::JsonDocument::fromJson(json, &error);
    if (!error.empty() || !doc.isObject()) {
        LOGE() << "Ignoring practice-emphasis.json: " << error;
        if (m_document == Document()) {
            m_unreadableSidecar = json;
        }
        return false;
    }

    const muse::JsonObject root = doc.rootObject();
    Document next;
    next.version = CURRENT_VERSION;
    next.playbackActive = root.value("playbackActive").toBool();
    next.emphasisMode = root.value("emphasisMode").toBool();
    next.emphasizedDb = static_cast<float>(root.contains("emphasizedGainDb")
                                           ? root.value("emphasizedGainDb").toDouble()
                                           : static_cast<double>(DEFAULT_EMPHASIZED_DB));
    next.otherDb = static_cast<float>(root.contains("otherGainDb")
                                      ? root.value("otherGainDb").toDouble()
                                      : static_cast<double>(DEFAULT_OTHER_DB));
    next.fromMeasure = std::max(0, root.value("fromMeasure").toInt());
    next.toMeasure = std::max(0, root.value("toMeasure").toInt());
    next.editingPartId = root.value("editingPartId").toStdString();
    next.solo = root.value("solo").toBool();

    const muse::JsonValue partsValue = root.value("parts");
    const muse::JsonArray parts = partsValue.isArray() ? partsValue.toArray() : muse::JsonArray();
    for (size_t i = 0; i < parts.size(); ++i) {
        const muse::JsonObject item = parts.at(i).toObject();
        PartRule part;
        part.id = item.value("id").toStdString();
        part.name = item.value("name").toStdString();
        part.staffEid = item.value("staffEid").toStdString();
        part.staffIndex = item.value("staffIndex").toInt();
        part.voice = item.value("voice").toInt();
        part.role = roleFromString(item.value("role").toString());
        part.selected = item.value("selected").toBool();
        if (item.contains("builtin")) {
            part.builtin = item.value("builtin").toBool();
        } else {
            part.builtin = isBuiltinPartId(part.id);
        }
        part.parentId = item.value("parentId").toStdString();
        if (part.builtin) {
            part.parentId.clear();
        }
        part.added = readEidArray(item, "added");
        part.removed = readEidArray(item, "removed");
        if (part.id.empty() || part.voice < 0 || part.voice > 3 || part.staffIndex < 0) {
            continue;
        }
        next.parts.push_back(std::move(part));
    }

    m_document = std::move(next);
    m_unreadableSidecar = {};
    notifyListeners();
    return true;
}

void PracticeEmphasisData::setPlaybackActive(bool active)
{
    if (m_document.playbackActive == active) {
        return;
    }
    m_document.playbackActive = active;
    applyNonUndoableChange();
}

void PracticeEmphasisData::setEmphasisMode(bool enabled)
{
    if (m_document.emphasisMode == enabled) {
        return;
    }
    m_document.emphasisMode = enabled;
    applyNonUndoableChange();
}

void PracticeEmphasisData::setGains(float emphasizedDb, float otherDb)
{
    if (m_document.emphasizedDb == emphasizedDb && m_document.otherDb == otherDb) {
        return;
    }
    m_document.emphasizedDb = emphasizedDb;
    m_document.otherDb = otherDb;
    applyNonUndoableChange();
}

void PracticeEmphasisData::setRange(int fromMeasure, int toMeasure)
{
    fromMeasure = std::max(0, fromMeasure);
    toMeasure = std::max(0, toMeasure);
    if (m_document.fromMeasure == fromMeasure && m_document.toMeasure == toMeasure) {
        return;
    }
    m_document.fromMeasure = fromMeasure;
    m_document.toMeasure = toMeasure;
    applyNonUndoableChange();
}

const PracticeEmphasisData::PartRule* PracticeEmphasisData::findPart(const std::string& id) const
{
    for (const PartRule& part : m_document.parts) {
        if (part.id == id) {
            return &part;
        }
    }
    return nullptr;
}

PracticeEmphasisData::PartRule* PracticeEmphasisData::findPart(const std::string& id)
{
    return const_cast<PartRule*>(static_cast<const PracticeEmphasisData*>(this)->findPart(id));
}

void PracticeEmphasisData::bindStaffEid(const Score* score, PartRule& part) const
{
    if (!score || part.staffIndex < 0) {
        return;
    }
    if (const Staff* staff = score->staff(static_cast<size_t>(part.staffIndex))) {
        if (!staff->eid().isValid()) {
            staff->assignNewEID();
        }
        part.staffEid = staff->eid().toStdString();
    }
}

PracticeEmphasisData::PartRule* PracticeEmphasisData::ensurePart(const Score* score, const std::string& id)
{
    if (id.empty()) {
        return nullptr;
    }
    if (PartRule* existing = findPart(id)) {
        return existing;
    }
    if (!score) {
        return nullptr;
    }

    const std::vector<PartRule> templates = templatesForStaffCount(static_cast<int>(score->nstaves()));
    for (const PartRule& candidate : templates) {
        if (candidate.id != id) {
            continue;
        }
        PartRule created = candidate;
        bindStaffEid(score, created);
        m_document.parts.push_back(std::move(created));
        return &m_document.parts.back();
    }
    return nullptr;
}

std::vector<PracticeEmphasisData::PartRule> PracticeEmphasisData::visibleParts(const Score* score) const
{
    const int staves = score ? static_cast<int>(score->nstaves()) : 0;
    std::vector<PartRule> result = templatesForStaffCount(staves);
    std::unordered_set<std::string> seen;
    for (PartRule& part : result) {
        seen.insert(part.id);
        if (const PartRule* existing = findPart(part.id)) {
            part = *existing;
        }
    }
    for (const PartRule& part : m_document.parts) {
        if (seen.insert(part.id).second) {
            result.push_back(part);
        }
    }
    return result;
}

const PracticeEmphasisData::PartRule* PracticeEmphasisData::resolvePart(const Score* score, const std::string& id) const
{
    if (const PartRule* existing = findPart(id)) {
        return existing;
    }
    if (!score || id.empty()) {
        return nullptr;
    }
    for (const PartRule& candidate : cachedTemplates(static_cast<int>(score->nstaves()))) {
        if (candidate.id == id) {
            return &candidate;
        }
    }
    return nullptr;
}

void PracticeEmphasisData::setPartSelected(const Score* score, const std::string& id, bool selected)
{
    if (!score || id.empty()) {
        return;
    }

    PartRule* existing = ensurePart(score, id);
    if (!existing) {
        existing = findPart(id);
    }
    if (!existing) {
        return;
    }
    if (existing->selected == selected) {
        return;
    }
    existing->selected = selected;
    if (selected && m_document.editingPartId.empty()) {
        m_document.editingPartId = id;
    }
    applyNonUndoableChange();
}

void PracticeEmphasisData::setEditingPartId(const Score* score, const std::string& id)
{
    if (m_document.editingPartId == id) {
        return;
    }
    if (!id.empty() && !ensurePart(score, id) && !findPart(id)) {
        return;
    }
    if (m_document.editingPartId == id) {
        return;
    }
    m_document.editingPartId = id;
    applyNonUndoableChange();
}

void PracticeEmphasisData::setSolo(bool enabled)
{
    if (m_document.solo == enabled) {
        return;
    }
    m_document.solo = enabled;
    applyNonUndoableChange();
}

void PracticeEmphasisData::setPartName(const std::string& id, const std::string& name)
{
    PartRule* part = findPart(id);
    if (!part || part->builtin || part->name == name) {
        return;
    }
    part->name = name;
    applyNonUndoableChange();
}

bool PracticeEmphasisData::wouldCreateCycle(const std::string& childId, const std::string& parentId) const
{
    if (childId.empty() || parentId.empty()) {
        return false;
    }
    if (childId == parentId) {
        return true;
    }
    std::unordered_set<std::string> seen;
    std::string current = parentId;
    while (!current.empty()) {
        if (current == childId || !seen.insert(current).second) {
            return true;
        }
        const PartRule* part = findPart(current);
        if (!part) {
            break;
        }
        current = part->parentId;
    }
    return false;
}

void PracticeEmphasisData::setPartParent(const Score* score, const std::string& id, const std::string& parentId)
{
    PartRule* part = findPart(id);
    if (!part || part->builtin) {
        return;
    }
    if (parentId == part->parentId) {
        return;
    }
    const Document before = m_document;
    if (!parentId.empty()) {
        if (wouldCreateCycle(id, parentId)) {
            return;
        }
        if (!ensurePart(score, parentId) && !findPart(parentId)) {
            return;
        }
    }
    // ensurePart may grow parts and invalidate the earlier pointer.
    part = findPart(id);
    if (!part) {
        return;
    }
    part->parentId = parentId;
    commitToScoreUndo(score, before, muse::TranslatableString("undoableAction", "Change practice emphasis parent"));
}

std::vector<std::string> PracticeEmphasisData::parentOptionIds(const Score* score, const std::string& childId) const
{
    std::vector<std::string> ids;
    ids.emplace_back();
    for (const PartRule& part : visibleParts(score)) {
        if (wouldCreateCycle(childId, part.id)) {
            continue;
        }
        ids.push_back(part.id);
    }
    return ids;
}

int PracticeEmphasisData::parentOptionIndex(const Score* score, const std::string& childId,
                                            const std::string& parentId) const
{
    const std::vector<std::string> ids = parentOptionIds(score, childId);
    for (size_t i = 0; i < ids.size(); ++i) {
        if (ids[i] == parentId) {
            return static_cast<int>(i);
        }
    }
    return 0;
}

std::string PracticeEmphasisData::addCustomPart(const Score* score, const std::string& name, const std::string& parentId)
{
    int index = 1;
    std::string id = "custom-1";
    while (findPart(id) || isBuiltinPartId(id)) {
        ++index;
        id = "custom-" + std::to_string(index);
    }

    PartRule part;
    part.id = id;
    part.name = name.empty() ? ("Custom " + std::to_string(index)) : name;
    part.builtin = false;
    if (!parentId.empty() && parentId != id && !wouldCreateCycle(id, parentId)) {
        if (ensurePart(score, parentId) || findPart(parentId)) {
            part.parentId = parentId;
        }
    }
    m_document.parts.push_back(std::move(part));
    m_document.editingPartId = id;
    applyNonUndoableChange();
    return id;
}

bool PracticeEmphasisData::deletePart(const std::string& id)
{
    PartRule* part = findPart(id);
    if (!part || part->builtin) {
        return false;
    }

    m_document.parts.erase(std::remove_if(m_document.parts.begin(), m_document.parts.end(), [&](const PartRule& candidate) {
        return candidate.id == id;
    }), m_document.parts.end());

    for (PartRule& remaining : m_document.parts) {
        if (remaining.parentId == id) {
            remaining.parentId.clear();
        }
    }
    if (m_document.editingPartId == id) {
        m_document.editingPartId.clear();
        if (!m_document.parts.empty()) {
            m_document.editingPartId = m_document.parts.front().id;
        }
    }
    applyNonUndoableChange();
    return true;
}

void PracticeEmphasisData::resetPartEdits(const Score* score, const std::string& id)
{
    PartRule* part = findPart(id);
    if (!part || (part->added.empty() && part->removed.empty())) {
        return;
    }
    const Document before = m_document;
    part->added.clear();
    part->removed.clear();
    commitToScoreUndo(score, before, muse::TranslatableString("undoableAction", "Reset practice emphasis edits"));
}

bool PracticeEmphasisData::partSelected(const std::string& id) const
{
    const PartRule* part = findPart(id);
    return part && part->selected;
}

int PracticeEmphasisData::resolvedStaffIndex(const Score* score, const PartRule& part) const
{
    if (score && score->masterScore() && !part.staffEid.empty()) {
        const EID eid = EID::fromStdString(part.staffEid);
        const EngravingObject* object = score->masterScore()->eidRegister()->itemFromEID(eid);
        if (object && object->isStaff()) {
            return static_cast<int>(toStaff(object)->idx());
        }
    }
    return part.staffIndex;
}

int PracticeEmphasisData::sourceStaffIndex(const Score* score, const PartRule& part) const
{
    std::unordered_set<std::string> visiting;
    return sourceStaffIndex(score, part, visiting);
}

int PracticeEmphasisData::sourceStaffIndex(const Score* score, const PartRule& part, std::unordered_set<std::string>& visiting) const
{
    if (!visiting.insert(part.id).second) {
        return -1;
    }
    if (part.builtin) {
        return resolvedStaffIndex(score, part);
    }
    if (part.parentId.empty()) {
        return -1;
    }
    if (const PartRule* parent = resolvePart(score, part.parentId)) {
        return sourceStaffIndex(score, *parent, visiting);
    }
    return -1;
}

bool PracticeEmphasisData::ruleMatches(const Note* note, const PartRule& part) const
{
    if (!note || !note->chord() || !part.builtin) {
        return false;
    }
    if (static_cast<int>(note->voice()) != part.voice) {
        return false;
    }
    if (static_cast<int>(note->staffIdx()) != resolvedStaffIndex(note->score(), part)) {
        return false;
    }

    const Measure* measure = note->findMeasure();
    const int number = displayedMeasureNumber(measure);
    if (m_document.fromMeasure > 0 && number < m_document.fromMeasure) {
        return false;
    }
    if (m_document.toMeasure > 0 && number > m_document.toMeasure) {
        return false;
    }

    const Chord* chord = note->chord();
    const bool shared = chord->notes().size() <= 1;
    const bool top = samePitchIsTop(chord, note);
    switch (part.role) {
    case Role::Any:
        return true;
    case Role::Top:
        return shared || top;
    case Role::Lower:
        return shared || !top;
    }
    return false;
}

bool PracticeEmphasisData::baseContains(const PartRule& part, const Note* note, std::unordered_set<std::string>& visiting) const
{
    if (!part.parentId.empty()) {
        if (const PartRule* parent = resolvePart(note ? note->score() : nullptr, part.parentId)) {
            return partContains(*parent, note, visiting);
        }
        return false;
    }
    if (part.builtin) {
        return ruleMatches(note, part);
    }
    return false;
}

bool PracticeEmphasisData::partContains(const PartRule& part, const Note* note, std::unordered_set<std::string>& visiting) const
{
    if (!note || !visiting.insert(part.id).second) {
        return false;
    }

    const std::string eid = noteEid(note);
    if (containsEid(part.removed, eid)) {
        visiting.erase(part.id);
        return false;
    }
    if (containsEid(part.added, eid)) {
        visiting.erase(part.id);
        return true;
    }

    const bool inBase = baseContains(part, note, visiting);
    visiting.erase(part.id);
    return inBase;
}

bool PracticeEmphasisData::partContains(const Note* note, const std::string& partId) const
{
    note = masterNote(note);
    if (!note || partId.empty()) {
        return false;
    }
    const PartRule* part = resolvePart(note->score(), partId);
    if (!part) {
        return false;
    }
    std::unordered_set<std::string> visiting;
    return partContains(*part, note, visiting);
}

bool PracticeEmphasisData::isInCheckedUnion(const Note* note) const
{
    if (!note) {
        return false;
    }
    for (const PartRule& part : m_document.parts) {
        if (!part.selected) {
            continue;
        }
        std::unordered_set<std::string> visiting;
        if (partContains(part, note, visiting)) {
            return true;
        }
    }
    return false;
}

bool PracticeEmphasisData::noteManuallyAdded(const Note* note, const std::string& partId) const
{
    if (!note || partId.empty()) {
        return false;
    }
    const PartRule* part = findPart(partId);
    return part && containsEid(part->added, noteEid(note));
}

bool PracticeEmphasisData::isEmphasized(const Note* note) const
{
    note = masterNote(note);
    if (!note) {
        return false;
    }
    if (m_document.solo) {
        return !m_document.editingPartId.empty() && partContains(note, m_document.editingPartId);
    }
    return isInCheckedUnion(note);
}

PracticeEmphasisData::Highlight PracticeEmphasisData::noteHighlight(const Note* note) const
{
    note = masterNote(note);
    if (!note || !m_document.emphasisMode) {
        return Highlight::None;
    }
    if (!m_document.editingPartId.empty()) {
        if (noteManuallyAdded(note, m_document.editingPartId)) {
            return Highlight::EditedAdded;
        }
        if (partContains(note, m_document.editingPartId)) {
            return Highlight::Edited;
        }
    }
    if (isInCheckedUnion(note)) {
        return Highlight::OtherLoud;
    }
    return Highlight::None;
}

bool PracticeEmphasisData::playbackEngaged() const
{
    if (m_document.solo) {
        return !m_document.editingPartId.empty();
    }
    if (!m_document.playbackActive) {
        return false;
    }
    for (const PartRule& part : m_document.parts) {
        if (part.selected) {
            return true;
        }
    }
    return false;
}

float PracticeEmphasisData::otherGainDb() const
{
    return m_document.solo ? SOLO_MUTE_DB : m_document.otherDb;
}

std::string PracticeEmphasisData::choosePartForStaff(const Score* score, int staffIndex, const std::string& currentId) const
{
    std::vector<std::string> matchIds;
    std::string topId;
    bool currentMatches = false;
    for (const PartRule& part : visibleParts(score)) {
        if (sourceStaffIndex(score, part) != staffIndex) {
            continue;
        }
        matchIds.push_back(part.id);
        if (part.id == currentId) {
            currentMatches = true;
        }
        if (topId.empty() && part.builtin && part.role == Role::Top) {
            topId = part.id;
        }
    }
    if (matchIds.empty()) {
        return {};
    }
    if (currentMatches) {
        return currentId;
    }
    if (!topId.empty()) {
        return topId;
    }
    return matchIds.front();
}

void PracticeEmphasisData::toggleMembership(PartRule& part, Note* note)
{
    eachTiedNote(note, [&](Note* tied) {
        const std::string eid = assignNoteEid(tied);
        if (eid.empty()) {
            return;
        }

        std::unordered_set<std::string> visiting;
        const bool currently = partContains(part, tied, visiting);
        visiting.clear();
        const bool inBase = baseContains(part, tied, visiting);

        if (currently) {
            eraseEid(part.added, eid);
            if (inBase) {
                insertEid(part.removed, eid);
            }
        } else {
            eraseEid(part.removed, eid);
            if (!inBase) {
                insertEid(part.added, eid);
            }
        }
    });
}

void PracticeEmphasisData::editNote(Note* note)
{
    note = writableMasterNote(note);
    if (!note) {
        return;
    }
    Score* score = note->score() ? note->score()->masterScore() : nullptr;
    if (!score) {
        score = note->score();
    }
    if (!score) {
        return;
    }

    const int noteStaff = static_cast<int>(note->staffIdx());
    std::string targetId = m_document.editingPartId;
    int currentStaff = -1;
    if (const PartRule* current = resolvePart(score, targetId)) {
        currentStaff = sourceStaffIndex(score, *current);
    }

    if (currentStaff >= 0 && currentStaff != noteStaff) {
        const std::string switched = choosePartForStaff(score, noteStaff, targetId);
        if (switched.empty()) {
            return;
        }
        targetId = switched;
    } else if (targetId.empty()) {
        targetId = choosePartForStaff(score, noteStaff, {});
        if (targetId.empty()) {
            return;
        }
    }

    PartRule* part = ensurePart(score, targetId);
    if (!part) {
        part = findPart(targetId);
    }
    if (!part) {
        return;
    }

    const Document before = m_document;
    m_document.editingPartId = targetId;
    toggleMembership(*part, note);
    commitToScoreUndo(score, before, muse::TranslatableString("undoableAction", "Edit practice emphasis"));
}

std::vector<std::pair<int, int> > PracticeEmphasisData::emphasizedVoices(const Score* score) const
{
    std::vector<std::pair<int, int> > voices;
    if (!score) {
        return voices;
    }

    auto add = [&voices](int staff, int voice) {
        const std::pair<int, int> key { staff, voice };
        if (std::find(voices.begin(), voices.end(), key) == voices.end()) {
            voices.push_back(key);
        }
    };

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < score->ntracks(); ++track) {
                const EngravingItem* element = segment->element(track);
                if (!element || !element->isChord()) {
                    continue;
                }
                visitChordNotes(toChord(element), [&](const Note* note) {
                    if (isEmphasized(note)) {
                        add(static_cast<int>(note->staffIdx()), static_cast<int>(note->voice()));
                    }
                });
            }
        }
    }
    return voices;
}

void PracticeEmphasisData::projectNonUndoableFields(Document& snap) const
{
    snap.version = m_document.version;
    snap.playbackActive = m_document.playbackActive;
    snap.emphasisMode = m_document.emphasisMode;
    snap.emphasizedDb = m_document.emphasizedDb;
    snap.otherDb = m_document.otherDb;
    snap.fromMeasure = m_document.fromMeasure;
    snap.toMeasure = m_document.toMeasure;
    snap.solo = m_document.solo;
    snap.editingPartId = m_document.editingPartId;

    std::vector<PartRule> next;
    next.reserve(m_document.parts.size());
    for (const PartRule& live : m_document.parts) {
        PartRule part = live;
        if (const PartRule* old = findRuleById(snap.parts, live.id)) {
            part.parentId = old->parentId;
            part.added = old->added;
            part.removed = old->removed;
        } else if (part.builtin) {
            // ensurePart during this command must not keep the new membership
            // when the snapshot predates that part.
            part.added.clear();
            part.removed.clear();
        }
        if (!part.parentId.empty() && !isBuiltinPartId(part.parentId)
            && !findRuleById(m_document.parts, part.parentId)) {
            part.parentId.clear();
        }
        next.push_back(std::move(part));
    }
    snap.parts = std::move(next);
}

void PracticeEmphasisData::applyNonUndoableChange()
{
    m_unreadableSidecar = {};
    notifyListeners(true);
}

void PracticeEmphasisData::commitToScoreUndo(const Score* score, const Document& before,
                                             const muse::TranslatableString& actionName)
{
    if (before == m_document) {
        return;
    }

    Score* writable = const_cast<Score*>(score);
    MasterScore* master = writable ? writable->masterScore() : nullptr;
    if (!master || !master->undoStack()) {
        notifyListeners(true);
        return;
    }

    Document after = m_document;
    // flip() projects live UI onto the command snapshot. Reverting live to
    // `before` would wipe in-command fields such as editingPartId on the first
    // flip. Keep before's membership with after's UI so that first flip sticks.
    Document prepared = before;
    projectNonUndoableFields(prepared);
    m_document = std::move(prepared);
    const bool started = !master->undoStack()->hasActiveTransaction();
    if (started) {
        master->startCmd(actionName);
    }
    master->undo(new ChangePracticeEmphasis(master, std::move(after)));
    if (started) {
        master->endCmd();
    }
}

bool PracticeEmphasisData::copyFromPart(const Score* score, const std::string& targetId, const std::string& sourceId)
{
    if (!score || targetId.empty() || sourceId.empty() || targetId == sourceId) {
        return false;
    }
    PartRule* target = findPart(targetId);
    if (!target || target->builtin) {
        return false;
    }
    if (!resolvePart(score, sourceId)) {
        return false;
    }

    const Document before = m_document;
    std::vector<std::string> added;
    std::vector<std::string> removed;
    Score* writable = const_cast<Score*>(score);
    Score* source = writable->masterScore() ? writable->masterScore() : writable;
    for (Measure* measure = source->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < source->ntracks(); ++track) {
                EngravingItem* element = segment->element(track);
                if (!element || !element->isChord()) {
                    continue;
                }
                visitChordNotes(toChord(element), [&](const Note* note) {
                    Note* writableNote = writableMasterNote(const_cast<Note*>(note));
                    if (!writableNote) {
                        return;
                    }
                    const std::string eid = assignNoteEid(writableNote);
                    if (eid.empty()) {
                        return;
                    }
                    const bool inSource = partContains(writableNote, sourceId);
                    std::unordered_set<std::string> visiting;
                    const bool inBase = baseContains(*target, writableNote, visiting);
                    if (inSource && !inBase) {
                        added.push_back(eid);
                    } else if (!inSource && inBase) {
                        removed.push_back(eid);
                    }
                });
            }
        }
    }
    normalizeEidList(added);
    normalizeEidList(removed);
    if (target->added == added && target->removed == removed) {
        return false;
    }
    target->added = std::move(added);
    target->removed = std::move(removed);
    commitToScoreUndo(score, before, muse::TranslatableString("undoableAction", "Copy practice emphasis part"));
    return true;
}

void PracticeEmphasisData::notifyListeners(bool nonUndoable)
{
    m_changed.send();
    if (nonUndoable) {
        m_nonUndoableChanged.send();
    }
}
