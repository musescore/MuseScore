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

#include "setupavsomrview.h"

#include "mscore/scoreview.h"

#include "avslog.h"
#include "infopopup.h"

using namespace Ms::Avs;

SetupAvsOmrView::SetupAvsOmrView()
{
}

//---------------------------------------------------------
//   setupView
//---------------------------------------------------------

void SetupAvsOmrView::setupView(Ms::ScoreView* view, std::shared_ptr<AvsOmr> avsOmr)
{
    IF_ASSERT(view) {
        return;
    }

    IF_ASSERT(avsOmr) {
        return;
    }

    AvsOmr::Config& config = avsOmr->config();

    InfoPopup* infoPopup = new InfoPopup();
    infoPopup->setParent(view);

    // recognized
    config.setIsShowRecognized(true);
    infoPopup->setRecognizedChecked(true);
    QObject::connect(infoPopup, &InfoPopup::recognizedCheckedChanged, [view, avsOmr](bool checked) {
        avsOmr->config().setIsShowRecognized(checked);
        view->update();
    });

    // notrecognized
    config.setIsShowNotRecognized(true);
    infoPopup->setNotRecognizedChecked(true);
    QObject::connect(infoPopup, &InfoPopup::notrecognizedCheckedChanged, [view, avsOmr](bool checked) {
        avsOmr->config().setIsShowNotRecognized(checked);
        view->update();
    });

    // show popup
    infoPopup->showOnView(view, avsOmr->info());
}
