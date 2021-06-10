/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "multiinstancesprovider.h"

#include "uri.h"
#include "ipcchannel.h"

using namespace mu::mi;

static const mu::UriQuery DEV_SHOW_INFO_URI("musescore://devtools/multiinstances/info?sync=false&modal=false");

MultiInstancesProvider::~MultiInstancesProvider()
{
    delete m_ipcChannel;
}

void MultiInstancesProvider::init()
{
    dispatcher()->reg(this, "multiinstances-dev-show-info", [this]() {
        if (!interactive()->isOpened(DEV_SHOW_INFO_URI.uri()).val) {
            interactive()->open(DEV_SHOW_INFO_URI);
        }
    });

    m_ipcChannel = new IpcChannel();
    m_selfID = m_ipcChannel->selfID().toStdString();

    m_ipcChannel->instancesChanged().onNotify(this, [this]() {
        m_instancesChanged.notify();
    });

    m_ipcChannel->init();
}

bool MultiInstancesProvider::isScoreAlreadyOpened(const io::path& scorePath) const
{
    Q_UNUSED(scorePath);
    return false;
}

void MultiInstancesProvider::activateWindowForScore(const io::path& scorePath)
{
    Q_UNUSED(scorePath);
}

const std::string& MultiInstancesProvider::selfID() const
{
    return m_selfID;
}

void MultiInstancesProvider::ping()
{
    m_ipcChannel->ping();
}

std::vector<InstanceMeta> MultiInstancesProvider::instances() const
{
    std::vector<InstanceMeta> ret;
    QList<IpcChannel::Meta> ints = m_ipcChannel->instances();
    for (const IpcChannel::Meta& m : qAsConst(ints)) {
        InstanceMeta im;
        im.id = m.id.toStdString();
        im.isServer = m.isServer;
        ret.push_back(std::move(im));
    }

    return ret;
}

mu::async::Notification MultiInstancesProvider::instancesChanged() const
{
    return m_instancesChanged;
}
