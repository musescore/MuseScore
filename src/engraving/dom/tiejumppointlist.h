#pragma once
#include "global/types/string.h"

using muse::String;
namespace mu::engraving {
class Note;
class Tie;
class TieJumpPointList;

// Items of TieJumpPointList
// Stores information about an endpoint
class TieJumpPoint
{
public:
    TieJumpPoint(Note* note, bool active, int idx, bool followingNote);
    TieJumpPoint() {}

    Note* note() const { return m_note; }
    Tie* endTie() const;
    bool followingNote() const { return m_followingNote; }
    const String& id() const { return m_id; }
    bool active() const { return m_active; }
    void setActive(bool v) { m_active = v; }
    void undoSetActive(bool v);
    void setJumpPointList(TieJumpPointList* jumpPointList) { m_jumpPointList = jumpPointList; }
    TieJumpPointList* jumpPointList() { return m_jumpPointList; }

    const String menuTitle() const;

private:
    String precedingJumpItemName() const;
    Note* m_note = nullptr;
    bool m_active = false;
    String m_id;
    TieJumpPointList* m_jumpPointList = nullptr;
    bool m_followingNote = false;
};

// Created & deleted by Note
// Manages adding and removing ties from endpoints
class TieJumpPointList
{
public:
    TieJumpPointList() = default;
    ~TieJumpPointList();

    void add(TieJumpPoint* item);
    void clear();
    size_t size() const { return m_jumpPoints.size(); }
    bool empty() const { return m_jumpPoints.empty(); }

    void setStartTie(Tie* startTie) { m_startTie = startTie; }
    Tie* startTie() const { return m_startTie; }

    TieJumpPoint* findJumpPoint(const String& id);
    void toggleJumpPoint(const String& id);

    void undoAddTieToScore(TieJumpPoint* jumpPoint);
    void undoRemoveTieFromScore(TieJumpPoint* jumpPoint);

    std::vector<TieJumpPoint*>::iterator begin() { return m_jumpPoints.begin(); }
    std::vector<TieJumpPoint*>::const_iterator begin() const { return m_jumpPoints.begin(); }
    std::vector<TieJumpPoint*>::iterator end() { return m_jumpPoints.end(); }
    std::vector<TieJumpPoint*>::const_iterator end() const { return m_jumpPoints.end(); }

private:
    std::vector<TieJumpPoint*> m_jumpPoints;
    Tie* m_startTie = nullptr;
};
}
