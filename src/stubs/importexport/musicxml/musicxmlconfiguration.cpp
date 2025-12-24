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
#include "musicxmlconfiguration.h"

#include "log.h"
#include "thirdparty/kors_logger/src/log_base.h"

using namespace mu;
using namespace muse;
using namespace mu::iex::musicxml;

bool MusicXmlConfiguration::importBreaks() const
{
    return false;
}

void MusicXmlConfiguration::setImportBreaks(bool)
{
    NOT_IMPLEMENTED;
}

async::Channel<bool> MusicXmlConfiguration::importBreaksChanged() const
{
    return {};
}

bool MusicXmlConfiguration::importLayout() const
{
    return false;
}

void MusicXmlConfiguration::setImportLayout(bool)
{
    NOT_IMPLEMENTED;
}

async::Channel<bool> MusicXmlConfiguration::importLayoutChanged() const
{
    return {};
}

bool MusicXmlConfiguration::exportLayout() const
{
    return false;
}

void MusicXmlConfiguration::setExportLayout(bool)
{
    NOT_IMPLEMENTED;
}

bool MusicXmlConfiguration::exportMu3Compat() const
{
    return false;
}

void MusicXmlConfiguration::setExportMu3Compat(bool)
{
    NOT_IMPLEMENTED;
}

MusicXmlConfiguration::MusicXmlExportBreaksType MusicXmlConfiguration::exportBreaksType() const
{
    return MusicXmlExportBreaksType::No;
}

void MusicXmlConfiguration::setExportBreaksType(MusicXmlExportBreaksType)
{
    NOT_IMPLEMENTED;
}

bool MusicXmlConfiguration::exportInvisibleElements() const
{
    return false;
}

void MusicXmlConfiguration::setExportInvisibleElements(bool)
{
    NOT_IMPLEMENTED;
}

bool MusicXmlConfiguration::needUseDefaultFont() const
{
    return false;
}

void MusicXmlConfiguration::setNeedUseDefaultFont(bool)
{
    NOT_IMPLEMENTED;
}

async::Channel<bool> MusicXmlConfiguration::needUseDefaultFontChanged() const
{
    return {};
}

void MusicXmlConfiguration::setNeedUseDefaultFontOverride(std::optional<bool>)
{
    NOT_IMPLEMENTED;
}

bool MusicXmlConfiguration::needAskAboutApplyingNewStyle() const
{
    return false;
}

void MusicXmlConfiguration::setNeedAskAboutApplyingNewStyle(bool)
{
    NOT_IMPLEMENTED;
}

async::Channel<bool> MusicXmlConfiguration::needAskAboutApplyingNewStyleChanged() const
{
    return {};
}

bool MusicXmlConfiguration::inferTextType() const
{
    return false;
}

void MusicXmlConfiguration::setInferTextType(bool)
{
    NOT_IMPLEMENTED;
}

async::Channel<bool> MusicXmlConfiguration::inferTextTypeChanged() const
{
    return {};
}

void MusicXmlConfiguration::setInferTextTypeOverride(std::optional<bool>)
{
    NOT_IMPLEMENTED;
}
