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

#include "uistate.h"

#include "global/async/async.h"

using namespace muse;
using namespace muse::ui;
using namespace muse::async;

static const QString WINDOW_GEOMETRY_KEY("window");

void UiState::init()
{
    m_uiArrangement.stateChanged(WINDOW_GEOMETRY_KEY).onNotify(this, [this]() {
        m_windowGeometryChanged.notify();
    });

    m_uiArrangement.load();
}

QByteArray UiState::windowGeometry() const
{
    return m_uiArrangement.state(WINDOW_GEOMETRY_KEY);
}

void UiState::setWindowGeometry(const QByteArray& geometry)
{
    m_uiArrangement.setState(WINDOW_GEOMETRY_KEY, geometry);
}

muse::async::Notification UiState::windowGeometryChanged() const
{
    return m_windowGeometryChanged;
}

ValNt<QByteArray> UiState::pageState(const QString& pageName) const
{
    ValNt<QByteArray> result;
    result.val = m_uiArrangement.state(pageName);
    result.notification = m_uiArrangement.stateChanged(pageName);

    return result;
}

void UiState::setPageState(const QString& pageName, const QByteArray& state)
{
    m_uiArrangement.setState(pageName, state);
}

ToolConfig UiState::toolConfig(const QString& toolName, const ToolConfig& defaultConfig) const
{
    ToolConfig config = m_uiArrangement.toolConfig(toolName);
    if (!config.isValid()) {
        return defaultConfig;
    }

    updateToolConfig(toolName, config, defaultConfig);
    return config;
}

void UiState::setToolConfig(const QString& toolName, const ToolConfig& config)
{
    m_uiArrangement.setToolConfig(toolName, config);
}

async::Notification UiState::toolConfigChanged(const QString& toolName) const
{
    return m_uiArrangement.toolConfigChanged(toolName);
}

void UiState::updateToolConfig(const QString& toolName, ToolConfig& userConfig, const ToolConfig& defaultConfig) const
{
    bool hasChanged = false;

    // Remove items that are not in the default config
    {
        QList<ToolConfig::Item> itemsToRemove;
        for (const auto& item : userConfig.items) {
            if (item.isSeparator()) {
                continue;
            }

            if (std::find_if(defaultConfig.items.cbegin(), defaultConfig.items.cend(), [item](const auto& defaultItem) {
                return item.action == defaultItem.action;
            }) == defaultConfig.items.cend()) {
                itemsToRemove << item;
            }
        }

        for (const auto& itemToRemove : itemsToRemove) {
            hasChanged = true;
            userConfig.items.removeAll(itemToRemove);
        }
    }

    // Insert items that are missing in the user config
    {
        for (const auto& defaultItem : defaultConfig.items) {
            if (defaultItem.isSeparator()) {
                continue;
            }

            if (std::find_if(userConfig.items.cbegin(), userConfig.items.cend(), [defaultItem](const auto& item) {
                return defaultItem.action == item.action;
            }) == userConfig.items.cend()) {
                hasChanged = true;

                // Try to find a good place to insert the item
                int indexOfDefaultItem = defaultConfig.items.indexOf(defaultItem);

                // If it was at the start of the default items...
                if (indexOfDefaultItem == 0) {
                    // insert it at the start of the user items
                    userConfig.items.prepend(defaultItem);
                    continue;
                }

                // If it was at the end of the default items...
                if (indexOfDefaultItem == defaultConfig.items.size() - 1) {
                    // insert it at the end of the user items
                    userConfig.items.append(defaultItem);
                    continue;
                }

                // Look at the item before it...
                {
                    const auto& itemBefore = defaultConfig.items[indexOfDefaultItem - 1];
                    if (!itemBefore.isSeparator()) {
                        auto it = std::find_if(userConfig.items.begin(), userConfig.items.end(), [itemBefore](const auto& item) {
                            return item.action == itemBefore.action;
                        });

                        if (it != userConfig.items.end()) {
                            userConfig.items.insert(++it, defaultItem);
                            continue;
                        }
                    }
                }

                // Look at the item after it...
                {
                    const auto& itemAfter  = defaultConfig.items[indexOfDefaultItem + 1];
                    if (!itemAfter.isSeparator()) {
                        auto it = std::find_if(userConfig.items.begin(), userConfig.items.end(), [itemAfter](const auto& item) {
                            return item.action == itemAfter.action;
                        });

                        userConfig.items.insert(it, defaultItem);
                        continue;
                    }
                }

                // Last resort: just insert at the end
                userConfig.items.append(defaultItem);
            }
        }
    }

    if (hasChanged) {
        // Save for later
        auto self = const_cast<UiState*>(this);

        Async::call(self, [self, toolName, userConfig]() {
            self->setToolConfig(toolName, userConfig);
        });
    }
}

QString UiState::uiItemState(const QString& itemName) const
{
    return m_uiArrangement.value(itemName);
}

void UiState::setUiItemState(const QString& itemName, const QString& value)
{
    m_uiArrangement.setValue(itemName, value);
}

Notification UiState::uiItemStateChanged(const QString& itemName) const
{
    return m_uiArrangement.valueChanged(itemName);
}

bool UiState::isVisible(const QString& key, bool def) const
{
    QString val = m_uiArrangement.value(key);
    bool ok = false;
    int valInt = val.toInt(&ok);
    return ok ? bool(valInt) : def;
}

void UiState::setIsVisible(const QString& key, bool val)
{
    m_uiArrangement.setValue(key, QString::number(val ? 1 : 0));
}

async::Notification UiState::isVisibleChanged(const QString& key) const
{
    return m_uiArrangement.valueChanged(key);
}
