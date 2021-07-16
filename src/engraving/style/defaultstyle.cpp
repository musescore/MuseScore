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

using namespace mu::engraving;
using namespace Ms;

static const int LEGACY_MSC_VERSION_V3 = 301;
static const int LEGACY_MSC_VERSION_V2 = 206;
static const int LEGACY_MSC_VERSION_V1 = 114;

DefaultStyle* DefaultStyle::instance()
{
    static DefaultStyle s;
    return &s;
}

const MStyle* styleDefaults301()
{
    static MStyle* result = nullptr;

    if (result) {
        return result;
    }

    result = new MStyle();

    QFile baseDefaults(":/styles/legacy-style-defaults-v3.mss");

    if (!baseDefaults.open(QIODevice::ReadOnly)) {
        return result;
    }

    result->load(&baseDefaults);

    return result;
}

const MStyle* styleDefaults206()
{
    static MStyle* result = nullptr;

    if (result) {
        return result;
    }

    result = new MStyle();
    QFile baseDefaults(":/styles/legacy-style-defaults-v2.mss");

    if (!baseDefaults.open(QIODevice::ReadOnly)) {
        return result;
    }

    result->load(&baseDefaults);

    return result;
}

const MStyle* styleDefaults114()
{
    static MStyle* result = nullptr;

    if (result) {
        return result;
    }

    result = new MStyle();
    QFile baseDefaults(":/styles/legacy-style-defaults-v1.mss");

    if (!baseDefaults.open(QIODevice::ReadOnly)) {
        return result;
    }

    result->load(&baseDefaults);

    return result;
}

void DefaultStyle::init(const QString& defaultSyleFilePath, const QString& partStyleFilePath)
{
    m_baseStyle.precomputeValues();
    m_defaultStyle.precomputeValues();

    readDefaultStyle(defaultSyleFilePath);
    readPartStyle(partStyleFilePath);
}

bool DefaultStyle::readDefaultStyle(QString file)
{
    if (file.isEmpty()) {
        return false;
    }
    MStyle style = defaultStyle();
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    bool rv = style.load(&f, true);
    if (rv) {
        m_defaultStyle = style;
    }
    f.close();
    return rv;
}

bool DefaultStyle::readPartStyle(QString filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    m_defaultStyleForParts = new MStyle(m_defaultStyle);
    bool rv = m_defaultStyleForParts->load(&file, true);
    if (rv) {
        m_defaultStyleForParts->precomputeValues();
    }
    file.close();
    return rv;
}

const MStyle* DefaultStyle::resolveStyleDefaults(const int defaultsVersion)
{
    switch (defaultsVersion) {
    case LEGACY_MSC_VERSION_V3:
        return styleDefaults301();
    case LEGACY_MSC_VERSION_V2:
        return styleDefaults206();
    case LEGACY_MSC_VERSION_V1:
        return styleDefaults114();
    default:
        return &baseStyle();
    }
}
