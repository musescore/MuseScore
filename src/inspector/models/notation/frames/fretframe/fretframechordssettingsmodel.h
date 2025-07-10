/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#pragma once

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "uicomponents/view/selectableitemlistmodel.h"

#include "models/abstractinspectormodel.h"
#include "fretframechordlistmodel.h"

namespace mu::engraving {
class FBox;
}

namespace mu::inspector {
class FretFrameChordsSettingsModel : public AbstractInspectorModel, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(FretFrameChordListModel * chordListModel READ chordListModel CONSTANT)

    Q_PROPERTY(PropertyItem * listOrder READ listOrder CONSTANT)

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    explicit FretFrameChordsSettingsModel(QObject* parent, IElementRepositoryService* repository);

    FretFrameChordListModel* chordListModel() const;

    PropertyItem* listOrder() const;

private:
    engraving::FBox* fretBox() const;

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

    void loadProperties(const mu::engraving::PropertyIdSet& propertyIdSet);

    std::shared_ptr<FretFrameChordListModel> m_chordListModel;

    PropertyItem* m_listOrder = nullptr;
};
}
