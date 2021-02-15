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

#include "vstscanner.h"
#include <string>
#include <QDir>
#include <QString>
#include "pluginloader.h"

using namespace mu::vst;

VSTScanner::VSTScanner(std::string paths)
{
    setPaths(paths);
}

VSTScanner::VSTScanner()
{
}

//TODO: rewrite with FsOperations when it will have needed functionality
void VSTScanner::scan()
{
    for (auto&& dirPath : m_paths) {
        QDir directory(QString::fromStdString(dirPath));
        if (directory.exists()) {
            for (auto&& pluginName : directory.entryList(QStringList({ "*.vst3" }))) {
                PluginLoader loader(dirPath, pluginName.toStdString());
                loader.load();
                auto plugins = loader.getPlugins();

                for (auto&& p : plugins) {
                    if (p.getType() == Plugin::Instrument) {
                        m_plugins[p.getId()] = p;
                    }
                }
            }
        }
    }
}

const std::map<std::string, Plugin>& VSTScanner::getPlugins() const
{
    return m_plugins;
}

std::string VSTScanner::paths() const
{
    std::string paths;
    for (auto&& path : m_paths) {
        paths += (paths.length() ? ";" : "") + path;
    }
    return paths;
}

void VSTScanner::setPaths(const std::string& paths)
{
    m_paths.clear();
    std::string::size_type pos = paths.find(";"), lastPos = 0;
    do {
        m_paths.push_back(paths.substr(lastPos, pos - lastPos));
        lastPos = pos + 1;
        pos = paths.find(";", lastPos);
    } while (lastPos - 1 != std::string::npos);
}
