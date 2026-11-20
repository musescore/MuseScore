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
#pragma once

#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "global/async/channel.h"
#include "global/types/bytearray.h"

namespace mu::engraving {
class Note;
class Score;

//! Practice-track emphasis stored beside the score, never in notation properties.
//!
//! Each part is a rule (builtin SATB, or an optional parent) plus its own added and
//! removed notes. Membership is parent-or-rule notes, plus additions, minus removals.
//! Checking a part only decides whether its members are loud. Loudness is the union of
//! the checked parts. A single notehead is claimed by both builtin parts of that voice
//! (written unison). The top note of a chord belongs to the "1" part and every lower
//! notehead belongs to the "2" part.
class PracticeEmphasisData
{
public:
    static constexpr float DEFAULT_EMPHASIZED_DB = 9.f;
    static constexpr float DEFAULT_OTHER_DB = -15.f;
    static constexpr float SOLO_MUTE_DB = -60.f;
    static constexpr int CURRENT_VERSION = 3;

    enum class Role {
        Any,
        Top,
        Lower
    };

    // EditedAdded uses the same note color as Edited. NotationPainting draws a
    // screen-only highlighter behind those noteheads when highlight mode is on.
    enum class Highlight {
        None,
        Edited,
        EditedAdded,
        OtherLoud
    };

    struct PartRule {
        std::string id;
        std::string name;
        std::string staffEid;
        int staffIndex = 0;
        int voice = 0;
        Role role = Role::Top;
        bool selected = false;
        bool builtin = false;
        std::string parentId;
        std::vector<std::string> added;
        std::vector<std::string> removed;

        bool operator==(const PartRule& other) const;
    };

    struct Document {
        int version = CURRENT_VERSION;
        bool playbackActive = false;
        bool emphasisMode = false;
        float emphasizedDb = DEFAULT_EMPHASIZED_DB;
        float otherDb = DEFAULT_OTHER_DB;
        int fromMeasure = 0;
        int toMeasure = 0;
        std::string editingPartId;
        bool solo = false;
        std::vector<PartRule> parts;

        bool operator==(const Document& other) const;
    };

    static std::vector<PartRule> templatesForStaffCount(int staffCount);

    const Document& document() const { return m_document; }
    void setDocument(Document document, bool notifyListeners = true);

    bool isDefault() const;
    muse::ByteArray toJson() const;
    bool fromJson(const muse::ByteArray& json);

    void setPlaybackActive(bool active);
    void setEmphasisMode(bool enabled);
    void setGains(float emphasizedDb, float otherDb);
    void setRange(int fromMeasure, int toMeasure);
    void setPartSelected(const Score* score, const std::string& id, bool selected);
    void setEditingPartId(const Score* score, const std::string& id);
    void setSolo(bool enabled);
    void setPartName(const std::string& id, const std::string& name);
    void setPartParent(const Score* score, const std::string& id, const std::string& parentId);
    std::string addCustomPart(const Score* score, const std::string& name = {}, const std::string& parentId = {});
    bool deletePart(const std::string& id);
    void resetPartEdits(const std::string& id);
    bool copyFromPart(const Score* score, const std::string& targetId, const std::string& sourceId);

    //! Parent dropdown values: empty string first ("(none)"), then every visible part
    //! that would not create a parent cycle. Index 0 is always none.
    std::vector<std::string> parentOptionIds(const Score* score, const std::string& childId) const;
    int parentOptionIndex(const Score* score, const std::string& childId, const std::string& parentId) const;

    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();

    bool partSelected(const std::string& id) const;
    bool partContains(const Note* note, const std::string& partId) const;
    bool isEmphasized(const Note* note) const;
    Highlight noteHighlight(const Note* note) const;
    bool playbackEngaged() const;
    float otherGainDb() const;
    void editNote(Note* note);

    //! Staff/voice pairs that contain at least one emphasized note.
    std::vector<std::pair<int, int> > emphasizedVoices(const Score* score) const;

    muse::async::Channel<> changed() const { return m_changed; }

private:
    const PartRule* findPart(const std::string& id) const;
    PartRule* findPart(const std::string& id);
    PartRule* ensurePart(const Score* score, const std::string& id);
    std::vector<PartRule> visibleParts(const Score* score) const;
    std::optional<PartRule> resolvePart(const Score* score, const std::string& id) const;

    bool ruleMatches(const Note* note, const PartRule& part) const;
    bool baseContains(const PartRule& part, const Note* note, std::unordered_set<std::string>& visiting) const;
    bool partContains(const PartRule& part, const Note* note, std::unordered_set<std::string>& visiting) const;
    bool isInCheckedUnion(const Note* note) const;
    bool noteManuallyAdded(const Note* note, const std::string& partId) const;
    int resolvedStaffIndex(const Score* score, const PartRule& part) const;
    int sourceStaffIndex(const Score* score, const PartRule& part) const;
    int sourceStaffIndex(const Score* score, const PartRule& part, std::unordered_set<std::string>& visiting) const;
    std::string choosePartForStaff(const Score* score, int staffIndex, const std::string& currentId) const;
    bool wouldCreateCycle(const std::string& childId, const std::string& parentId) const;
    void bindStaffEid(const Score* score, PartRule& part) const;
    void toggleMembership(PartRule& part, Note* note);
    void resetHistory();
    void commitEdit(const Document& before, const std::string& coalesceKey = {});
    void applyNonUndoableChange();
    void projectNonUndoableFields(Document& snap) const;
    void notifyListeners();

    Document m_document;
    std::vector<Document> m_history;
    size_t m_historyIndex = 0;
    std::string m_coalesceKey;
    bool m_applyingHistory = false;
    muse::async::Channel<> m_changed;
};
}
