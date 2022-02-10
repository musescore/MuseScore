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
#include "defaultstyle.h"

#include <QFile>

#include "log.h"

using namespace mu::engraving;
using namespace Ms;

static const int LEGACY_MSC_VERSION_V302 = 302;
static const int LEGACY_MSC_VERSION_V3 = 301;
static const int LEGACY_MSC_VERSION_V2 = 206;
static const int LEGACY_MSC_VERSION_V1 = 114;

static const QString LEGACY_MSS_V1_PATH(":/engraving/styles/legacy-style-defaults-v1.mss");
static const QString LEGACY_MSS_V2_PATH(":/engraving/styles/legacy-style-defaults-v2.mss");
static const QString LEGACY_MSS_V3_PATH(":/engraving/styles/legacy-style-defaults-v3.mss");
static const QString LEGACY_MSS_V302_PATH(":/engraving/styles/legacy-style-defaults-v302.mss");

DefaultStyle* DefaultStyle::instance()
{
    static DefaultStyle s;
    return &s;
}

void DefaultStyle::init(const QString& defaultSyleFilePath, const QString& partStyleFilePath)
{
    m_baseStyle.precomputeValues();

    if (!defaultSyleFilePath.isEmpty()) {
        m_defaultStyle = new MStyle();
        bool ok = doLoadStyle(m_defaultStyle, defaultSyleFilePath);
        if (!ok) {
            delete m_defaultStyle;
            m_defaultStyle = nullptr;
        } else {
            m_defaultStyle->precomputeValues();
        }
    }

    if (!partStyleFilePath.isEmpty()) {
        m_defaultStyleForParts = new MStyle();
        bool ok = doLoadStyle(m_defaultStyleForParts, defaultSyleFilePath);
        if (!ok) {
            delete m_defaultStyleForParts;
            m_defaultStyleForParts = nullptr;
        } else {
            m_defaultStyleForParts->precomputeValues();
        }
    }
}

bool DefaultStyle::doLoadStyle(Ms::MStyle* style, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOGE() << "failed load style: " << filePath;
        return false;
    }

    return style->read(&file);
}

// Static

const Ms::MStyle& DefaultStyle::baseStyle()
{
    return instance()->m_baseStyle;
}

bool DefaultStyle::isHasDefaultStyle()
{
    if (instance()->m_defaultStyle) {
        return true;
    }
    return false;
}

const Ms::MStyle& DefaultStyle::defaultStyle()
{
    if (instance()->m_defaultStyle) {
        return *instance()->m_defaultStyle;
    }
    return instance()->m_baseStyle;
}

const MStyle* DefaultStyle::defaultStyleForParts()
{
    return instance()->m_defaultStyleForParts;
}

const MStyle& DefaultStyle::resolveStyleDefaults(const int defaultsVersion)
{
    static auto loadedStyle = [](MStyle& style, const QString& path, bool& loaded_flag) -> const MStyle&
    {
        if (loaded_flag) {
            return style;
        }
        loaded_flag = doLoadStyle(&style, path);
        return style;
    };

    switch (defaultsVersion) {
    case LEGACY_MSC_VERSION_V302: {
        static MStyle style_v302;
        static bool loaded_v302 = false;
        return loadedStyle(style_v302, LEGACY_MSS_V302_PATH, loaded_v302);
    } break;
    case LEGACY_MSC_VERSION_V3: {
        static MStyle style_v3;
        static bool loaded_v3 = false;
        return loadedStyle(style_v3, LEGACY_MSS_V3_PATH, loaded_v3);
    } break;
    case LEGACY_MSC_VERSION_V2: {
        static MStyle style_v2;
        static bool loaded_v2 = false;
        return loadedStyle(style_v2, LEGACY_MSS_V2_PATH, loaded_v2);
    } break;
    case LEGACY_MSC_VERSION_V1: {
        static MStyle style_v1;
        static bool loaded_v1 = false;
        return loadedStyle(style_v1, LEGACY_MSS_V1_PATH, loaded_v1);
    } break;
    default:
        return baseStyle();
    }
}
