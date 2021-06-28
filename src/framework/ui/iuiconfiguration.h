/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_UI_IUICONFIGURATION_H
#define MU_UI_IUICONFIGURATION_H

#include <optional>

#include "modularity/imoduleexport.h"
#include "async/notification.h"
#include "actions/actiontypes.h"

#include "uitypes.h"

class QByteArray;
class QWindow;

namespace mu::ui {
class IUiConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUiConfiguration)

public:
    virtual ~IUiConfiguration() = default;

    virtual ThemeList themes() const = 0;
    virtual QStringList possibleFontFamilies() const = 0;
    virtual QStringList possibleAccentColors() const = 0;

    virtual const ThemeInfo& currentTheme() const = 0;
    virtual void setCurrentTheme(const ThemeCode& codeKey) = 0;
    virtual void setCurrentThemeStyleValue(ThemeStyleKey key, const Val& val) = 0;
    virtual async::Notification currentThemeChanged() const = 0;

    virtual std::string fontFamily() const = 0;
    virtual void setFontFamily(const std::string& family) = 0;
    virtual int fontSize(FontSizeType type) const = 0;
    virtual void setBodyFontSize(int size) = 0;
    virtual async::Notification fontChanged() const = 0;

    virtual std::string iconsFontFamily() const = 0;
    virtual int iconsFontSize(IconSizeType type) const = 0;
    virtual async::Notification iconsFontChanged() const = 0;

    virtual std::string musicalFontFamily() const = 0;
    virtual int musicalFontSize() const = 0;
    virtual async::Notification musicalFontChanged() const = 0;

    virtual float guiScaling() const = 0;
    virtual float physicalDotsPerInch() const = 0;

    //! NOTE Maybe set from command line
    virtual void setPhysicalDotsPerInch(std::optional<float> dpi) = 0;

    virtual QByteArray pageState(const QString& pageName) const = 0;
    virtual void setPageState(const QString& pageName, const QByteArray& state) = 0;

    virtual QByteArray windowGeometry() const = 0;
    virtual void setWindowGeometry(const QByteArray& state) = 0;
    virtual async::Notification windowGeometryChanged() const = 0;

    virtual void applyPlatformStyle(QWindow* window) = 0;

    virtual bool isVisible(const QString& key, bool def = true) const = 0;
    virtual void setIsVisible(const QString& key, bool val) = 0;
    virtual async::Notification isVisibleChanged(const QString& key) const = 0;

    virtual ToolConfig toolConfig(const QString& toolName) const = 0;
    virtual void setToolConfig(const QString& toolName, const ToolConfig& config) = 0;
    virtual async::Notification toolConfigChanged(const QString& toolName) const = 0;
};
}

#endif // MU_UI_IUICONFIGURATION_H
