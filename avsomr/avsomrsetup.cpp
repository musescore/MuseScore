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

#include "avsomrsetup.h"

#include <QTimer>

#include "mscore/preferences.h"

#include "avslog.h"
#include "avsomrlocal.h"

using namespace Ms::Avs;

//---------------------------------------------------------
//   moduleName
//---------------------------------------------------------

std::string AvsOmrSetup::moduleName() const
{
    return "avsomr";
}

//---------------------------------------------------------
//   onStartApp
//---------------------------------------------------------

void AvsOmrSetup::onStartApp()
{
    AvsOmrLocal* avsLocal = AvsOmrLocal::instance();

    //! NOTE If enabled use local avs then check the installation or update
    //! on application start
    QTimer::singleShot(10000, [avsLocal]() {
        if (avsLocal->isUseLocal()) {
            avsLocal->checkInstallOrUpdate(false);
        }
    });

    //! NOTE If enabled use local avs then immediately start the installation
    //! on preference changed
    preferences.addOnSetListener([avsLocal](const QString& key, const QVariant& value) {
        if (key != PREF_IMPORT_AVSOMR_USELOCAL) {
            return;
        }

        bool useLocalAvsOmr = value.toBool();
        if (!useLocalAvsOmr) {
            return;
        }

        avsLocal->checkInstallOrUpdate(false);
    });
}
