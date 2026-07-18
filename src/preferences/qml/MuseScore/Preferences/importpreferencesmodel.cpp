/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "importpreferencesmodel.h"

#include <QTextCodec>

#include "engraving/types/constants.h"

#include "translation.h"

using namespace mu::preferences;

ImportPreferencesModel::ImportPreferencesModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void ImportPreferencesModel::load()
{
    notationConfiguration()->styleFileImportPathChanged().onReceive(this, [this](const std::string& val) {
        emit styleFileImportPathChanged(QString::fromStdString(val));
    });

    oveConfiguration()->importOvertureCharsetChanged().onReceive(this, [this](const std::string& val) {
        emit currentOvertureCharsetChanged(QString::fromStdString(val));
    });

    musicXmlConfiguration()->importLayoutChanged().onReceive(this, [this](bool val) {
        emit importLayoutChanged(val);
    });

    musicXmlConfiguration()->importBreaksChanged().onReceive(this, [this](bool val) {
        emit importBreaksChanged(val);
    });

    musicXmlConfiguration()->needUseDefaultFontChanged().onReceive(this, [this](bool val) {
        emit needUseDefaultFontChanged(val);
    });

    musicXmlConfiguration()->inferTextTypeChanged().onReceive(this, [this](bool val) {
        emit inferTextTypeChanged(val);
    });

    midiImportExportConfiguration()->midiShortestNoteChanged().onReceive(this, [this](int val) {
        emit currentShortestNoteChanged(val);
    });

    midiImportExportConfiguration()->roundTempoChanged().onReceive(this, [this](bool val) {
        emit roundTempoChanged(val);
    });

    meiConfiguration()->meiImportLayoutChanged().onReceive(this, [this](bool val) {
        emit meiImportLayoutChanged(val);
    });

    musicXmlConfiguration()->needAskAboutApplyingNewStyleChanged().onReceive(this, [this](bool val) {
        emit needAskAboutApplyingNewStyleChanged(val);
    });

    encoreConfiguration()->importPageLayoutChanged().onReceive(this, [this](bool val) {
        emit encoreImportPageLayoutChanged(val);
    });

    encoreConfiguration()->importPageBreaksChanged().onReceive(this, [this](bool val) {
        emit encoreImportPageBreaksChanged(val);
    });

    encoreConfiguration()->importSystemLocksChanged().onReceive(this, [this](bool val) {
        emit encoreImportSystemLocksChanged(val);
    });

    encoreConfiguration()->importStaffSizeChanged().onReceive(this, [this](bool val) {
        emit encoreImportStaffSizeChanged(val);
    });

    encoreConfiguration()->importTempoTextSemanticChanged().onReceive(this, [this](bool val) {
        emit encoreImportTempoTextSemanticChanged(val);
    });

    encoreConfiguration()->importUnsupportedArticulationsAsTextChanged().onReceive(this, [this](bool val) {
        emit encoreImportUnsupportedArticulationsAsTextChanged(val);
    });

    encoreConfiguration()->firstMeasureIsPickupChanged().onReceive(this, [this](bool val) {
        emit encoreFirstMeasureIsPickupChanged(val);
    });

    encoreConfiguration()->mergeVoicesChanged().onReceive(this, [this](bool val) {
        emit encoreMergeVoicesChanged(val);
    });

    encoreConfiguration()->instrumentSearchModeChanged().onReceive(this, [this](iex::enc::InstrumentSearchMode val) {
        emit encoreInstrumentSearchModeChanged(static_cast<int>(val));
    });
    encoreConfiguration()->tablatureImportModeChanged().onReceive(this, [this](iex::enc::TablatureImportMode val) {
        emit encoreTablatureImportModeChanged(static_cast<int>(val));
    });

    encoreConfiguration()->underfillMeasureStrategyChanged().onReceive(this, [this](iex::enc::UnderfillStrategy val) {
        emit encoreUnderfillStrategyChanged(static_cast<int>(val));
    });

    encoreConfiguration()->overfillMeasureStrategyChanged().onReceive(this, [this](iex::enc::OverfillStrategy val) {
        emit encoreOverfillStrategyChanged(static_cast<int>(val));
    });
}

QVariantList ImportPreferencesModel::charsets() const
{
    QList<QByteArray> charsets = QTextCodec::availableCodecs();
    std::sort(charsets.begin(), charsets.end());

    QVariantList result;
    for (QByteArray charset: charsets) {
        result << QString(charset);
    }

    return result;
}

QVariantList ImportPreferencesModel::shortestNotes() const
{
    constexpr int division =  engraving::Constants::DIVISION;

    QVariantList result = {
        QVariantMap { { "title", muse::qtrc("preferences", "Quarter") }, { "value", division } },
        QVariantMap { { "title", muse::qtrc("preferences", "Eighth") }, { "value", division / 2 } },
        QVariantMap { { "title", muse::qtrc("preferences", "16th") }, { "value", division / 4 } },
        QVariantMap { { "title", muse::qtrc("preferences", "32nd") }, { "value", division / 8 } },
        QVariantMap { { "title", muse::qtrc("preferences", "64th") }, { "value", division / 16 } },
        QVariantMap { { "title", muse::qtrc("preferences", "128th") }, { "value", division / 32 } },
        QVariantMap { { "title", muse::qtrc("preferences", "256th") }, { "value", division / 64 } },
        QVariantMap { { "title", muse::qtrc("preferences", "512th") }, { "value", division / 128 } },
        QVariantMap { { "title", muse::qtrc("preferences", "1024th") }, { "value", division / 256 } }
    };

    return result;
}

QStringList ImportPreferencesModel::stylePathFilter() const
{
    return { muse::qtrc("preferences", "MuseScore style file") + " (*.mss)" };
}

QString ImportPreferencesModel::styleChooseTitle() const
{
    return muse::qtrc("preferences", "Choose default style for imports");
}

QString ImportPreferencesModel::fileDirectory(const QString& filePath) const
{
    return muse::io::dirpath(filePath.toStdString()).toQString();
}

QString ImportPreferencesModel::styleFileImportPath() const
{
    return notationConfiguration()->styleFileImportPath().toQString();
}

QString ImportPreferencesModel::currentOvertureCharset() const
{
    return QString::fromStdString(oveConfiguration()->importOvertureCharset());
}

bool ImportPreferencesModel::importLayout() const
{
    return musicXmlConfiguration()->importLayout();
}

bool ImportPreferencesModel::importBreaks() const
{
    return musicXmlConfiguration()->importBreaks();
}

bool ImportPreferencesModel::needUseDefaultFont() const
{
    return musicXmlConfiguration()->needUseDefaultFont();
}

bool ImportPreferencesModel::inferTextType() const
{
    return musicXmlConfiguration()->inferTextType();
}

int ImportPreferencesModel::currentShortestNote() const
{
    return midiImportExportConfiguration()->midiShortestNote();
}

bool ImportPreferencesModel::roundTempo() const
{
    return midiImportExportConfiguration()->roundTempo();
}

bool ImportPreferencesModel::needAskAboutApplyingNewStyle() const
{
    return musicXmlConfiguration()->needAskAboutApplyingNewStyle();
}

bool ImportPreferencesModel::meiImportLayout() const
{
    return meiConfiguration()->meiImportLayout();
}

bool ImportPreferencesModel::mnxRequireExactSchemaValidation() const
{
    return mnxConfiguration()->mnxRequireExactSchemaValidation();
}

void ImportPreferencesModel::setStyleFileImportPath(QString path)
{
    if (path == styleFileImportPath()) {
        return;
    }

    notationConfiguration()->setStyleFileImportPath(path.toStdString());
}

void ImportPreferencesModel::setCurrentOvertureCharset(QString charset)
{
    if (charset == currentOvertureCharset()) {
        return;
    }

    oveConfiguration()->setImportOvertureCharset(charset.toStdString());
}

void ImportPreferencesModel::setImportLayout(bool import)
{
    if (import == importLayout()) {
        return;
    }

    musicXmlConfiguration()->setImportLayout(import);
}

void ImportPreferencesModel::setImportBreaks(bool import)
{
    if (import == importBreaks()) {
        return;
    }

    musicXmlConfiguration()->setImportBreaks(import);
}

void ImportPreferencesModel::setNeedUseDefaultFont(bool value)
{
    if (value == needUseDefaultFont()) {
        return;
    }

    musicXmlConfiguration()->setNeedUseDefaultFont(value);
}

void ImportPreferencesModel::setInferTextType(bool value)
{
    if (value == inferTextType()) {
        return;
    }

    musicXmlConfiguration()->setInferTextType(value);
}

void ImportPreferencesModel::setCurrentShortestNote(int note)
{
    if (note == currentShortestNote()) {
        return;
    }

    midiImportExportConfiguration()->setMidiShortestNote(note);
}

void ImportPreferencesModel::setRoundTempo(bool value)
{
    if (value == roundTempo()) {
        return;
    }

    midiImportExportConfiguration()->setRoundTempo(value);
}

void ImportPreferencesModel::setNeedAskAboutApplyingNewStyle(bool value)
{
    if (value == needAskAboutApplyingNewStyle()) {
        return;
    }

    musicXmlConfiguration()->setNeedAskAboutApplyingNewStyle(value);
}

void ImportPreferencesModel::setMeiImportLayout(bool import)
{
    if (import == meiImportLayout()) {
        return;
    }

    meiConfiguration()->setMeiImportLayout(import);
}

void ImportPreferencesModel::setMnxRequireExactSchemaValidation(bool value)
{
    if (value == mnxRequireExactSchemaValidation()) {
        return;
    }

    mnxConfiguration()->setMnxRequireExactSchemaValidation(value);
    emit mnxRequireExactSchemaValidationChanged(value);
}

QVariantList ImportPreferencesModel::encoreInstrumentSearchModeModel() const
{
    return {
        QVariantMap { { "title", muse::qtrc("preferences", "Try Name first, then MIDI") },
            { "value", static_cast<int>(iex::enc::InstrumentSearchMode::NameAndMidi) } },
        QVariantMap { { "title", muse::qtrc("preferences", "Use MIDI program only") },
            { "value", static_cast<int>(iex::enc::InstrumentSearchMode::MidiOnly) } },
        QVariantMap { { "title", muse::qtrc("preferences", "Set Grand Piano for all staves") },
            { "value", static_cast<int>(iex::enc::InstrumentSearchMode::Piano) } },
    };
}

QVariantList ImportPreferencesModel::encoreUnderfillStrategyModel() const
{
    QVariantList result = {
        QVariantMap { { "title", muse::qtrc("preferences", "Reduce measure time (irregular)") },
            { "value", static_cast<int>(iex::enc::UnderfillStrategy::IrregularMeasure) } },
        QVariantMap { { "title", muse::qtrc("preferences", "Fill with rests") },
            { "value", static_cast<int>(iex::enc::UnderfillStrategy::VisibleRests) } },
        QVariantMap { { "title", muse::qtrc("preferences", "Fill with invisible rests") },
            { "value", static_cast<int>(iex::enc::UnderfillStrategy::InvisibleRests) } }
    };

    return result;
}

QVariantList ImportPreferencesModel::encoreOverfillStrategyModel() const
{
    QVariantList result = {
        QVariantMap { { "title", muse::qtrc("preferences", "Enlarge measure time (irregular)") },
            { "value", static_cast<int>(iex::enc::OverfillStrategy::IrregularMeasure) } },
        QVariantMap { { "title", muse::qtrc("preferences", "Remove last notes") },
            { "value", static_cast<int>(iex::enc::OverfillStrategy::Truncate) } },
        QVariantMap { { "title", muse::qtrc("preferences", "Stretch last notes") },
            { "value", static_cast<int>(iex::enc::OverfillStrategy::StretchLastNote) } }
    };

    return result;
}

QVariantList ImportPreferencesModel::encoreTablatureImportModeModel() const
{
    return {
        QVariantMap { { "title", muse::qtrc("preferences", "Link tablature to its notation staff") },
            { "value", static_cast<int>(iex::enc::TablatureImportMode::Linked) } },
        QVariantMap { { "title", muse::qtrc("preferences", "Keep tablature as a separate staff") },
            { "value", static_cast<int>(iex::enc::TablatureImportMode::Separate) } },
        QVariantMap { { "title", muse::qtrc("preferences", "Ignore tablature staves") },
            { "value", static_cast<int>(iex::enc::TablatureImportMode::Ignore) } },
    };
}

bool ImportPreferencesModel::encoreImportPageLayout() const
{
    return encoreConfiguration()->importPageLayout();
}

void ImportPreferencesModel::setEncoreImportPageLayout(bool value)
{
    if (value == encoreImportPageLayout()) {
        return;
    }

    encoreConfiguration()->setImportPageLayout(value);
}

bool ImportPreferencesModel::encoreImportPageBreaks() const
{
    return encoreConfiguration()->importPageBreaks();
}

void ImportPreferencesModel::setEncoreImportPageBreaks(bool value)
{
    if (value == encoreImportPageBreaks()) {
        return;
    }

    encoreConfiguration()->setImportPageBreaks(value);
}

bool ImportPreferencesModel::encoreImportSystemLocks() const
{
    return encoreConfiguration()->importSystemLocks();
}

void ImportPreferencesModel::setEncoreImportSystemLocks(bool value)
{
    if (value == encoreImportSystemLocks()) {
        return;
    }

    encoreConfiguration()->setImportSystemLocks(value);
}

bool ImportPreferencesModel::encoreImportStaffSize() const
{
    return encoreConfiguration()->importStaffSize();
}

void ImportPreferencesModel::setEncoreImportStaffSize(bool value)
{
    if (value == encoreImportStaffSize()) {
        return;
    }

    encoreConfiguration()->setImportStaffSize(value);
}

bool ImportPreferencesModel::encoreImportTempoTextSemantic() const
{
    return encoreConfiguration()->importTempoTextSemantic();
}

void ImportPreferencesModel::setEncoreImportTempoTextSemantic(bool value)
{
    if (value == encoreImportTempoTextSemantic()) {
        return;
    }

    encoreConfiguration()->setImportTempoTextSemantic(value);
}

bool ImportPreferencesModel::encoreImportUnsupportedArticulationsAsText() const
{
    return encoreConfiguration()->importUnsupportedArticulationsAsText();
}

void ImportPreferencesModel::setEncoreImportUnsupportedArticulationsAsText(bool value)
{
    if (value == encoreImportUnsupportedArticulationsAsText()) {
        return;
    }

    encoreConfiguration()->setImportUnsupportedArticulationsAsText(value);
}

int ImportPreferencesModel::encoreInstrumentSearchMode() const
{
    return static_cast<int>(encoreConfiguration()->instrumentSearchMode());
}

void ImportPreferencesModel::setEncoreInstrumentSearchMode(int value)
{
    if (value == encoreInstrumentSearchMode()) {
        return;
    }

    encoreConfiguration()->setInstrumentSearchMode(static_cast<iex::enc::InstrumentSearchMode>(value));
}

int ImportPreferencesModel::encoreTablatureImportMode() const
{
    return static_cast<int>(encoreConfiguration()->tablatureImportMode());
}

void ImportPreferencesModel::setEncoreTablatureImportMode(int value)
{
    if (value == encoreTablatureImportMode()) {
        return;
    }

    encoreConfiguration()->setTablatureImportMode(static_cast<iex::enc::TablatureImportMode>(value));
}

int ImportPreferencesModel::encoreUnderfillStrategy() const
{
    return static_cast<int>(encoreConfiguration()->underfillMeasureStrategy());
}

void ImportPreferencesModel::setEncoreUnderfillStrategy(int value)
{
    if (value == encoreUnderfillStrategy()) {
        return;
    }

    encoreConfiguration()->setUnderfillMeasureStrategy(static_cast<iex::enc::UnderfillStrategy>(value));
}

int ImportPreferencesModel::encoreOverfillStrategy() const
{
    return static_cast<int>(encoreConfiguration()->overfillMeasureStrategy());
}

void ImportPreferencesModel::setEncoreOverfillStrategy(int value)
{
    if (value == encoreOverfillStrategy()) {
        return;
    }

    encoreConfiguration()->setOverfillMeasureStrategy(static_cast<iex::enc::OverfillStrategy>(value));
}

bool ImportPreferencesModel::encoreFirstMeasureIsPickup() const
{
    return encoreConfiguration()->firstMeasureIsPickup();
}

void ImportPreferencesModel::setEncoreFirstMeasureIsPickup(bool value)
{
    if (value == encoreFirstMeasureIsPickup()) {
        return;
    }

    encoreConfiguration()->setFirstMeasureIsPickup(value);
}

bool ImportPreferencesModel::encoreMergeVoices() const
{
    return encoreConfiguration()->mergeVoices();
}

void ImportPreferencesModel::setEncoreMergeVoices(bool value)
{
    if (value == encoreMergeVoices()) {
        return;
    }

    encoreConfiguration()->setMergeVoices(value);
}
