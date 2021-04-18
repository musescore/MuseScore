//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_IAPPSHELLCONFIGURATION_H
#define MU_APPSHELL_IAPPSHELLCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "retval.h"

#include "io/path.h"
#include "appshelltypes.h"

namespace mu::appshell {
class IAppShellConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAppshellConfiguration)

public:
    virtual ~IAppShellConfiguration() = default;

    virtual StartupSessionType startupSessionType() const = 0;
    virtual void setStartupSessionType(StartupSessionType type) = 0;

    virtual io::path startupScorePath() const = 0;
    virtual void setStartupScorePath(const io::path& scorePath) = 0;

    virtual bool isAppUpdatable() const = 0;

    virtual bool needCheckForUpdate() const = 0;
    virtual void setNeedCheckForUpdate(bool needCheck) = 0;

    virtual std::string handbookUrl() const = 0;
    virtual std::string askForHelpUrl() const = 0;
    virtual std::string bugReportUrl() const = 0;
    virtual std::string leaveFeedbackUrl() const = 0;
    virtual std::string museScoreUrl() const = 0;
    virtual std::string museScoreContributionUrl() const = 0;
    virtual std::string musicXMLLicenseUrl() const = 0;
    virtual std::string musicXMLLicenseDeedUrl() const = 0;

    virtual std::string museScoreVersion() const = 0;
    virtual std::string museScoreRevision() const = 0;

    virtual ValCh<io::paths> recentScorePaths() const = 0;

    virtual ValCh<bool> isNotationStatusBarVisible() const = 0;
    virtual void setIsNotationStatusBarVisible(bool visible) const = 0;
    virtual ValCh<bool> isNotationNavigatorVisible() const = 0;
    virtual void setIsNotationNavigatorVisible(bool visible) const = 0;

    virtual bool needShowSplashScreen() const = 0;
    virtual void setNeedShowSplashScreen(bool show) = 0;

    virtual bool needShowTours() const = 0;
    virtual void setNeedShowTours(bool show) = 0;

    virtual void startEditSettings() = 0;
    virtual void applySettings() = 0;
    virtual void rollbackSettings() = 0;

    virtual void revertToFactorySettings(bool keepDefaultSettings = false) const = 0;
};
}

#endif // MU_APPSHELL_IAPPSHELLCONFIGURATION_H
