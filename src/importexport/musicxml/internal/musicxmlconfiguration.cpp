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
#include "musicxmlconfiguration.h"

#include "settings.h"
#include "translation.h"

using namespace mu;
using namespace muse;
using namespace mu::iex::musicxml;

static const std::string module_name("iex_musicxml");

static const Settings::Key MUSICXML_IMPORT_BREAKS_KEY(module_name, "import/musicXml/importBreaks");
static const Settings::Key MUSICXML_IMPORT_LAYOUT_KEY(module_name, "import/musicXml/importLayout");
static const Settings::Key MUSICXML_EXPORT_LAYOUT_KEY(module_name, "export/musicXml/exportLayout");
static const Settings::Key MUSICXML_EXPORT_MU3_COMPAT_KEY(module_name, "export/musicXml/exportMu3Compat");
static const Settings::Key MUSICXML_EXPORT_BREAKS_TYPE_KEY(module_name, "export/musicXml/exportBreaks");
static const Settings::Key MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY(module_name, "export/musicXml/exportInvisibleElements");
static const Settings::Key MIGRATION_APPLY_EDWIN_FOR_XML(module_name, "import/compatibility/apply_edwin_for_xml");
static const Settings::Key MIGRATION_NOT_ASK_AGAIN_KEY(module_name, "import/compatibility/do_not_ask_me_again");
static const Settings::Key MUSICXML_IMPORT_INFER_TEXT_TYPE(module_name, "import/musicXml/importInferTextType");

void MusicXmlConfiguration::init()
{
    settings()->setDefaultValue(MUSICXML_IMPORT_BREAKS_KEY, Val(true));
    settings()->valueChanged(MUSICXML_IMPORT_BREAKS_KEY).onReceive(this, [this](const Val& val) {
        m_importBreaksChanged.send(val.toBool());
    });

    settings()->setDefaultValue(MUSICXML_IMPORT_LAYOUT_KEY, Val(true));
    settings()->valueChanged(MUSICXML_IMPORT_LAYOUT_KEY).onReceive(this, [this](const Val& val) {
        m_importLayoutChanged.send(val.toBool());
    });

    settings()->setDefaultValue(MUSICXML_EXPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_MU3_COMPAT_KEY, Val(false));
    settings()->setDescription(MUSICXML_EXPORT_MU3_COMPAT_KEY,
                               //: Means that less information will be included in exported MusicXML files,
                               //: to prevent errors when importing them into MuseScore 3
                               muse::trc("iex_musicxml", "Limit MusicXML export for compatibility with MuseScore 3"));
    settings()->setCanBeManuallyEdited(MUSICXML_EXPORT_MU3_COMPAT_KEY, true);
    settings()->setDefaultValue(MUSICXML_EXPORT_BREAKS_TYPE_KEY, Val(MusicXmlExportBreaksType::All));
    settings()->setDefaultValue(MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY, Val(false));
    settings()->setDescription(MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY,
                               muse::trc("iex_musicxml", "Export invisible elements to MusicXML"));
    settings()->setCanBeManuallyEdited(MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY, true);

    settings()->setDefaultValue(MIGRATION_APPLY_EDWIN_FOR_XML, Val(false));
    settings()->valueChanged(MIGRATION_APPLY_EDWIN_FOR_XML).onReceive(this, [this](const Val& val) {
        m_needUseDefaultFontChanged.send(val.toBool());
    });

    settings()->setDefaultValue(MIGRATION_NOT_ASK_AGAIN_KEY, Val(false));
    settings()->valueChanged(MIGRATION_NOT_ASK_AGAIN_KEY).onReceive(this, [this](const Val& val) {
        m_needAskAboutApplyingNewStyleChanged.send(val.toBool());
    });

    settings()->setDefaultValue(MUSICXML_IMPORT_INFER_TEXT_TYPE, Val(false));
    settings()->valueChanged(MUSICXML_IMPORT_INFER_TEXT_TYPE).onReceive(this, [this](const Val& val) {
        m_inferTextTypeChanged.send(val.toBool());
    });
}

bool MusicXmlConfiguration::importBreaks() const
{
    return settings()->value(MUSICXML_IMPORT_BREAKS_KEY).toBool();
}

void MusicXmlConfiguration::setImportBreaks(bool value)
{
    settings()->setSharedValue(MUSICXML_IMPORT_BREAKS_KEY, Val(value));
}

async::Channel<bool> MusicXmlConfiguration::importBreaksChanged() const
{
    return m_importBreaksChanged;
}

bool MusicXmlConfiguration::importLayout() const
{
    return settings()->value(MUSICXML_IMPORT_LAYOUT_KEY).toBool();
}

void MusicXmlConfiguration::setImportLayout(bool value)
{
    settings()->setSharedValue(MUSICXML_IMPORT_LAYOUT_KEY, Val(value));
}

async::Channel<bool> MusicXmlConfiguration::importLayoutChanged() const
{
    return m_importLayoutChanged;
}

bool MusicXmlConfiguration::exportLayout() const
{
    return settings()->value(MUSICXML_EXPORT_LAYOUT_KEY).toBool();
}

void MusicXmlConfiguration::setExportLayout(bool value)
{
    settings()->setSharedValue(MUSICXML_EXPORT_LAYOUT_KEY, Val(value));
}

bool MusicXmlConfiguration::exportMu3Compat() const
{
    return settings()->value(MUSICXML_EXPORT_MU3_COMPAT_KEY).toBool();
}

void MusicXmlConfiguration::setExportMu3Compat(bool value)
{
    settings()->setSharedValue(MUSICXML_EXPORT_MU3_COMPAT_KEY, Val(value));
}

MusicXmlConfiguration::MusicXmlExportBreaksType MusicXmlConfiguration::exportBreaksType() const
{
    return settings()->value(MUSICXML_EXPORT_BREAKS_TYPE_KEY).toEnum<MusicXmlExportBreaksType>();
}

void MusicXmlConfiguration::setExportBreaksType(MusicXmlExportBreaksType breaksType)
{
    settings()->setSharedValue(MUSICXML_EXPORT_BREAKS_TYPE_KEY, Val(breaksType));
}

bool MusicXmlConfiguration::exportInvisibleElements() const
{
    return settings()->value(MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY).toBool();
}

void MusicXmlConfiguration::setExportInvisibleElements(bool value)
{
    settings()->setSharedValue(MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY, Val(value));
}

bool MusicXmlConfiguration::needUseDefaultFont() const
{
    if (m_needUseDefaultFontOverride.has_value()) {
        return m_needUseDefaultFontOverride.value();
    }

    return settings()->value(MIGRATION_APPLY_EDWIN_FOR_XML).toBool();
}

void MusicXmlConfiguration::setNeedUseDefaultFont(bool value)
{
    settings()->setSharedValue(MIGRATION_APPLY_EDWIN_FOR_XML, Val(value));
}

async::Channel<bool> MusicXmlConfiguration::needUseDefaultFontChanged() const
{
    return m_needUseDefaultFontChanged;
}

void MusicXmlConfiguration::setNeedUseDefaultFontOverride(std::optional<bool> value)
{
    m_needUseDefaultFontOverride = value;
}

bool MusicXmlConfiguration::needAskAboutApplyingNewStyle() const
{
    return !settings()->value(MIGRATION_NOT_ASK_AGAIN_KEY).toBool();
}

void MusicXmlConfiguration::setNeedAskAboutApplyingNewStyle(bool value)
{
    settings()->setSharedValue(MIGRATION_NOT_ASK_AGAIN_KEY, Val(!value));
}

async::Channel<bool> MusicXmlConfiguration::needAskAboutApplyingNewStyleChanged() const
{
    return m_needAskAboutApplyingNewStyleChanged;
}

bool MusicXmlConfiguration::inferTextType() const
{
    if (m_inferTextTypeOverride.has_value()) {
        return m_inferTextTypeOverride.value();
    }

    return settings()->value(MUSICXML_IMPORT_INFER_TEXT_TYPE).toBool();
}

void MusicXmlConfiguration::setInferTextType(bool value)
{
    settings()->setSharedValue(MUSICXML_IMPORT_INFER_TEXT_TYPE, Val(value));
}

async::Channel<bool> MusicXmlConfiguration::inferTextTypeChanged() const
{
    return m_inferTextTypeChanged;
}

void MusicXmlConfiguration::setInferTextTypeOverride(std::optional<bool> value)
{
    m_inferTextTypeOverride = value;
}
