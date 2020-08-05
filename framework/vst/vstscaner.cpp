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

#include "vstscaner.h"

#include <QDir>
#include <QString>
#include "pluginloader.h"

using namespace mu::vst;

VSTScaner::VSTScaner(std::string paths)
    : m_paths(paths), m_plugins()
{
}

void VSTScaner::scan()
{
    QString dirPaths = QString::fromStdString(m_paths);
    for (auto dirPath : dirPaths.split(";")) {
        QDir directory(dirPath);
        if (directory.exists()) {
            for (auto pluginName : directory.entryList(QStringList({"*.vst3"}))) {

                PluginLoader loader(dirPath.toStdString(), pluginName.toStdString());
                loader.load();
                auto plugins = loader.getPlugins();

                for (auto p : plugins) {
                    if (p.getType() == Plugin::Instrument) {
                        m_plugins[p.getId()] = p;
                    }
                }
            }
        }
    }
}

std::string VSTScaner::paths() const
{
    return m_paths;
}

void VSTScaner::setPaths(const std::string &paths)
{
    m_paths = paths;
}
