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
#include "notationconfiguration.h"
#include "settings.h"

using namespace mu::domain::notation;
using namespace mu::framework;

static std::string module_name("notation");

static const Settings::Key ANCHORLINE_COLOR(module_name, "ui/score/voice4/color");
static const Settings::Key USER_TEMPLATES(module_name, "application/paths/myTemplates");

void NotationConfiguration::init()
{
    settings()->addItem(ANCHORLINE_COLOR, Val(QColor("#C31989")));
}

QColor NotationConfiguration::anchorLineColor() const
{
    return settings()->value(ANCHORLINE_COLOR).toQColor();
}

QString NotationConfiguration::templatesPath() const
{
    return io::pathToQString(globalConfiguration()->sharePath() + "/templates");
}

QString NotationConfiguration::userTemplatesPath() const
{
    return QString::fromStdString(settings()->value(USER_TEMPLATES).toString());
}

QStringList NotationConfiguration::extensionsTemplatesPaths() const
{
    return extensionsConfiguration()->templatesPaths();
}
