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
#include "noteinputbarmodel.h"

#include "rcommand/commandtypes.h"
#include "types/translatablestring.h"

#include "notation/inotationelements.h" // IWYU pragma: keep
#include "notation/inotationinteraction.h"
#include "notation/inotationnoteinput.h"
#include "notation/inotationselection.h"
#include "notation/inotationundostack.h"

#include "notationcommands.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::uicomponents;

static const QString TOOLBAR_NAME("noteInput");

const std::string NoteInputBarModel::CROSS_STAFF_BEAMING_SUBITEMS("cross-staff-beaming-subitems");
const std::string NoteInputBarModel::TUPLET_SUBITEMS("tuplets-subitems");

NoteInputBarModel::NoteInputBarModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

QVariant NoteInputBarModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (!isIndexValid(row)) {
        return QVariant();
    }

    const MenuItem* item = items().at(row);
    switch (role) {
    case OrderRole: return row;
    case SectionRole: return item->section();
    }

    return AbstractMenuModel::data(index, role);
}

QHash<int, QByteArray> NoteInputBarModel::roleNames() const
{
    QHash<int, QByteArray> roles = AbstractMenuModel::roleNames();
    roles[OrderRole] = "order";
    roles[SectionRole] = "section";

    return roles;
}

void NoteInputBarModel::classBegin()
{
    init();
}

void NoteInputBarModel::init()
{
    subscribeOnChanges();

    uiState()->toolConfigChanged(TOOLBAR_NAME).onNotify(this, [this]() {
        load();
    });

    commandsController()->isNoteInputAllowedChanged().onReceive(this, [this](bool) {
        emit isInputAllowedChanged();
    });

    load();
}

const muse::ui::ToolConfig& NoteInputBarModel::defaultNoteInputConfig()
{
    static ToolConfig config;
    if (!config.isValid()) {
        config.items = {
            { ToggleNoteInputByNameCommand::id, true },
            { ToggleNoteInputByDurationCommand::id, true },
            { ToggleNoteInputRhythmCommand::id, false },
            { ToggleNoteInputRepitchCommand::id, false },
            { ToggleNoteInputRealtimeAutoCommand::id, false },
            { ToggleNoteInputRealtimeManualCommand::id, false },
            { ToggleNoteInputTimewiseCommand::id, false },
            { ToolConfig::____________, true },
            { SetDuration1024thCommand::id, false },
            { SetDuration512thCommand::id, false },
            { SetDuration256thCommand::id, false },
            { SetDuration128thCommand::id, false },
            { SetDuration64thCommand::id, true },
            { SetDuration32ndCommand::id, true },
            { SetDuration16thCommand::id, true },
            { SetDurationEighthCommand::id, true },
            { SetDurationQuarterCommand::id, true },
            { SetDurationHalfCommand::id, true },
            { SetDurationWholeCommand::id, true },
            { SetDurationBreveCommand::id, false },
            { SetDurationLongaCommand::id, false },
            { ToolConfig::____________, true },
            { ToggleDotCommand::id, true },
            { ToggleDot2Command::id, false },
            { ToggleDot3Command::id, false },
            { ToggleDot4Command::id, false },
            { ToggleRestCommand::id, true },
            { ToolConfig::____________, true },
            { ToggleFlat2Command::id, true },
            { ToggleFlatCommand::id, true },
            { ToggleNaturalCommand::id, true },
            { ToggleSharpCommand::id, true },
            { ToggleSharp2Command::id, true },
            { ToolConfig::____________, true },
            { ToggleTieCommand::id, true },
            { AddSlurCommand::id, true },
            { ToggleLaissezVibrerCommand::id, false },
            { ToolConfig::____________, true },
            { TOGGLE_MARCATO_COMMAND, true },
            { TOGGLE_SFORZATO_COMMAND, true },
            { TOGGLE_TENUTO_COMMAND, true },
            { TOGGLE_STACCATO_COMMAND, true },
            { ToolConfig::____________, true },
            { CROSS_STAFF_BEAMING_SUBITEMS, true }, // service
            { TUPLET_SUBITEMS, true },              // service
            { FlipCommand::id, true },
            { ToolConfig::____________, true },
            { UseVoice1Command::id, true },
            { UseVoice2Command::id, true },
            { UseVoice3Command::id, false },
            { UseVoice4Command::id, false }
        };
    }
    return config;
}

void NoteInputBarModel::load()
{
    MenuItemList items;

    ToolConfig noteInputConfig = uiState()->toolConfig(TOOLBAR_NAME, NoteInputBarModel::defaultNoteInputConfig());

    int section = 0;
    for (const ToolConfig::Item& citem : noteInputConfig.items) {
        if (!citem.show) {
            continue;
        }

        if (citem.isSeparator()) {
            section++;
            continue;
        }

        if (citem.intent == CROSS_STAFF_BEAMING_SUBITEMS) {
            MenuItem* item = makeServiceItem(CROSS_STAFF_BEAMING_SUBITEMS, QString::number(section));
            item->setSubitems(makeCrossStaffBeamingItems());
            items << item;
        } else if (citem.intent == TUPLET_SUBITEMS) {
            MenuItem* item = makeServiceItem(TUPLET_SUBITEMS, QString::number(section));
            item->setSubitems(makeTupletItems());
            items << item;
        } else {
            MenuItem* item = makeCommandItem(rcommand::Command(citem.intent), QString::number(section));
            items << item;
        }
    }

    items << makeAddItem(QString::number(++section));
    setItems(items);
}

bool NoteInputBarModel::isInputAllowed() const
{
    return commandsController()->isNoteInputAllowed();
}

NoteInputBarModel::ServiceItemInfo NoteInputBarModel::serviceItemInfo(const std::string& intent)
{
    if (intent == CROSS_STAFF_BEAMING_SUBITEMS) {
        return { TranslatableString("action", "Cross-staff beaming"), IconCode::Code::CROSS_STAFF_BEAMING };
    } else if (intent == TUPLET_SUBITEMS) {
        return { TranslatableString("action", "Tuplet"), IconCode::Code::NOTE_TUPLET };
    }
    return { };
}

MenuItem* NoteInputBarModel::makeServiceItem(const std::string& intent, const QString& section)
{
    const ServiceItemInfo info = serviceItemInfo(intent);
    MenuItem* item = new MenuItem(this);
    item->setId(QString::fromStdString(intent));
    item->setTitle(info.title);
    item->setIcon(info.icon);
    item->setSection(section);
    return item;
}

MenuItem* NoteInputBarModel::makeCommandItem(const muse::rcommand::Command& command, const QString& section)
{
    const rcommand::CommandInfo info = commandsRegister()->commandInfo(command);
    MenuItem* item = new MenuItem(info, this);
    item->setSection(section);
    return item;
}

MenuItem* NoteInputBarModel::makeAddItem(const QString& section)
{
    MenuItem* item = new MenuItem(this);
    item->setTitle(TranslatableString("global", "Add"));
    item->setIcon(IconCode::Code::PLUS);
    item->setSection(section);
    item->setSubitems(makeAddItems());
    return item;
}

MenuItemList NoteInputBarModel::makeCrossStaffBeamingItems()
{
    MenuItemList items {
        makeMenuItem(MOVE_UP_COMMAND),
        makeMenuItem(MOVE_DOWN_COMMAND)
    };

    return items;
}

MenuItemList NoteInputBarModel::makeTupletItems()
{
    MenuItemList items {
        makeMenuItem(AddDupletCommand::id),
        makeMenuItem(AddTripletCommand::id),
        makeMenuItem(AddQuadrupletCommand::id),
        makeMenuItem(AddQuintupletCommand::id),
        makeMenuItem(AddSextupletCommand::id),
        makeMenuItem(AddSeptupletCommand::id),
        makeMenuItem(AddOctupletCommand::id),
        makeMenuItem(AddNonupletCommand::id),
        makeMenuItem(OpenTupletConfigureCommand::id)
    };

    return items;
}

MenuItemList NoteInputBarModel::makeAddItems()
{
    MenuItemList items {
        makeMenu(TranslatableString("notation", "Notes"), makeNotesItems()),
        makeMenu(TranslatableString("notation", "Intervals"), makeIntervalsItems()),
        makeMenu(TranslatableString("notation", "Measures"), makeMeasuresItems()),
        makeMenu(TranslatableString("notation", "Frames"), makeFramesItems()),
        makeMenu(TranslatableString("notation", "Text"), makeTextItems()),
        makeMenu(TranslatableString("notation", "Lines"), makeLinesItems()),
        makeMenu(TranslatableString("notation", "Chords and fretboard diagrams"), makeChordAndFretboardDiagramsItems()),
    };

    return items;
}

MenuItemList NoteInputBarModel::makeNotesItems()
{
    MenuItemList items {
        makeMenuItem(AddNoteC_ToNextChordCommand::id),
        makeMenuItem(AddNoteD_ToNextChordCommand::id),
        makeMenuItem(AddNoteE_ToNextChordCommand::id),
        makeMenuItem(AddNoteF_ToNextChordCommand::id),
        makeMenuItem(AddNoteG_ToNextChordCommand::id),
        makeMenuItem(AddNoteA_ToNextChordCommand::id),
        makeMenuItem(AddNoteB_ToNextChordCommand::id),
        makeSeparator(),
        makeMenuItem(AddNoteC_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteD_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteE_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteF_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteG_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteA_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteB_ToCurrentChordCommand::id)
    };

    return items;
}

MenuItemList NoteInputBarModel::makeIntervalsItems()
{
    MenuItemList items {
        makeMenuItem(AddInterval1Command::id),
        makeMenuItem(AddInterval2Command::id),
        makeMenuItem(AddInterval3Command::id),
        makeMenuItem(AddInterval4Command::id),
        makeMenuItem(AddInterval5Command::id),
        makeMenuItem(AddInterval6Command::id),
        makeMenuItem(AddInterval7Command::id),
        makeMenuItem(AddInterval8Command::id),
        makeMenuItem(AddInterval9Command::id),
        makeMenuItem(AddInterval10Command::id),
        makeSeparator(),
        makeMenuItem(AddIntervalMinus2Command::id),
        makeMenuItem(AddIntervalMinus3Command::id),
        makeMenuItem(AddIntervalMinus4Command::id),
        makeMenuItem(AddIntervalMinus5Command::id),
        makeMenuItem(AddIntervalMinus6Command::id),
        makeMenuItem(AddIntervalMinus7Command::id),
        makeMenuItem(AddIntervalMinus8Command::id),
        makeMenuItem(AddIntervalMinus9Command::id),
        makeMenuItem(AddIntervalMinus10Command::id)
    };

    return items;
}

MenuItemList NoteInputBarModel::makeMeasuresItems()
{
    MenuItemList items {
        makeMenuItem(InsertMeasureCommand::id),
        makeMenuItem(AppendMeasureCommand::id),
        makeSeparator(),
        makeMenuItem(InsertMeasuresCommand::id),
        makeMenuItem(InsertMeasuresAfterSelectionCommand::id),
        makeSeparator(),
        makeMenuItem(InsertMeasuresAtStartOfScoreCommand::id),
        makeMenuItem(AppendMeasuresCommand::id)
    };

    return items;
}

MenuItemList NoteInputBarModel::makeFramesItems()
{
    MenuItemList items {
        makeMenuItem(InsertHBoxCommand::id),
        makeMenuItem(InsertVBoxCommand::id),
        makeMenuItem(InsertTextFrameCommand::id),
        makeMenuItem(InsertFretFrameCommand::id),
        makeSeparator(),
        makeMenu(TranslatableString("notation", "Insert at end of score"), {
            makeMenuItem(AppendHBoxCommand::id),
            makeMenuItem(AppendVBoxCommand::id),
            makeMenuItem(AppendTextFrameCommand::id),
            makeMenuItem(AppendFretFrameCommand::id)
        })
    };

    return items;
}

MenuItemList NoteInputBarModel::makeTextItems()
{
    MenuItemList items {
        makeMenuItem(AddTitleTextCommand::id),
        makeMenuItem(AddSubtitleTextCommand::id),
        makeMenuItem(AddComposerTextCommand::id),
        makeMenuItem(AddLyricistTextCommand::id),
        makeMenuItem(AddPartTextCommand::id),
        makeSeparator(),
        makeMenuItem(AddSystemTextCommand::id),
        makeMenuItem(AddStaffTextCommand::id),
        makeMenuItem(ADD_DYNAMIC_COMMAND),
        makeMenuItem(AddExpressionTextCommand::id),
        makeMenuItem(AddRehearsalMarkTextCommand::id),
        makeMenuItem(AddInstrumentChangeTextCommand::id),
        makeMenuItem(AddFingeringTextCommand::id),
        makeSeparator(),
        makeMenuItem(AddStickingTextCommand::id),
        makeMenuItem(AddChordTextCommand::id),
        makeMenuItem(AddRomanNumeralTextCommand::id),
        makeMenuItem(AddNashvilleNumberTextCommand::id),
        makeMenuItem(AddLyricsCommand::id),
        makeMenuItem(ADD_FIGURED_BASS_COMMAND),
        makeMenuItem(AddTempoCommand::id)
    };

    return items;
}

MenuItemList NoteInputBarModel::makeLinesItems()
{
    MenuItemList items {
        makeMenuItem(AddSlurCommand::id),
        makeMenuItem(ADD_HAIRPIN_COMMAND),
        makeMenuItem(ADD_HAIRPIN_REVERSE_COMMAND),
        makeMenuItem(ADD_OTTAVA_8VA_COMMAND),
        makeMenuItem(ADD_OTTAVA_8VB_COMMAND),
        makeMenuItem(ADD_NOTELINE_COMMAND)
    };

    return items;
}

MenuItemList NoteInputBarModel::makeChordAndFretboardDiagramsItems()
{
    MenuItemList items {
        makeMenuItem(AddChordTextCommand::id),
        makeMenuItem(ADD_FRETBOARD_DIAGRAM_COMMAND),
        makeSeparator(),
        makeMenuItem(InsertFretFrameCommand::id, TranslatableString("notation", "Fretboard diagram legend"))
    };

    return items;
}
