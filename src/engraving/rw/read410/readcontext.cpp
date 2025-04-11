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

#include "readcontext.h"

#include "dom/beam.h"
#include "dom/linkedobjects.h"
#include "dom/score.h"
#include "dom/trill.h"
#include "dom/tuplet.h"
#include "dom/undo.h"

#include "connectorinforeader.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::read410;

ReadContext::ReadContext(Score* score)
    : m_score(score)
{
}

ReadContext::~ReadContext()
{
    if (!_connectors.empty() || !_pendingConnectors.empty()) {
        LOGD("XmlReader::~XmlReader: there are unpaired connectors left");
        for (auto& c : _connectors) {
            EngravingItem* conn = c->releaseConnector();
            if (conn && !conn->isTuplet()) { // tuplets are added to score even when not finished
                delete conn;
            }
        }
        for (auto& c : _pendingConnectors) {
            delete c->releaseConnector();
        }
    }
}

void ReadContext::setScore(Score* score)
{
    m_score = score;
}

Score* ReadContext::score() const
{
    return m_score;
}

bool ReadContext::isMasterScore() const
{
    return m_score->isMaster();
}

void ReadContext::setMasterCtx(ReadContext* ctx)
{
    m_masterCtx = ctx;
}

ReadContext* ReadContext::masterCtx()
{
    if (!m_score) {
        return this;
    }

    if (m_score->isMaster()) {
        return this;
    }

    DO_ASSERT(m_masterCtx);

    return m_masterCtx;
}

const ReadContext* ReadContext::masterCtx() const
{
    if (!m_score) {
        return this;
    }

    if (m_score->isMaster()) {
        return this;
    }

    DO_ASSERT(m_masterCtx);

    return m_masterCtx;
}

const MStyle& ReadContext::style() const
{
    return score()->style();
}

std::shared_ptr<IEngravingFontsProvider> ReadContext::engravingFonts() const
{
    return score()->engravingFonts();
}

String ReadContext::mscoreVersion() const
{
    return m_score->mscoreVersion();
}

int ReadContext::mscVersion() const
{
    return m_score->mscVersion();
}

int ReadContext::fileDivision() const
{
    return m_score->fileDivision();
}

int ReadContext::fileDivision(int t) const
{
    return m_score->fileDivision(t);
}

double ReadContext::spatium() const
{
    return m_score->style().spatium();
}

void ReadContext::setSpatium(double v)
{
    m_score->style().set(Sid::spatium, v);
}

compat::DummyElement* ReadContext::dummy() const
{
    return m_score->dummy();
}

Staff* ReadContext::staff(int n)
{
    return m_score->staff(n);
}

void ReadContext::appendStaff(Staff* staff)
{
    m_score->appendStaff(staff);
}

void ReadContext::addSpanner(Spanner* s)
{
    m_score->addSpanner(s);
}

bool ReadContext::undoStackActive() const
{
    return m_score->undoStack()->hasActiveCommand();
}

bool ReadContext::isSameScore(const EngravingObject* obj) const
{
    return obj->score() == m_score;
}

rw::ReadLinks ReadContext::readLinks() const
{
    return doReadLinks();
}

rw::ReadLinks ReadContext::doReadLinks() const
{
    rw::ReadLinks l;
    l.linksIndexer = m_linksIndexer;
    l.staffLinkedElements = m_staffLinkedElements;
    return l;
}

void ReadContext::initLinks(const rw::ReadLinks& l)
{
    doInitLinks(l);
}

void ReadContext::doInitLinks(const rw::ReadLinks& l)
{
    m_linksIndexer = l.linksIndexer;
    m_staffLinkedElements = l.staffLinkedElements;
}

void ReadContext::addLink(Staff* staff, LinkedObjects* link, const Location& location)
{
    doAddLink(staff, link, location);
}

void ReadContext::doAddLink(Staff* staff, LinkedObjects* link, const Location& location)
{
    int staffIndex = static_cast<int>(staff->idx());
    const bool isMasterScore = staff->score()->isMaster();
    if (!isMasterScore) {
        staffIndex *= -1;
    }

    std::vector<std::pair<LinkedObjects*, Location> >& staffLinks = m_staffLinkedElements[staffIndex];
    if (!isMasterScore) {
        if (!staffLinks.empty()
            && (link->mainElement()->score() != staffLinks.front().first->mainElement()->score())
            ) {
            staffLinks.clear();
        }
    }

    m_linksIndexer.assignLocalIndex(location);
    staffLinks.push_back(std::make_pair(link, location));
}

LinkedObjects* ReadContext::getLink(bool isMasterScore, const Location& location, int localIndexDiff)
{
    return doGetLink(isMasterScore, location, localIndexDiff);
}

LinkedObjects* ReadContext::doGetLink(bool isMasterScore, const Location& location, int localIndexDiff)
{
    int staffIndex = location.staff();
    if (!isMasterScore) {
        staffIndex *= -1;
    }

    const int localIndex = m_linksIndexer.assignLocalIndex(location) + localIndexDiff;
    std::vector<std::pair<LinkedObjects*, Location> >& staffLinks = m_staffLinkedElements[staffIndex];

    if (!staffLinks.empty() && staffLinks.back().second == location) {
        // This element potentially affects local index for "main"
        // elements that may go afterwards at the same tick, so
        // append it to staffLinks as well.
        staffLinks.push_back(staffLinks.back()); // nothing should reference exactly this local index, so it shouldn't matter what to append
    }

    for (size_t i = 0; i < staffLinks.size(); ++i) {
        if (staffLinks[i].second == location) {
            if (localIndex == 0) {
                return staffLinks[i].first;
            }

            i += localIndex;
            if (i >= staffLinks.size()) {
                return nullptr;
            }

            if (staffLinks[i].second == location) {
                return staffLinks[i].first;
            }

            return nullptr;
        }
    }

    return nullptr;
}

std::map<int, std::vector<std::pair<LinkedObjects*, Location> > >& ReadContext::staffLinkedElements()
{
    return m_staffLinkedElements;
}

std::map<int, LinkedObjects*>& ReadContext::linkIds()
{
    return masterCtx()->_elinks;
}

Fraction ReadContext::rtick() const
{
    return _curMeasure ? _tick - _curMeasure->tick() : _tick;
}

void ReadContext::setTick(const Fraction& f)
{
    _tick = f.reduced();
    _intTick = _tick.ticks();
}

void ReadContext::incTick(const Fraction& f)
{
    _tick += f;
    _tick.reduce();
    _intTick += f.ticks();
}

Location ReadContext::location(bool forceAbsFrac) const
{
    return doLocation(forceAbsFrac);
}

Location ReadContext::doLocation(bool forceAbsFrac) const
{
    Location l = Location::absolute();
    doFillLocation(l, forceAbsFrac);
    return l;
}

//---------------------------------------------------------
//   fillLocation
//    fills location fields which have default values with
//    values relevant for the current reader's position.
//    When in paste mode (or forceAbsFrac is true) absolute
//    fraction values are used and measure number is set to
//    zero.
//---------------------------------------------------------

void ReadContext::fillLocation(Location& l, bool forceAbsFrac) const
{
    doFillLocation(l, forceAbsFrac);
}

void ReadContext::doFillLocation(Location& l, bool forceAbsFrac) const
{
    constexpr Location defaults = Location::absolute();
    const bool absFrac = (pasteMode() || forceAbsFrac);
    if (l.track() == defaults.track()) {
        l.setTrack(static_cast<int>(track()));
    }
    if (l.frac() == defaults.frac()) {
        l.setFrac(absFrac ? tick() : rtick());
    }
    if (l.measure() == defaults.measure()) {
        l.setMeasure(absFrac ? 0 : currentMeasureIndex());
    }
}

//---------------------------------------------------------
//   setLocation
//    sets a new reading location, taking into account its
//    type (absolute or relative).
//---------------------------------------------------------

void ReadContext::setLocation(const Location& l)
{
    doSetLocation(l);
}

void ReadContext::doSetLocation(const Location& l)
{
    if (l.isRelative()) {
        Location newLoc = l;
        newLoc.toAbsolute(location());
        int intTicks = l.frac().ticks();
        if (_tick == Fraction::fromTicks(_intTick + intTicks)) {
            _intTick += intTicks;
            setTrack(newLoc.track() - _trackOffset);
            return;
        }
        doSetLocation(newLoc);     // recursion
        return;
    }
    setTrack(l.track() - _trackOffset);
    setTick(l.frac() - _tickOffset);
    if (!pasteMode()) {
        assert(l.measure() == currentMeasureIndex());
        incTick(currentMeasure()->tick());
    }
}

void ReadContext::addBeam(Beam* s)
{
    _beams.insert_or_assign(s->id(), s);
}

void ReadContext::addTuplet(Tuplet* s)
{
    _tuplets.insert_or_assign(s->id(), s);
}

void ReadContext::checkTuplets()
{
    for (auto& p : tuplets()) {
        Tuplet* tuplet = p.second;
        if (tuplet->elements().empty()) {
            // this should not happen and is a sign of input file corruption
            LOGD("Measure:read(): empty tuplet id %d (%p), input file corrupted?", tuplet->id(), tuplet);
            delete tuplet;
        } else {
            //sort tuplet elements. Needed for nested tuplets #22537
            tuplet->sortElements();
            tuplet->sanitizeTuplet();
        }
    }
    // This requires a separate pass in case of nested tuplets that required sanitizing
    for (auto& p : tuplets()) {
        Tuplet* tuplet = p.second;
        tuplet->addMissingElements();
    }
}

void ReadContext::addSpanner(int id, Spanner* s)
{
    _spanner.push_back(std::pair<int, Spanner*>(id, s));
}

void ReadContext::removeSpanner(const Spanner* s)
{
    for (auto i : _spanner) {
        if (i.second == s) {
            muse::remove(_spanner, i);
            return;
        }
    }
}

Spanner* ReadContext::findSpanner(int id)
{
    for (auto i : _spanner) {
        if (i.first == id) {
            return i.second;
        }
    }
    return nullptr;
}

int ReadContext::spannerId(const Spanner* s)
{
    for (auto i : _spanner) {
        if (i.second == s) {
            return i.first;
        }
    }
    LOGD() << "spannerId not found";
    return -1;
}

const SpannerValues* ReadContext::spannerValues(int id) const
{
    for (const SpannerValues& v : _spannerValues) {
        if (v.spannerId == id) {
            return &v;
        }
    }
    return 0;
}

void ReadContext::removeConnector(const ConnectorInfoReader* c)
{
    while (c->prev()) {
        c = c->prev();
    }
    while (c) {
        ConnectorInfoReader* next = c->next();
        for (auto it = _connectors.begin(); it != _connectors.end(); ++it) {
            if (it->get() == c) {
                _connectors.erase(it);
                break;
            }
        }
        c = next;
    }
}

void ReadContext::addConnectorInfoLater(std::shared_ptr<ConnectorInfoReader> c)
{
    _pendingConnectors.push_back(c);
}

void ReadContext::checkConnectors()
{
    doCheckConnectors();
}

void ReadContext::doCheckConnectors()
{
    for (std::shared_ptr<ConnectorInfoReader>& c : _pendingConnectors) {
        addConnectorInfo(c);
    }
    _pendingConnectors.clear();
}

void ReadContext::addConnectorInfo(std::shared_ptr<ConnectorInfoReader> c)
{
    _connectors.push_back(std::move(c));
    ConnectorInfoReader* c1 = _connectors.back().get();
    c1->update();
    for (std::shared_ptr<ConnectorInfoReader>& c2 : _connectors) {
        if (c2->connect(c1)) {
            if (c2->finished()) {
                c2->addToScore(pasteMode());
                removeConnector(c2.get());
            }
            break;
        }
    }
}

static bool distanceSort(const std::pair<int, std::pair<ConnectorInfoReader*, ConnectorInfoReader*> >& p1,
                         const std::pair<int, std::pair<ConnectorInfoReader*, ConnectorInfoReader*> >& p2)
{
    return p1.first < p2.first;
}

//---------------------------------------------------------
//   reconnectBrokenConnectors
//---------------------------------------------------------

void ReadContext::reconnectBrokenConnectors()
{
    doReconnectBrokenConnectors();
}

void ReadContext::clearOrphanedConnectors()
{
    if (_connectors.empty() && _pendingConnectors.empty()) {
        return;
    }

    LOGD("XmlReader::~XmlReader: there are unpaired connectors left");

    std::set<LinkedObjects*> deletedLinks;

    auto deleteConnectors = [&deletedLinks](std::shared_ptr<ConnectorInfoReader> c) {
        EngravingItem* conn = c ? c->releaseConnector() : nullptr;
        if (!conn) {
            return;
        }

        LinkedObjects* links = conn->links();
        bool linksWillBeDeleted = links && links->size() == 1;

        if (!conn->isTuplet()) {     // tuplets are added to score even when not finished
            if (linksWillBeDeleted) {
                deletedLinks.insert(links);
                if (conn->isTrill()) {
                    EngravingItem* ornament = (EngravingItem*)toTrill(conn)->ornament();
                    if (ornament && ornament->links()) {
                        deletedLinks.insert(ornament->links());
                    }
                }
            }

            delete conn;
        }
    };

    if (!_connectors.empty()) {
        for (auto& c : _connectors) {
            deleteConnectors(c);
        }
        _connectors.clear();
    }

    if (!_pendingConnectors.empty()) {
        for (auto& c : _pendingConnectors) {
            deleteConnectors(c);
        }
        _pendingConnectors.clear();
    }

    for (auto& it : m_staffLinkedElements) {
        std::vector<std::pair<LinkedObjects*, Location> >& vector = it.second;
        muse::remove_if(vector, [&deletedLinks](std::pair<LinkedObjects*, Location>& pair){
            return deletedLinks.count(pair.first);
        });
    }
}

void ReadContext::doReconnectBrokenConnectors()
{
    if (_connectors.empty()) {
        return;
    }
    LOGD("Reconnecting broken connectors (%d nodes)", int(_connectors.size()));
    std::vector<std::pair<int, std::pair<ConnectorInfoReader*, ConnectorInfoReader*> > > brokenPairs;
    for (size_t i = 1; i < _connectors.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            ConnectorInfoReader* c1 = _connectors[i].get();
            ConnectorInfoReader* c2 = _connectors[j].get();
            int d = c1->connectionDistance(*c2);
            if (d >= 0) {
                brokenPairs.push_back(std::make_pair(d, std::make_pair(c1, c2)));
            } else {
                brokenPairs.push_back(std::make_pair(-d, std::make_pair(c2, c1)));
            }
        }
    }
    std::sort(brokenPairs.begin(), brokenPairs.end(), distanceSort);
    std::set<ConnectorInfoReader*> processed;
    for (auto& distPair : brokenPairs) {
        if (distPair.first == INT_MAX) {
            continue;
        }
        auto& pair = distPair.second;
        if (processed.count(pair.first) || processed.count(pair.second)) {
            continue;
        }
        pair.first->forceConnect(pair.second);
        processed.insert(pair.first);
        processed.insert(pair.second);
    }
    std::set<ConnectorInfoReader*> reconnected;
    for (auto& conn : _connectors) {
        ConnectorInfoReader* c = conn.get();
        if (c->finished()) {
            reconnected.insert(static_cast<ConnectorInfoReader*>(c->start()));
        }
    }
    for (ConnectorInfoReader* cptr : reconnected) {
        cptr->addToScore(pasteMode());
        removeConnector(cptr);
    }
    LOGD() << "reconnected broken connectors: " << reconnected.size();
}

//---------------------------------------------------------
//   addUserTextStyle
//    return false if mapping is not possible
//      (too many user text styles)
//---------------------------------------------------------

TextStyleType ReadContext::addUserTextStyle(const String& name)
{
    LOGD() << name;
    TextStyleType id = TextStyleType::TEXT_TYPES;
    if (userTextStyles.size() == 0) {
        id = TextStyleType::USER1;
    } else if (userTextStyles.size() == 1) {
        id = TextStyleType::USER2;
    } else if (userTextStyles.size() == 2) {
        id = TextStyleType::USER3;
    } else if (userTextStyles.size() == 3) {
        id = TextStyleType::USER4;
    } else if (userTextStyles.size() == 4) {
        id = TextStyleType::USER5;
    } else if (userTextStyles.size() == 5) {
        id = TextStyleType::USER6;
    } else if (userTextStyles.size() == 6) {
        id = TextStyleType::USER7;
    } else if (userTextStyles.size() == 7) {
        id = TextStyleType::USER8;
    } else if (userTextStyles.size() == 8) {
        id = TextStyleType::USER9;
    } else if (userTextStyles.size() == 9) {
        id = TextStyleType::USER10;
    } else if (userTextStyles.size() == 10) {
        id = TextStyleType::USER11;
    } else if (userTextStyles.size() == 11) {
        id = TextStyleType::USER12;
    } else {
        LOGD() << "too many user defined textstyles";
    }
    if (id != TextStyleType::TEXT_TYPES) {
        userTextStyles.push_back({ name, id });
    }
    return id;
}

TextStyleType ReadContext::lookupUserTextStyle(const String& name) const
{
    for (const auto& i : userTextStyles) {
        if (i.name == name) {
            return i.ss;
        }
    }
    return TextStyleType::TEXT_TYPES;         // not found
}

void ReadContext::addPartAudioSettingCompat(PartAudioSettingsCompat partAudioSetting)
{
    if (_settingsCompat.audioSettings.count(partAudioSetting.instrumentId.partId) == 0) {
        _settingsCompat.audioSettings.insert({ partAudioSetting.instrumentId.partId, partAudioSetting });
    }
}

TimeSigMap* ReadContext::compatTimeSigMap()
{
    return &m_compatTimeSigMap;
}
