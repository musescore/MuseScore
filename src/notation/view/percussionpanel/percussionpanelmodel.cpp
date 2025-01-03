/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "percussionpanelmodel.h"
#include "types/translatablestring.h"
#include "ui/view/iconcodes.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/undo.h"
#include "engraving/dom/utils.h"

#include "audio/audioutils.h"

static const QString PAD_NAMES_CODE("percussion-pad-names");
static const QString NOTATION_PREVIEW_CODE("percussion-notation-preview");
static const QString EDIT_LAYOUT_CODE("percussion-edit-layout");
static const QString RESET_LAYOUT_CODE("percussion-reset-layout");

using namespace muse;
using namespace ui;
using namespace mu::notation;

PercussionPanelModel::PercussionPanelModel(QObject* parent)
    : QObject(parent)
{
    m_padListModel = new PercussionPanelPadListModel(this);
    qApp->installEventFilter(this);
}

bool PercussionPanelModel::enabled() const
{
    return m_enabled;
}

void PercussionPanelModel::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }
    m_enabled = enabled;
    emit enabledChanged();
}

QString PercussionPanelModel::soundTitle() const
{
    return m_soundTitle;
}

void PercussionPanelModel::setSoundTitle(const QString& soundTitle)
{
    if (m_soundTitle == soundTitle) {
        return;
    }
    m_soundTitle = soundTitle;
    emit soundTitleChanged();
}

PanelMode::Mode PercussionPanelModel::currentPanelMode() const
{
    return m_currentPanelMode;
}

void PercussionPanelModel::setCurrentPanelMode(const PanelMode::Mode& panelMode)
{
    if (m_currentPanelMode == panelMode) {
        return;
    }

    // After editing, return to the last non-edit mode...
    if (panelMode != PanelMode::Mode::EDIT_LAYOUT && m_panelModeToRestore != panelMode) {
        m_panelModeToRestore = panelMode;
    }

    m_currentPanelMode = panelMode;
    emit currentPanelModeChanged(m_currentPanelMode);
}

bool PercussionPanelModel::useNotationPreview() const
{
    return m_useNotationPreview;
}

void PercussionPanelModel::setUseNotationPreview(bool useNotationPreview)
{
    if (m_useNotationPreview == useNotationPreview) {
        return;
    }

    m_useNotationPreview = useNotationPreview;
    emit useNotationPreviewChanged(m_useNotationPreview);
}

PercussionPanelPadListModel* PercussionPanelModel::padListModel() const
{
    return m_padListModel;
}

void PercussionPanelModel::init()
{
    setUpConnections();

    globalContext()->currentNotationChanged().onNotify(this, [this] {
        setUpConnections();
    });
}

QList<QVariantMap> PercussionPanelModel::layoutMenuItems() const
{
    const TranslatableString padNamesTitle("notation/percussion", "Pad names");
    // Using IconCode for this instead of "checked" because we want the tick to display on the left
    const int padNamesIcon = static_cast<int>(m_useNotationPreview ? IconCode::Code::NONE : IconCode::Code::TICK_RIGHT_ANGLE);

    const TranslatableString notationPreviewTitle("notation/percussion", "Notation preview");
    // Using IconCode for this instead of "checked" because we want the tick to display on the left
    const int notationPreviewIcon = static_cast<int>(m_useNotationPreview ? IconCode::Code::TICK_RIGHT_ANGLE : IconCode::Code::NONE);

    const TranslatableString editLayoutTitle = m_currentPanelMode == PanelMode::Mode::EDIT_LAYOUT
                                               ? TranslatableString("notation/percussion", "Finish editing")
                                               : TranslatableString("notation/percussion", "Edit layout");
    const int editLayoutIcon = static_cast<int>(IconCode::Code::CONFIGURE);

    const TranslatableString resetLayoutTitle("notation/percussion", "Reset layout");
    const int resetLayoutIcon = static_cast<int>(IconCode::Code::UNDO);

    QList<QVariantMap> menuItems = {
        { { "id", PAD_NAMES_CODE },
            { "title", padNamesTitle.qTranslated() }, { "icon", padNamesIcon }, { "enabled", true } },

        { { "id", NOTATION_PREVIEW_CODE },
            { "title", notationPreviewTitle.qTranslated() }, { "icon", notationPreviewIcon }, { "enabled", true } },

        { }, // separator

        { { "id", EDIT_LAYOUT_CODE },
            { "title", editLayoutTitle.qTranslated() }, { "icon", editLayoutIcon }, { "enabled", true } },

        { { "id", RESET_LAYOUT_CODE },
            { "title", resetLayoutTitle.qTranslated() }, { "icon", resetLayoutIcon }, { "enabled", true } },
    };

    return menuItems;
}

void PercussionPanelModel::handleMenuItem(const QString& itemId)
{
    if (itemId == PAD_NAMES_CODE) {
        setUseNotationPreview(false);
    } else if (itemId == NOTATION_PREVIEW_CODE) {
        setUseNotationPreview(true);
    } else if (itemId == EDIT_LAYOUT_CODE) {
        const bool currentlyEditing = m_currentPanelMode == PanelMode::Mode::EDIT_LAYOUT;
        currentlyEditing ? finishEditing() : setCurrentPanelMode(PanelMode::Mode::EDIT_LAYOUT);
    } else if (itemId == RESET_LAYOUT_CODE) {
        resetLayout();
    }
}

void PercussionPanelModel::finishEditing(bool discardChanges)
{
    if (!interaction()) {
        //! NOTE: Can happen if we close the project while editing the layout...
        m_padListModel->setDrumset(nullptr);
        setCurrentPanelMode(m_panelModeToRestore);
        return;
    }

    m_padListModel->removeEmptyRows();

    NoteInputState inputState = interaction()->noteInput()->state();
    const Staff* staff = inputState.staff;

    IF_ASSERT_FAILED(staff && staff->part()) {
        return;
    }

    Instrument* inst = staff->part()->instrument(inputState.segment->tick());

    IF_ASSERT_FAILED(inst && inst->drumset()) {
        return;
    }

    if (discardChanges) {
        m_padListModel->setDrumset(inst->drumset());
        setCurrentPanelMode(m_panelModeToRestore);
        return;
    }

    Drumset updatedDrumset = *m_padListModel->drumset();

    for (int i = 0; i < m_padListModel->padList().size(); ++i) {
        const PercussionPanelPadModel* model = m_padListModel->padList().at(i);
        if (!model) {
            continue;
        }

        const int row = i / m_padListModel->numColumns();
        const int column = i % m_padListModel->numColumns();

        engraving::DrumInstrument& drum = updatedDrumset.drum(model->pitch());

        drum.panelRow = row;
        drum.panelColumn = column;

        const QString& shortcut = model->keyboardShortcut();
        drum.shortcut = shortcut.isEmpty() ? '\0' : shortcut.toLatin1().at(0);
    }

    // Return if nothing changed after edit...
    if (inst->drumset() && *inst->drumset() == updatedDrumset) {
        setCurrentPanelMode(m_panelModeToRestore);
        m_padListModel->focusLastActivePad();
        return;
    }

    INotationUndoStackPtr undoStack = notation()->undoStack();

    undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Edit percussion panel layout"));
    score()->undo(new engraving::ChangeDrumset(inst, &updatedDrumset, staff->part()));
    undoStack->commitChanges();

    setCurrentPanelMode(m_panelModeToRestore);
    m_padListModel->focusLastActivePad();
}

void PercussionPanelModel::customizeKit()
{
    dispatcher()->dispatch("customize-kit");
}

void PercussionPanelModel::setUpConnections()
{
    const auto updatePadModels = [this](Drumset* drumset) {
        if (drumset && m_padListModel->drumset() && *drumset == *m_padListModel->drumset()) {
            return;
        }

        if (m_currentPanelMode == PanelMode::Mode::EDIT_LAYOUT) {
            finishEditing(/*discardChanges*/ true);
        }

        m_padListModel->setDrumset(drumset);
        updateSoundTitle(currentTrackId());
    };

    if (!notation()) {
        updatePadModels(nullptr);
        return;
    }

    const INotationNoteInputPtr noteInput = interaction()->noteInput();
    updatePadModels(noteInput->state().drumset);
    setEnabled(m_padListModel->hasActivePads());

    noteInput->stateChanged().onNotify(this, [this, updatePadModels]() {
        if (!notation()) {
            updatePadModels(nullptr);
            return;
        }
        const INotationNoteInputPtr ni = interaction()->noteInput();
        updatePadModels(ni->state().drumset);
    });

    m_padListModel->hasActivePadsChanged().onNotify(this, [this]() {
        setEnabled(m_padListModel->hasActivePads());
    });

    m_padListModel->padTriggered().onReceive(this, [this](int pitch) {
        switch (currentPanelMode()) {
        case PanelMode::Mode::EDIT_LAYOUT: return;
        case PanelMode::Mode::WRITE: writePitch(pitch); // fall through
        case PanelMode::Mode::SOUND_PREVIEW: playPitch(pitch);
        }
    });

    if (!audioSettings()) {
        return;
    }

    audioSettings()->trackInputParamsChanged().onReceive(this, [this](InstrumentTrackId trackId) {
        if (trackId != currentTrackId()) {
            return;
        }
        updateSoundTitle(trackId);
    });
}

void PercussionPanelModel::updateSoundTitle(const InstrumentTrackId& trackId)
{
    if (!trackId.isValid()) {
        setSoundTitle(QString());
        return;
    }

    const audio::AudioInputParams& params = audioSettings()->trackInputParams(trackId);

    const QString name = muse::audio::audioSourceName(params).toQString();
    const QString category = muse::audio::audioSourceCategoryName(params).toQString();
    if (name.isEmpty() || category.isEmpty()) {
        setSoundTitle(QString());
        return;
    }

    setSoundTitle(category + ": " + name);
}

bool PercussionPanelModel::eventFilter(QObject* watched, QEvent* event)
{
    // Finish editing on escape...
    if (m_currentPanelMode != PanelMode::Mode::EDIT_LAYOUT || event->type() != QEvent::Type::ShortcutOverride
        || m_padListModel->swapInProgress()) {
        return QObject::eventFilter(watched, event);
    }
    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
    if (!keyEvent || keyEvent->key() != Qt::Key_Escape) {
        return QObject::eventFilter(watched, event);
    }
    finishEditing();
    event->setAccepted(true);
    return true;
}

void PercussionPanelModel::writePitch(int pitch)
{
    INotationUndoStackPtr undoStack = notation()->undoStack();
    if (!interaction() || !undoStack) {
        return;
    }

    undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Enter percussion note"));

    interaction()->noteInput()->startNoteInput(/*focusNotation*/ false);

    score()->addMidiPitch(pitch, false, /*transpose*/ false);
    undoStack->commitChanges();

    const mu::engraving::InputState& inputState = score()->inputState();
    if (inputState.cr()) {
        interaction()->showItem(inputState.cr());
    }
}

void PercussionPanelModel::playPitch(int pitch)
{
    const mu::engraving::InputState& inputState = score()->inputState();
    if (!inputState.cr()) {
        return;
    }

    Chord* chord = mu::engraving::Factory::createChord(inputState.lastSegment());
    chord->setParent(inputState.lastSegment());

    Note* note = mu::engraving::Factory::createNote(chord);
    note->setParent(chord);

    note->setStaffIdx(mu::engraving::track2staff(inputState.cr()->track()));

    const mu::engraving::NoteVal nval = score()->noteVal(pitch, /*transpose*/ false);
    note->setNval(nval);

    playbackController()->playElements({ note });

    note->setParent(nullptr);
    delete note;
}

void PercussionPanelModel::resetLayout()
{
    if (m_currentPanelMode == PanelMode::Mode::EDIT_LAYOUT) {
        finishEditing(/*discardChanges*/ true);
    }

    NoteInputState inputState = interaction()->noteInput()->state();
    const Staff* staff = inputState.staff;

    IF_ASSERT_FAILED(staff && staff->part()) {
        return;
    }

    Instrument* inst = staff->part()->instrument(inputState.segment->tick());

    IF_ASSERT_FAILED(inst) {
        return;
    }

    const InstrumentTemplate& instTemplate = instrumentsRepository()->instrumentTemplate(inst->id());
    const Drumset* defaultDrumset = instTemplate.drumset;

    IF_ASSERT_FAILED(defaultDrumset) {
        return;
    }

    Drumset defaultLayout = m_padListModel->constructDefaultLayout(defaultDrumset);
    if (defaultLayout == *m_padListModel->drumset()) {
        // Nothing changed after reset...
        return;
    }

    INotationUndoStackPtr undoStack = notation()->undoStack();

    undoStack->prepareChanges(muse::TranslatableString("undoableAction", "Reset percussion panel layout"));
    score()->undo(new engraving::ChangeDrumset(inst, &defaultLayout, staff->part()));
    undoStack->commitChanges();
}

InstrumentTrackId PercussionPanelModel::currentTrackId() const
{
    if (!interaction()) {
        return InstrumentTrackId();
    }

    const NoteInputState inputState = interaction()->noteInput()->state();
    const Staff* staff = inputState.staff;

    if (!staff || !staff->part() || !inputState.segment) {
        return InstrumentTrackId();
    }

    return { staff->part()->id(), staff->part()->instrumentId(inputState.segment->tick()) };
}

const project::IProjectAudioSettingsPtr PercussionPanelModel::audioSettings() const
{
    return globalContext()->currentProject() ? globalContext()->currentProject()->audioSettings() : nullptr;
}

const INotationPtr PercussionPanelModel::notation() const
{
    return globalContext()->currentNotation();
}

const INotationInteractionPtr PercussionPanelModel::interaction() const
{
    return notation() ? notation()->interaction() : nullptr;
}

mu::engraving::Score* PercussionPanelModel::score() const
{
    return notation() ? notation()->elements()->msScore() : nullptr;
}
