/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

// Persists each Encore import option in Settings and emits change channels on update.

#include "enc-importconfiguration.h"

#include "settings.h"

using namespace mu;
using namespace muse;
using namespace mu::iex::enc;

static const std::string module_name("iex_encore");

static const Settings::Key ENC_IMPORT_PAGE_LAYOUT_KEY(module_name, "import/encore/importPageLayout");
static const Settings::Key ENC_IMPORT_PAGE_BREAKS_KEY(module_name, "import/encore/importPageBreaks");
static const Settings::Key ENC_IMPORT_SYSTEM_LOCKS_KEY(module_name, "import/encore/importSystemLocks");
static const Settings::Key ENC_IMPORT_STAFF_SIZE_KEY(module_name, "import/encore/importStaffSize");
static const Settings::Key ENC_IMPORT_TEMPO_SEMANTIC_KEY(module_name, "import/encore/importTempoTextSemantic");
static const Settings::Key ENC_IMPORT_ARTIC_AS_TEXT_KEY(module_name, "import/encore/importUnsupportedArticulationsAsText");
static const Settings::Key ENC_INSTRUMENT_SEARCH_MODE_KEY(module_name, "import/encore/instrumentSearchMode");
static const Settings::Key ENC_UNDERFILL_STRATEGY_KEY(module_name, "import/encore/underfillMeasureStrategy");
static const Settings::Key ENC_OVERFILL_STRATEGY_KEY(module_name, "import/encore/overfillMeasureStrategy");
static const Settings::Key ENC_FIRST_MEASURE_PICKUP_KEY(module_name, "import/encore/firstMeasureIsPickup");
static const Settings::Key ENC_MERGE_VOICES_KEY(module_name, "import/encore/mergeVoices");

void EncImportConfiguration::init()
{
    settings()->setDefaultValue(ENC_IMPORT_PAGE_LAYOUT_KEY, Val(true));
    settings()->valueChanged(ENC_IMPORT_PAGE_LAYOUT_KEY).onReceive(this, [this](const Val& val) {
        m_importPageLayoutChanged.send(val.toBool());
    });

    settings()->setDefaultValue(ENC_IMPORT_PAGE_BREAKS_KEY, Val(true));
    settings()->valueChanged(ENC_IMPORT_PAGE_BREAKS_KEY).onReceive(this, [this](const Val& val) {
        m_importPageBreaksChanged.send(val.toBool());
    });

    settings()->setDefaultValue(ENC_IMPORT_SYSTEM_LOCKS_KEY, Val(true));
    settings()->valueChanged(ENC_IMPORT_SYSTEM_LOCKS_KEY).onReceive(this, [this](const Val& val) {
        m_importSystemLocksChanged.send(val.toBool());
    });

    settings()->setDefaultValue(ENC_IMPORT_STAFF_SIZE_KEY, Val(true));
    settings()->valueChanged(ENC_IMPORT_STAFF_SIZE_KEY).onReceive(this, [this](const Val& val) {
        m_importStaffSizeChanged.send(val.toBool());
    });

    settings()->setDefaultValue(ENC_IMPORT_TEMPO_SEMANTIC_KEY, Val(true));
    settings()->valueChanged(ENC_IMPORT_TEMPO_SEMANTIC_KEY).onReceive(this, [this](const Val& val) {
        m_importTempoTextSemanticChanged.send(val.toBool());
    });

    settings()->setDefaultValue(ENC_IMPORT_ARTIC_AS_TEXT_KEY, Val(false));
    settings()->valueChanged(ENC_IMPORT_ARTIC_AS_TEXT_KEY).onReceive(this, [this](const Val& val) {
        m_importUnsupportedArticulationsAsTextChanged.send(val.toBool());
    });

    settings()->setDefaultValue(ENC_INSTRUMENT_SEARCH_MODE_KEY,
                                Val(static_cast<int>(InstrumentSearchMode::NameAndMidi)));
    settings()->valueChanged(ENC_INSTRUMENT_SEARCH_MODE_KEY).onReceive(this, [this](const Val& val) {
        m_instrumentSearchModeChanged.send(static_cast<InstrumentSearchMode>(val.toInt()));
    });

    // Shipped GUI defaults, authoritative for real imports. They intentionally differ from the
    // EncImportOptions struct initializers (test-only fallbacks); do not assume the two should match.
    settings()->setDefaultValue(ENC_UNDERFILL_STRATEGY_KEY, Val(static_cast<int>(UnderfillStrategy::IrregularMeasure)));
    settings()->valueChanged(ENC_UNDERFILL_STRATEGY_KEY).onReceive(this, [this](const Val& val) {
        m_underfillMeasureStrategyChanged.send(static_cast<UnderfillStrategy>(val.toInt()));
    });

    settings()->setDefaultValue(ENC_OVERFILL_STRATEGY_KEY, Val(static_cast<int>(OverfillStrategy::IrregularMeasure)));
    settings()->valueChanged(ENC_OVERFILL_STRATEGY_KEY).onReceive(this, [this](const Val& val) {
        m_overfillMeasureStrategyChanged.send(static_cast<OverfillStrategy>(val.toInt()));
    });

    settings()->setDefaultValue(ENC_FIRST_MEASURE_PICKUP_KEY, Val(true));
    settings()->valueChanged(ENC_FIRST_MEASURE_PICKUP_KEY).onReceive(this, [this](const Val& val) {
        m_firstMeasureIsPickupChanged.send(val.toBool());
    });

    settings()->setDefaultValue(ENC_MERGE_VOICES_KEY, Val(true));
    settings()->valueChanged(ENC_MERGE_VOICES_KEY).onReceive(this, [this](const Val& val) {
        m_mergeVoicesChanged.send(val.toBool());
    });
}

bool EncImportConfiguration::importPageLayout() const
{
    return settings()->value(ENC_IMPORT_PAGE_LAYOUT_KEY).toBool();
}

void EncImportConfiguration::setImportPageLayout(bool value)
{
    settings()->setSharedValue(ENC_IMPORT_PAGE_LAYOUT_KEY, Val(value));
}

async::Channel<bool> EncImportConfiguration::importPageLayoutChanged() const
{
    return m_importPageLayoutChanged;
}

bool EncImportConfiguration::importPageBreaks() const
{
    return settings()->value(ENC_IMPORT_PAGE_BREAKS_KEY).toBool();
}

void EncImportConfiguration::setImportPageBreaks(bool value)
{
    settings()->setSharedValue(ENC_IMPORT_PAGE_BREAKS_KEY, Val(value));
}

async::Channel<bool> EncImportConfiguration::importPageBreaksChanged() const
{
    return m_importPageBreaksChanged;
}

bool EncImportConfiguration::importSystemLocks() const
{
    return settings()->value(ENC_IMPORT_SYSTEM_LOCKS_KEY).toBool();
}

void EncImportConfiguration::setImportSystemLocks(bool value)
{
    settings()->setSharedValue(ENC_IMPORT_SYSTEM_LOCKS_KEY, Val(value));
}

async::Channel<bool> EncImportConfiguration::importSystemLocksChanged() const
{
    return m_importSystemLocksChanged;
}

bool EncImportConfiguration::importStaffSize() const
{
    return settings()->value(ENC_IMPORT_STAFF_SIZE_KEY).toBool();
}

void EncImportConfiguration::setImportStaffSize(bool value)
{
    settings()->setSharedValue(ENC_IMPORT_STAFF_SIZE_KEY, Val(value));
}

async::Channel<bool> EncImportConfiguration::importStaffSizeChanged() const
{
    return m_importStaffSizeChanged;
}

bool EncImportConfiguration::importTempoTextSemantic() const
{
    return settings()->value(ENC_IMPORT_TEMPO_SEMANTIC_KEY).toBool();
}

void EncImportConfiguration::setImportTempoTextSemantic(bool value)
{
    settings()->setSharedValue(ENC_IMPORT_TEMPO_SEMANTIC_KEY, Val(value));
}

async::Channel<bool> EncImportConfiguration::importTempoTextSemanticChanged() const
{
    return m_importTempoTextSemanticChanged;
}

bool EncImportConfiguration::importUnsupportedArticulationsAsText() const
{
    return settings()->value(ENC_IMPORT_ARTIC_AS_TEXT_KEY).toBool();
}

void EncImportConfiguration::setImportUnsupportedArticulationsAsText(bool value)
{
    settings()->setSharedValue(ENC_IMPORT_ARTIC_AS_TEXT_KEY, Val(value));
}

async::Channel<bool> EncImportConfiguration::importUnsupportedArticulationsAsTextChanged() const
{
    return m_importUnsupportedArticulationsAsTextChanged;
}

InstrumentSearchMode EncImportConfiguration::instrumentSearchMode() const
{
    return static_cast<InstrumentSearchMode>(
        settings()->value(ENC_INSTRUMENT_SEARCH_MODE_KEY).toInt());
}

void EncImportConfiguration::setInstrumentSearchMode(InstrumentSearchMode value)
{
    settings()->setSharedValue(ENC_INSTRUMENT_SEARCH_MODE_KEY, Val(static_cast<int>(value)));
}

async::Channel<InstrumentSearchMode> EncImportConfiguration::instrumentSearchModeChanged() const
{
    return m_instrumentSearchModeChanged;
}

UnderfillStrategy EncImportConfiguration::underfillMeasureStrategy() const
{
    return static_cast<UnderfillStrategy>(settings()->value(ENC_UNDERFILL_STRATEGY_KEY).toInt());
}

void EncImportConfiguration::setUnderfillMeasureStrategy(UnderfillStrategy value)
{
    settings()->setSharedValue(ENC_UNDERFILL_STRATEGY_KEY, Val(static_cast<int>(value)));
}

async::Channel<UnderfillStrategy> EncImportConfiguration::underfillMeasureStrategyChanged() const
{
    return m_underfillMeasureStrategyChanged;
}

OverfillStrategy EncImportConfiguration::overfillMeasureStrategy() const
{
    return static_cast<OverfillStrategy>(settings()->value(ENC_OVERFILL_STRATEGY_KEY).toInt());
}

void EncImportConfiguration::setOverfillMeasureStrategy(OverfillStrategy value)
{
    settings()->setSharedValue(ENC_OVERFILL_STRATEGY_KEY, Val(static_cast<int>(value)));
}

async::Channel<OverfillStrategy> EncImportConfiguration::overfillMeasureStrategyChanged() const
{
    return m_overfillMeasureStrategyChanged;
}

bool EncImportConfiguration::firstMeasureIsPickup() const
{
    return settings()->value(ENC_FIRST_MEASURE_PICKUP_KEY).toBool();
}

void EncImportConfiguration::setFirstMeasureIsPickup(bool value)
{
    settings()->setSharedValue(ENC_FIRST_MEASURE_PICKUP_KEY, Val(value));
}

async::Channel<bool> EncImportConfiguration::firstMeasureIsPickupChanged() const
{
    return m_firstMeasureIsPickupChanged;
}

bool EncImportConfiguration::mergeVoices() const
{
    return settings()->value(ENC_MERGE_VOICES_KEY).toBool();
}

void EncImportConfiguration::setMergeVoices(bool value)
{
    settings()->setSharedValue(ENC_MERGE_VOICES_KEY, Val(value));
}

async::Channel<bool> EncImportConfiguration::mergeVoicesChanged() const
{
    return m_mergeVoicesChanged;
}
