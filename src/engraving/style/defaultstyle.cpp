/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "io/file.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

static const int LEGACY_MSC_VERSION_V302 = 302;
static const int LEGACY_MSC_VERSION_V3 = 301;
static const int LEGACY_MSC_VERSION_V2 = 206;
static const int LEGACY_MSC_VERSION_V1 = 114;

static const String LEGACY_MSS_V1_PATH(u":/engraving/styles/legacy-style-defaults-v1.mss");
static const String LEGACY_MSS_V2_PATH(u":/engraving/styles/legacy-style-defaults-v2.mss");
static const String LEGACY_MSS_V3_PATH(u":/engraving/styles/legacy-style-defaults-v3.mss");
static const String LEGACY_MSS_V302_PATH(u":/engraving/styles/legacy-style-defaults-v302.mss");

DefaultStyle* DefaultStyle::instance()
{
    static DefaultStyle s;
    return &s;
}

static void applyPageSizeToStyle(MStyle* style, const SizeF& pageSize)
{
    double oldWidth = style->styleD(Sid::pageWidth);
    double newPrintableWidth = style->styleD(Sid::pagePrintableWidth) + (pageSize.width() - oldWidth);

    style->set(Sid::pageWidth, pageSize.width());
    style->set(Sid::pageHeight, pageSize.height());
    style->set(Sid::pagePrintableWidth, newPrintableWidth);
}

bool DefaultStyle::init(const path_t& defaultStyleFilePath, const path_t& partStyleFilePath,
                        const path_t& paletteStyleFilePath, const SizeF& defaultPageSize)
{
    m_baseStyle.precomputeValues();

    {
        applyPageSizeToStyle(&m_defaultStyle, defaultPageSize);

        if (!defaultStyleFilePath.empty()) {
            bool ok = doLoadStyle(&m_defaultStyle, defaultStyleFilePath);
            if (!ok) {
                LOGW() << "Failed to load default style file from " << defaultStyleFilePath;
            }
        }

        m_defaultStyle.precomputeValues();
    }

    if (!partStyleFilePath.empty()) {
        m_defaultStyleForParts = new MStyle();

        applyPageSizeToStyle(m_defaultStyleForParts, defaultPageSize);

        bool ok = doLoadStyle(m_defaultStyleForParts, partStyleFilePath);
        if (!ok) {
            LOGW() << "Failed to load default part style file from " << partStyleFilePath;
            delete m_defaultStyleForParts;
            m_defaultStyleForParts = nullptr;
        } else {
            m_defaultStyleForParts->precomputeValues();
        }
    }

    if (!paletteStyleFilePath.empty()) {
        applyPageSizeToStyle(&m_defaultStyleForPalette, defaultPageSize);

        bool ok = doLoadStyle(&m_defaultStyleForPalette, paletteStyleFilePath);
        if (!ok) {
            LOGW() << "Failed to load default palette style file from " << paletteStyleFilePath;
        } else {
            m_defaultStyleForPalette.precomputeValues();
            return true;
        }
    }
    return false;
}

bool DefaultStyle::doLoadStyle(MStyle* style, const path_t& filePath)
{
    File file(filePath);
    if (!file.open(IODevice::ReadOnly)) {
        LOGE() << "failed load style: " << filePath;
        return false;
    }

    return style->read(&file);
}

// Static

const MStyle& DefaultStyle::baseStyle()
{
    return instance()->m_baseStyle;
}

const MStyle& DefaultStyle::defaultStyle()
{
    return instance()->m_defaultStyle;
}

const MStyle* DefaultStyle::defaultStyleForParts()
{
    return instance()->m_defaultStyleForParts;
}

const MStyle& DefaultStyle::defaultStyleForPalette()
{
    return instance()->m_defaultStyleForPalette;
}

const MStyle& DefaultStyle::resolveStyleDefaults(const int defaultsVersion)
{
    static auto loadedStyle = [](MStyle& style, const String& path, bool& loaded_flag) -> const MStyle&
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
