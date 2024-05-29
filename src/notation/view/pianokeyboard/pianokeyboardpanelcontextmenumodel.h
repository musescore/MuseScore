/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
#ifndef MU_NOTATION_PIANOKEYBOARDPANELCONTEXTMENUMODEL_H
#define MU_NOTATION_PIANOKEYBOARDPANELCONTEXTMENUMODEL_H

#include "uicomponents/view/abstractmenumodel.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "inotationconfiguration.h"
#include "actions/iactionsdispatcher.h"

namespace muse {
class TranslatableString;
}

namespace mu::notation {
class PianoKeyboardPanelContextMenuModel : public muse::uicomponents::AbstractMenuModel, public muse::actions::Actionable
{
    Q_OBJECT

    Q_PROPERTY(int numberOfKeys READ numberOfKeys NOTIFY numberOfKeysChanged)
    Q_PROPERTY(qreal keyWidthScaling READ keyWidthScaling WRITE setKeyWidthScaling NOTIFY keyWidthScalingChanged)

    muse::Inject<INotationConfiguration> configuration = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit PianoKeyboardPanelContextMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

    qreal keyWidthScaling() const;
    void setKeyWidthScaling(qreal scaling);

    int numberOfKeys() const;

signals:
    void keyWidthScalingChanged();
    void setKeyWidthScalingRequested(qreal scaling);
    void numberOfKeysChanged();

private:
    muse::uicomponents::MenuItem* makeViewMenu();

    muse::uicomponents::MenuItem* makeKeyWidthScalingItem(const muse::TranslatableString& title, qreal scaling);
    muse::uicomponents::MenuItem* makeNumberOfKeysItem(const muse::TranslatableString& title, int numberOfKeys);

    void updateKeyWidthScalingItems();

    muse::uicomponents::MenuItemList m_keyWidthScalingItems;

    qreal m_currentKeyWidthScaling = 0.0;
};
}

#endif // MU_NOTATION_PIANOKEYBOARDPANELCONTEXTMENUMODEL_H
