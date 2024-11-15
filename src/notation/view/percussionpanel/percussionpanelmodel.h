/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"
#include "playback/iplaybackcontroller.h"

#include "percussionpanelpadlistmodel.h"

namespace mu::notation {
class PanelMode
{
    Q_GADGET
public:
    enum Mode
    {
        WRITE,
        SOUND_PREVIEW, //! NOTE: Not to be confused with "notation preview"
        EDIT_LAYOUT
    };
    Q_ENUM(Mode)
};

class PercussionPanelModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<playback::IPlaybackController> playbackController = { this };

    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)

    Q_PROPERTY(PanelMode::Mode currentPanelMode READ currentPanelMode WRITE setCurrentPanelMode NOTIFY currentPanelModeChanged)
    Q_PROPERTY(bool useNotationPreview READ useNotationPreview WRITE setUseNotationPreview NOTIFY useNotationPreviewChanged)

    Q_PROPERTY(PercussionPanelPadListModel * padListModel READ padListModel NOTIFY padListModelChanged)

    Q_PROPERTY(QList<QVariantMap> layoutMenuItems READ layoutMenuItems CONSTANT)

public:
    explicit PercussionPanelModel(QObject* parent = nullptr);

    bool enabled() const;
    void setEnabled(bool enabled);

    PanelMode::Mode currentPanelMode() const;
    void setCurrentPanelMode(const PanelMode::Mode& panelMode);

    bool useNotationPreview() const;
    void setUseNotationPreview(bool useNotationPreview);

    PercussionPanelPadListModel* padListModel() const;

    Q_INVOKABLE void init();

    QList<QVariantMap> layoutMenuItems() const;
    Q_INVOKABLE void handleMenuItem(const QString& itemId);

    Q_INVOKABLE void finishEditing();

    Q_INVOKABLE void customizeKit();

signals:
    void enabledChanged();

    void currentPanelModeChanged(const PanelMode::Mode& panelMode);
    void useNotationPreviewChanged(bool useNotationPreview);

    void padListModelChanged();

private:
    void setUpConnections();

    void writePitch(int pitch);
    void playPitch(int pitch);

    const mu::notation::INotationPtr notation() const;
    const mu::notation::INotationInteractionPtr interaction() const;

    mu::engraving::Score* score() const;

    bool m_enabled = false;

    PanelMode::Mode m_currentPanelMode = PanelMode::Mode::WRITE;
    PanelMode::Mode m_panelModeToRestore = PanelMode::Mode::WRITE;
    bool m_useNotationPreview = false;

    PercussionPanelPadListModel* m_padListModel = nullptr;
};
}
