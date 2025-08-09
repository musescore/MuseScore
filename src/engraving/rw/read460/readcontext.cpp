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

#include "dom/linkedobjects.h"
#include "dom/score.h"
#include "dom/trill.h"
#include "dom/undo.h"

#include "connectorinforeader.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::read460;

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

Score* ReadContext::score() const
{
    return m_score;
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

Staff* ReadContext::staff(staff_idx_t n)
{
    return m_score->staff(n);
}

bool ReadContext::isSameScore(const EngravingObject* obj) const
{
    return obj->score() == m_score;
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
    Location l = Location::absolute();
    fillLocation(l, forceAbsFrac);
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
    if (l.isRelative()) {
        Location newLoc = l;
        newLoc.toAbsolute(location());
        int intTicks = l.frac().ticks();
        if (_tick == Fraction::fromTicks(_intTick + intTicks)) {
            _intTick += intTicks;
            setTrack(newLoc.track() - _trackOffset);
            return;
        }
        setLocation(newLoc); // recursion
        return;
    }
    setTrack(l.track() - _trackOffset);
    setTick(l.frac() - _tickOffset);
    if (!pasteMode()) {
        assert(l.measure() == currentMeasureIndex());
        incTick(currentMeasure()->tick());
    }
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
}

void ReadContext::addPartAudioSettingCompat(PartAudioSettingsCompat partAudioSetting)
{
    if (_settingsCompat.audioSettings.count(partAudioSetting.instrumentId.partId) == 0) {
        _settingsCompat.audioSettings.insert({ partAudioSetting.instrumentId.partId, partAudioSetting });
    }
}
