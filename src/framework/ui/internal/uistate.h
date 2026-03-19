/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "../iuistate.h"

#include "modularity/ioc.h"

#include "global/async/notification.h"
#include "uiarrangement.h"

namespace muse::ui {
class UiState : public IUiState, public async::Asyncable, public Contextable
{
public:
    UiState(const modularity::ContextPtr& iocCtx)
        : Contextable(iocCtx), m_uiArrangement(iocCtx) {}

    void init();

    QByteArray windowGeometry() const override;
    void setWindowGeometry(const QByteArray& state) override;
    async::Notification windowGeometryChanged() const override;

    ValNt<QByteArray> pageState(const QString& pageName) const override;
    void setPageState(const QString& pageName, const QByteArray& state) override;

    ToolConfig toolConfig(const QString& toolName, const ToolConfig& defaultConfig) const override;
    void setToolConfig(const QString& toolName, const ToolConfig& config) override;
    async::Notification toolConfigChanged(const QString& toolName) const override;

    QString uiItemState(const QString& itemName) const override;
    void setUiItemState(const QString& itemName, const QString& value) override;
    async::Notification uiItemStateChanged(const QString& itemName) const override;

    bool isVisible(const QString& key, bool def = true) const override;
    void setIsVisible(const QString& key, bool val) override;
    async::Notification isVisibleChanged(const QString& key) const override;

private:

    void updateToolConfig(const QString& toolName, ToolConfig& userConfig, const ToolConfig& defaultConfig) const;

    UiArrangement m_uiArrangement;

    async::Notification m_windowGeometryChanged;
};
}
