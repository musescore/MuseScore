//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "instrumentsconfiguration.h"

#include "log.h"
#include "settings.h"

using namespace mu;
using namespace mu::instruments;
using namespace mu::framework;

static const std::string module_name("instruments");
static const Settings::Key FIRST_INSTRUMENT_LIST_KEY(module_name, "application/paths/instrumentList1");
static const Settings::Key SECOND_INSTRUMENT_LIST_KEY(module_name, "application/paths/instrumentList2");
static const Settings::Key FIRST_SCORE_ORDER_LIST_KEY(module_name, "application/paths/scoreOrderList1");
static const Settings::Key SECOND_SCORE_ORDER_LIST_KEY(module_name, "application/paths/scoreOrderList2");

void InstrumentsConfiguration::init()
{
    settings()->setDefaultValue(FIRST_INSTRUMENT_LIST_KEY,
                                Val(globalConfiguration()->sharePath().toStdString() + "instruments/instruments.xml"));
    settings()->valueChanged(FIRST_INSTRUMENT_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_instrumentListPathsChanged.notify();
    });

    settings()->setDefaultValue(SECOND_INSTRUMENT_LIST_KEY, Val(""));
    settings()->valueChanged(SECOND_INSTRUMENT_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_instrumentListPathsChanged.notify();
    });

    settings()->setDefaultValue(FIRST_SCORE_ORDER_LIST_KEY,
                                Val(globalConfiguration()->sharePath().toStdString() + "instruments/orders.xml"));
    settings()->valueChanged(FIRST_SCORE_ORDER_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_scoreOrderListPathsChanged.notify();
    });

    settings()->setDefaultValue(SECOND_SCORE_ORDER_LIST_KEY, Val(""));
    settings()->valueChanged(SECOND_SCORE_ORDER_LIST_KEY).onReceive(nullptr, [this](const Val&) {
        m_scoreOrderListPathsChanged.notify();
    });
}

io::paths InstrumentsConfiguration::instrumentListPaths() const
{
    io::paths paths;

    io::path firstInstrumentListPath = this->firstInstrumentListPath();
    paths.push_back(firstInstrumentListPath);

    io::path secondInstrumentListPath = this->secondInstrumentListPath();
    if (!secondInstrumentListPath.empty()) {
        paths.push_back(secondInstrumentListPath);
    }

    io::paths extensionsPath = this->extensionsPaths();
    paths.insert(paths.end(), extensionsPath.begin(), extensionsPath.end());

    return paths;
}

async::Notification InstrumentsConfiguration::instrumentListPathsChanged() const
{
    return m_instrumentListPathsChanged;
}

io::paths InstrumentsConfiguration::userInstrumentListPaths() const
{
    io::paths paths = {
        firstInstrumentListPath(),
        secondInstrumentListPath()
    };

    return paths;
}

void InstrumentsConfiguration::setUserInstrumentListPaths(const io::paths& paths)
{
    if (paths.empty()) {
        return;
    }

    setFirstInstrumentListPath(paths[0]);
    if (paths.size() > 1) {
        setSecondInstrumentListPath(paths[1]);
    }
}

io::path InstrumentsConfiguration::firstInstrumentListPath() const
{
    return settings()->value(FIRST_INSTRUMENT_LIST_KEY).toString();
}

void InstrumentsConfiguration::setFirstInstrumentListPath(const io::path& path)
{
    settings()->setValue(FIRST_INSTRUMENT_LIST_KEY, Val(path.toStdString()));
}

io::path InstrumentsConfiguration::secondInstrumentListPath() const
{
    return settings()->value(SECOND_INSTRUMENT_LIST_KEY).toString();
}

void InstrumentsConfiguration::setSecondInstrumentListPath(const io::path& path)
{
    settings()->setValue(SECOND_INSTRUMENT_LIST_KEY, Val(path.toStdString()));
}

io::paths InstrumentsConfiguration::scoreOrderListPaths() const
{
    io::paths paths;

    io::path firstScoreOrderListPath = this->firstScoreOrderListPath();
    paths.push_back(firstScoreOrderListPath);

    io::path secondScoreOrderListPath = this->secondScoreOrderListPath();
    if (!secondScoreOrderListPath.empty()) {
        paths.push_back(secondScoreOrderListPath);
    }

    return paths;
}

async::Notification InstrumentsConfiguration::scoreOrderListPathsChanged() const
{
    return m_scoreOrderListPathsChanged;
}

io::paths InstrumentsConfiguration::userScoreOrderListPaths() const
{
    io::paths paths = {
        firstScoreOrderListPath(),
        secondScoreOrderListPath()
    };

    return paths;
}

void InstrumentsConfiguration::setUserScoreOrderListPaths(const io::paths& paths)
{
    if (paths.empty()) {
        return;
    }

    setFirstScoreOrderListPath(paths[0]);
    if (paths.size() > 1) {
        setSecondScoreOrderListPath(paths[1]);
    }
}

io::path InstrumentsConfiguration::firstScoreOrderListPath() const
{
    return settings()->value(FIRST_SCORE_ORDER_LIST_KEY).toString();
}

void InstrumentsConfiguration::setFirstScoreOrderListPath(const io::path& path)
{
    settings()->setValue(FIRST_SCORE_ORDER_LIST_KEY, Val(path.toStdString()));
}

io::path InstrumentsConfiguration::secondScoreOrderListPath() const
{
    return settings()->value(SECOND_SCORE_ORDER_LIST_KEY).toString();
}

void InstrumentsConfiguration::setSecondScoreOrderListPath(const io::path& path)
{
    settings()->setValue(SECOND_SCORE_ORDER_LIST_KEY, Val(path.toStdString()));
}

io::paths InstrumentsConfiguration::extensionsPaths() const
{
    io::paths extensionsInstrumentsPaths = extensionsConfigurator()->instrumentsPaths();
    io::paths paths;

    for (const io::path& path: extensionsInstrumentsPaths) {
        RetVal<io::paths> files = fileSystem()->scanFiles(path, { QString("*.xml") });
        if (!files.ret) {
            LOGE() << files.ret.toString();
            continue;
        }

        for (const io::path& file: files.val) {
            paths.push_back(file);
        }
    }

    return paths;
}
