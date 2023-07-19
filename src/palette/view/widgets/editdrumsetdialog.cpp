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

#include "editdrumsetdialog.h"

#include "io/file.h"

#include "engraving/infrastructure/smufl.h"

#include "engraving/types/constants.h"
#include "engraving/types/symnames.h"
#include "engraving/types/typesconv.h"

#include "engraving/rw/xmlreader.h"
#include "engraving/rw/xmlwriter.h"

#include "libmscore/chord.h"
#include "libmscore/factory.h"
#include "libmscore/masterscore.h"
#include "libmscore/note.h"
#include "libmscore/score.h"
#include "libmscore/stem.h"
#include "libmscore/utils.h"

#include "draw/types/geometry.h"

#include <QMessageBox>

using namespace mu::palette;
using namespace mu::io;
using namespace mu::framework;
using namespace mu::notation;
using namespace mu::engraving;

static const QString EDIT_DRUMSET_DIALOG_NAME("EditDrumsetDialog");
static const std::string_view POSSIBLE_SHORTCUTS("ABCDEFG");

enum Column : char {
    PITCH, NOTE, SHORTCUT, NAME
};

class EditDrumsetTreeWidgetItem : public QTreeWidgetItem
{
public:
    EditDrumsetTreeWidgetItem(QTreeWidget* parent)
        : QTreeWidgetItem(parent) {}

    bool operator<(const QTreeWidgetItem& other) const override
    {
        if (treeWidget()->sortColumn() == Column::PITCH) {
            return data(Column::PITCH, Qt::UserRole).toInt() < other.data(Column::PITCH, Qt::UserRole).toInt();
        }

        return QTreeWidgetItem::operator<(other);
    }
};

//---------------------------------------------------------
//   noteHeadNames
//   "Sol" and "Alt. Brevis" omitted,
//   as not being useful for drums
//---------------------------------------------------------

NoteHeadGroup noteHeadNames[] = {
    NoteHeadGroup::HEAD_NORMAL,
    NoteHeadGroup::HEAD_CROSS,
    NoteHeadGroup::HEAD_PLUS,
    NoteHeadGroup::HEAD_XCIRCLE,
    NoteHeadGroup::HEAD_WITHX,
    NoteHeadGroup::HEAD_TRIANGLE_UP,
    NoteHeadGroup::HEAD_TRIANGLE_DOWN,
    NoteHeadGroup::HEAD_SLASH,
    NoteHeadGroup::HEAD_SLASHED1,
    NoteHeadGroup::HEAD_SLASHED2,
    NoteHeadGroup::HEAD_DIAMOND,
    NoteHeadGroup::HEAD_DIAMOND_OLD,
    NoteHeadGroup::HEAD_CIRCLED,
    NoteHeadGroup::HEAD_CIRCLED_LARGE,
    NoteHeadGroup::HEAD_LARGE_ARROW,
    NoteHeadGroup::HEAD_DO,
    NoteHeadGroup::HEAD_RE,
    NoteHeadGroup::HEAD_MI,
    NoteHeadGroup::HEAD_FA,
    NoteHeadGroup::HEAD_LA,
    NoteHeadGroup::HEAD_TI,
    NoteHeadGroup::HEAD_SWISS_RUDIMENTS_FLAM,
    NoteHeadGroup::HEAD_SWISS_RUDIMENTS_DOUBLE,
    NoteHeadGroup::HEAD_CUSTOM
};

//---------------------------------------------------------
//   EditDrumsetDialog
//---------------------------------------------------------

struct SymbolIcon {
    SymId id;
    QIcon icon;
    SymbolIcon(SymId i, QIcon j)
        : id(i), icon(j)
    {}

    static SymbolIcon generateIcon(const SymId& id, double w, double h, double defaultScale)
    {
        QIcon icon;
        QPixmap image(w, h);
        image.fill(Qt::transparent);
        mu::draw::Painter painter(&image, "generateicon");
        const mu::RectF& bbox = EditDrumsetDialog::engravingFonts()->fallbackFont()->bbox(id, 1);
        const qreal actualSymbolScale = std::min(w / bbox.width(), h / bbox.height());
        qreal mag = std::min(defaultScale, actualSymbolScale);
        const qreal& xStShift = (w - mag * bbox.width()) / 2 - mag * bbox.left();
        const qreal& yStShift = (h - mag * bbox.height()) / 2 - mag * bbox.top();
        const mu::PointF& stPtPos = mu::PointF(xStShift, yStShift);
        EditDrumsetDialog::engravingFonts()->fallbackFont()->draw(id, &painter, mag, stPtPos);
        icon.addPixmap(image);
        return SymbolIcon(id, icon);
    }
};

EditDrumsetDialog::EditDrumsetDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName(EDIT_DRUMSET_DIALOG_NAME);

    m_notation = globalContext()->currentNotation();
    if (!m_notation) {
        return;
    }

    const INotationInteractionPtr interaction = m_notation->interaction();
    INotationInteraction::HitElementContext context = interaction->hitElementContext();

    if (context.element && context.staff) {
        mu::engraving::Instrument* instrument = context.staff->part()->instrument(context.element->tick());
        m_instrumentKey.instrumentId = instrument->id();
        m_instrumentKey.partId = context.staff->part()->id();
        m_instrumentKey.tick = context.element->tick();
        m_originDrumset = *instrument->drumset();
    } else {
        NoteInputState state = m_notation->interaction()->noteInput()->state();
        const Staff* staff = m_notation->elements()->msScore()->staff(track2staff(state.currentTrack));
        m_instrumentKey.instrumentId = staff ? staff->part()->instrumentId().toQString() : QString();
        m_instrumentKey.partId = staff ? staff->part()->id() : ID();
        m_originDrumset = state.drumset ? *state.drumset : Drumset();
    }

    m_editedDrumset = m_originDrumset;

    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    drumNote->setGridSize(70, 80);
    drumNote->setDrawGrid(false);
    drumNote->setReadOnly(true);

    loadPitchesList();

    for (auto g : noteHeadNames) {
        noteHead->addItem(TConv::translatedUserName(g), int(g));
    }

    connect(pitchList, &QTreeWidget::currentItemChanged, this, &EditDrumsetDialog::itemChanged);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &EditDrumsetDialog::bboxClicked);
    connect(name, &QLineEdit::textChanged, this, &EditDrumsetDialog::nameChanged);
    connect(noteHead, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditDrumsetDialog::valueChanged);
    connect(staffLine, QOverload<int>::of(&QSpinBox::valueChanged), this, &EditDrumsetDialog::valueChanged);
    connect(voice, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditDrumsetDialog::valueChanged);
    connect(stemDirection, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditDrumsetDialog::valueChanged);
    connect(shortcut, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditDrumsetDialog::shortcutChanged);
    connect(loadButton, &QPushButton::clicked, this, &EditDrumsetDialog::load);
    connect(saveButton, &QPushButton::clicked, this, &EditDrumsetDialog::save);
    pitchList->setColumnWidth(0, 40);
    pitchList->setColumnWidth(1, 60);
    pitchList->setColumnWidth(2, 30);

    QStringList validNoteheadRanges
        = { "Noteheads", "Round and square noteheads", "Slash noteheads", "Shape note noteheads", "Shape note noteheads supplement",
            "Techniques noteheads" };
    QSet<QString> excludeSym = { "noteheadParenthesisLeft", "noteheadParenthesisRight", "noteheadParenthesis", "noteheadNull" };
    QStringList primaryNoteheads = {
        "noteheadXOrnate",
        "noteheadXBlack",
        "noteheadXHalf",
        "noteheadXWhole",
        "noteheadXDoubleWhole",
        "noteheadSlashedBlack1",
        "noteheadSlashedHalf1",
        "noteheadSlashedWhole1",
        "noteheadSlashedDoubleWhole1",
        "noteheadSlashedBlack2",
        "noteheadSlashedHalf2",
        "noteheadSlashedWhole2",
        "noteheadSlashedDoubleWhole2",
        "noteheadSquareBlack",
        "noteheadMoonBlack",
        "noteheadTriangleUpRightBlack",
        "noteheadTriangleDownBlack",
        "noteheadTriangleUpBlack",
        "noteheadTriangleLeftBlack",
        "noteheadTriangleRoundDownBlack",
        "noteheadDiamondBlack",
        "noteheadDiamondHalf",
        "noteheadDiamondWhole",
        "noteheadDiamondDoubleWhole",
        "noteheadRoundWhiteWithDot",
        "noteheadVoidWithX",
        "noteheadHalfWithX",
        "noteheadWholeWithX",
        "noteheadDoubleWholeWithX",
        "noteheadLargeArrowUpBlack",
        "noteheadLargeArrowUpHalf",
        "noteheadLargeArrowUpWhole",
        "noteheadLargeArrowUpDoubleWhole"
    };

    int w = quarterCmb->iconSize().width() * qApp->devicePixelRatio();
    int h = quarterCmb->iconSize().height() * qApp->devicePixelRatio();
    //default scale is 0.3, will use smaller scale for large noteheads symbols
    const qreal defaultScale = 0.3 * qApp->devicePixelRatio();

    QList<SymbolIcon> resNoteheads;
    for (auto symName : primaryNoteheads) {
        SymId id = SymNames::symIdByName(symName);
        resNoteheads.append(SymbolIcon::generateIcon(id, w, h, defaultScale));
    }

    for (QString range : validNoteheadRanges) {
        for (auto symName : Smufl::smuflRanges().at(range)) {
            SymId id = SymNames::symIdByName(symName);
            if (!excludeSym.contains(symName) && !primaryNoteheads.contains(symName)) {
                resNoteheads.append(SymbolIcon::generateIcon(id, w, h, defaultScale));
            }
        }
    }

    QComboBox* combos[] = { wholeCmb, halfCmb, quarterCmb, doubleWholeCmb };
    for (QComboBox* combo : combos) {
        for (auto si : resNoteheads) {
            SymId id = si.id;
            QIcon icon = si.icon;
            combo->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            combo->addItem(icon, SymNames::translatedUserNameForSymId(id), SymNames::nameForSymId(id).ascii());
        }
    }
    wholeCmb->setCurrentIndex(quarterCmb->findData(SymNames::nameForSymId(SymId::noteheadWhole).ascii()));
    halfCmb->setCurrentIndex(quarterCmb->findData(SymNames::nameForSymId(SymId::noteheadHalf).ascii()));
    quarterCmb->setCurrentIndex(quarterCmb->findData(SymNames::nameForSymId(SymId::noteheadBlack).ascii()));
    doubleWholeCmb->setCurrentIndex(quarterCmb->findData(SymNames::nameForSymId(SymId::noteheadDoubleWhole).ascii()));

    connect(customGbox, &QGroupBox::toggled, this, &EditDrumsetDialog::customGboxToggled);
    connect(quarterCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditDrumsetDialog::customQuarterChanged);

    Q_ASSERT(pitchList->topLevelItemCount() > 0);
    pitchList->setCurrentItem(pitchList->topLevelItem(0));

    quarterCmb->setAccessibleName(quarterLbl->text() + " " + quarterCmb->currentText());
    halfCmb->setAccessibleName(halfLbl->text() + " " + halfCmb->currentText());
    wholeCmb->setAccessibleName(wholeLbl->text() + " " + wholeCmb->currentText());
    doubleWholeCmb->setAccessibleName(doubleWholeLbl->text() + " " + doubleWholeCmb->currentText());

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

EditDrumsetDialog::EditDrumsetDialog(const EditDrumsetDialog& other)
    : QDialog(other.parentWidget())
{
}

int EditDrumsetDialog::static_metaTypeId()
{
    return QMetaType::type(EDIT_DRUMSET_DIALOG_NAME.toStdString().c_str());
}

//---------------------------------------------------------
//   customGboxToggled
//---------------------------------------------------------

void EditDrumsetDialog::customGboxToggled(bool checked)
{
    noteHead->setEnabled(!checked);
    if (checked) {
        noteHead->setCurrentIndex(noteHead->findData(int(NoteHeadGroup::HEAD_CUSTOM)));
    } else {
        noteHead->setCurrentIndex(noteHead->findData(int(NoteHeadGroup::HEAD_NORMAL)));
    }
}

void EditDrumsetDialog::loadPitchesList()
{
    pitchList->blockSignals(true);
    pitchList->clear();
    pitchList->blockSignals(false);

    for (int i = 0; i < 128; ++i) {
        QTreeWidgetItem* item = new EditDrumsetTreeWidgetItem(pitchList);
        item->setText(Column::PITCH, QString("%1").arg(i));
        item->setText(Column::NOTE, pitch2string(i));
        if (m_editedDrumset.shortcut(i) == 0) {
            item->setText(Column::SHORTCUT, "");
        } else {
            QString s(QChar(m_editedDrumset.shortcut(i)));
            item->setText(Column::SHORTCUT, s);
        }
        item->setText(Column::NAME, m_editedDrumset.translatedName(i));
        item->setData(Column::PITCH, Qt::UserRole, i);
    }
    pitchList->sortItems(3, Qt::SortOrder::DescendingOrder);
}

void EditDrumsetDialog::setEnabledPitchControls(bool enable)
{
    customGbox->setEnabled(enable);
    noteHead->setEnabled(enable);
    voice->setEnabled(enable);
    shortcut->setEnabled(enable);
    staffLine->setEnabled(enable);
    stemDirection->setEnabled(enable);
    drumNote->setEnabled(enable);
    label_2->setEnabled(enable);
    label_3->setEnabled(enable);
    label_4->setEnabled(enable);
    label_5->setEnabled(enable);
    label_6->setEnabled(enable);
}

//---------------------------------------------------------
//   nameChanged
//---------------------------------------------------------

void EditDrumsetDialog::nameChanged(const QString& n)
{
    QTreeWidgetItem* item = pitchList->currentItem();
    if (item) {
        item->setText(Column::NAME, n);
        int pitch = item->data(Column::PITCH, Qt::UserRole).toInt();
        if (!n.isEmpty()) {
            if (!m_editedDrumset.isValid(pitch)) {
                noteHead->setCurrentIndex(0);
            }
        } else {
            m_editedDrumset.drum(pitch).name.clear();
        }
    }
    setEnabledPitchControls(!n.isEmpty());
}

//---------------------------------------------------------
//   shortcutChanged
//---------------------------------------------------------

void EditDrumsetDialog::shortcutChanged()
{
    QTreeWidgetItem* item = pitchList->currentItem();
    if (!item) {
        return;
    }

    int pitch = item->data(Column::PITCH, Qt::UserRole).toInt();
    int index = shortcut->currentIndex();
    bool invalidIndex = index < 0 || index >= static_cast<int>(POSSIBLE_SHORTCUTS.size());
    int sc;

    if (invalidIndex) {
        sc = 0;
    } else {
        sc = POSSIBLE_SHORTCUTS[index];
    }

    if (QString(QChar(m_editedDrumset.drum(pitch).shortcut)) != shortcut->currentText()) {
        //
        // remove conflicting shortcuts
        //
        for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
            if (i == pitch) {
                continue;
            }
            if (m_editedDrumset.drum(i).shortcut == sc) {
                m_editedDrumset.drum(i).shortcut = 0;
            }
        }
        m_editedDrumset.drum(pitch).shortcut = sc;
        if (invalidIndex) {
            item->setText(Column::SHORTCUT, "");
        } else {
            item->setText(Column::SHORTCUT, shortcut->currentText());
        }
    }
}

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------
void EditDrumsetDialog::bboxClicked(QAbstractButton* button)
{
    switch (buttonBox->buttonRole(button)) {
    case QDialogButtonBox::ApplyRole:
    case QDialogButtonBox::AcceptRole:
        apply();
        break;
    case QDialogButtonBox::RejectRole:
        cancel();
        break;
    default:
        break;
    }
}

void EditDrumsetDialog::apply()
{
    valueChanged();    //save last changes in name
}

void EditDrumsetDialog::cancel()
{
    m_notation->parts()->replaceDrumset(m_instrumentKey, m_originDrumset);
}

//---------------------------------------------------------
//   fillCustomNoteheadsDataFromComboboxes
//---------------------------------------------------------
void EditDrumsetDialog::fillCustomNoteheadsDataFromComboboxes(int pitch)
{
    m_editedDrumset.drum(pitch).notehead = NoteHeadGroup::HEAD_CUSTOM;
    m_editedDrumset.drum(pitch).noteheads[int(NoteHeadType::HEAD_WHOLE)] = SymNames::symIdByName(wholeCmb->currentData().toString());
    m_editedDrumset.drum(pitch).noteheads[int(NoteHeadType::HEAD_QUARTER)] = SymNames::symIdByName(quarterCmb->currentData().toString());
    m_editedDrumset.drum(pitch).noteheads[int(NoteHeadType::HEAD_HALF)] = SymNames::symIdByName(halfCmb->currentData().toString());
    m_editedDrumset.drum(pitch).noteheads[int(NoteHeadType::HEAD_BREVIS)]
        = SymNames::symIdByName(doubleWholeCmb->currentData().toString());
}

void EditDrumsetDialog::fillNoteheadsComboboxes(bool customGroup, int pitch)
{
    if (customGroup) {
        wholeCmb->setCurrentIndex(quarterCmb->findData(
                                      SymNames::nameForSymId(m_editedDrumset.noteHeads(pitch, NoteHeadType::HEAD_WHOLE)).ascii())
                                  );
        halfCmb->setCurrentIndex(quarterCmb->findData(
                                     SymNames::nameForSymId(m_editedDrumset.noteHeads(pitch, NoteHeadType::HEAD_HALF)).ascii())
                                 );
        quarterCmb->setCurrentIndex(quarterCmb->findData(
                                        SymNames::nameForSymId(m_editedDrumset.noteHeads(pitch, NoteHeadType::HEAD_QUARTER)).ascii())
                                    );
        doubleWholeCmb->setCurrentIndex(quarterCmb->findData(
                                            SymNames::nameForSymId(m_editedDrumset.noteHeads(pitch, NoteHeadType::HEAD_BREVIS)).ascii())
                                        );
    } else {
        const auto group = m_editedDrumset.drum(pitch).notehead;
        if (group == NoteHeadGroup::HEAD_INVALID) {
            return;
        }

        wholeCmb->setCurrentIndex(quarterCmb->findData(SymNames::nameForSymId(Note::noteHead(0, group, NoteHeadType::HEAD_WHOLE)).ascii()));
        halfCmb->setCurrentIndex(quarterCmb->findData(SymNames::nameForSymId(Note::noteHead(0, group, NoteHeadType::HEAD_HALF)).ascii()));
        quarterCmb->setCurrentIndex(quarterCmb->findData(SymNames::nameForSymId(Note::noteHead(0, group,
                                                                                               NoteHeadType::HEAD_QUARTER)).ascii()));
        doubleWholeCmb->setCurrentIndex(quarterCmb->findData(SymNames::nameForSymId(Note::noteHead(0, group,
                                                                                                   NoteHeadType::HEAD_BREVIS)).ascii()));
    }
}

//---------------------------------------------------------
//   itemChanged
//---------------------------------------------------------
void EditDrumsetDialog::itemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (previous) {
        int pitch = previous->data(0, Qt::UserRole).toInt();
        m_editedDrumset.drum(pitch).name          = name->text();
        if (customGbox->isChecked()) {
            fillCustomNoteheadsDataFromComboboxes(pitch);
        } else {
            const QVariant currData = noteHead->currentData();
            if (currData.isValid()) {
                m_editedDrumset.drum(pitch).notehead = NoteHeadGroup(currData.toInt());
            }
        }

        m_editedDrumset.drum(pitch).line          = staffLine->value();
        m_editedDrumset.drum(pitch).voice         = voice->currentIndex();
        int index = shortcut->currentIndex();

        if (index < 0 || index >= static_cast<int>(POSSIBLE_SHORTCUTS.size())) {
            m_editedDrumset.drum(pitch).shortcut = 0;
        } else {
            m_editedDrumset.drum(pitch).shortcut = POSSIBLE_SHORTCUTS[index];
        }
        m_editedDrumset.drum(pitch).stemDirection = DirectionV(stemDirection->currentIndex());
        previous->setText(Column::NAME, m_editedDrumset.translatedName(pitch));
    }
    if (current == 0) {
        return;
    }

    staffLine->blockSignals(true);
    voice->blockSignals(true);
    stemDirection->blockSignals(true);
    noteHead->blockSignals(true);

    int pitch = current->data(0, Qt::UserRole).toInt();
    name->setText(m_editedDrumset.translatedName(pitch));
    staffLine->setValue(m_editedDrumset.line(pitch));
    voice->setCurrentIndex(m_editedDrumset.voice(pitch));
    stemDirection->setCurrentIndex(int(m_editedDrumset.stemDirection(pitch)));

    NoteHeadGroup nh = m_editedDrumset.noteHead(pitch);
    bool isCustomGroup = (nh == NoteHeadGroup::HEAD_CUSTOM);
    if (m_editedDrumset.isValid(pitch)) {
        setCustomNoteheadsGUIEnabled(isCustomGroup);
    }
    noteHead->setCurrentIndex(noteHead->findData(int(nh)));
    fillNoteheadsComboboxes(isCustomGroup, pitch);

    if (m_editedDrumset.shortcut(pitch) == 0) {
        shortcut->setCurrentIndex(7);
    } else {
        shortcut->setCurrentIndex(m_editedDrumset.shortcut(pitch) - 'A');
    }

    staffLine->blockSignals(false);
    voice->blockSignals(false);
    stemDirection->blockSignals(false);
    noteHead->blockSignals(false);

    updateExample();
}

//---------------------------------------------------------
//   setCustomNoteheadsGUIEnabled
//---------------------------------------------------------
void EditDrumsetDialog::setCustomNoteheadsGUIEnabled(bool enabled)
{
    customGbox->setChecked(enabled);
    noteHead->setEnabled(!enabled);
    if (enabled) {
        noteHead->setCurrentIndex(noteHead->findData(int(NoteHeadGroup::HEAD_CUSTOM)));
    }
}

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------
void EditDrumsetDialog::valueChanged()
{
    if (!pitchList->currentItem()) {
        return;
    }
    int pitch = pitchList->currentItem()->data(Column::PITCH, Qt::UserRole).toInt();
    m_editedDrumset.drum(pitch).name          = name->text();
    if (customGbox->isChecked() || noteHead->currentIndex() == noteHead->findData(int(NoteHeadGroup::HEAD_CUSTOM))) {
        fillCustomNoteheadsDataFromComboboxes(pitch);
        setCustomNoteheadsGUIEnabled(true);
    } else {
        m_editedDrumset.drum(pitch).notehead = NoteHeadGroup(noteHead->currentData().toInt());
        fillNoteheadsComboboxes(false, pitch);
        setCustomNoteheadsGUIEnabled(false);
    }

    m_editedDrumset.drum(pitch).line          = staffLine->value();
    m_editedDrumset.drum(pitch).voice         = voice->currentIndex();
    m_editedDrumset.drum(pitch).stemDirection = DirectionV(stemDirection->currentIndex());
    if (QString(QChar(m_editedDrumset.drum(pitch).shortcut)) != shortcut->currentText()) {
        if (shortcut->currentText().isEmpty()) {
            m_editedDrumset.drum(pitch).shortcut = 0;
        } else {
            m_editedDrumset.drum(pitch).shortcut = shortcut->currentText().at(0).toLatin1();
        }
    }
    updateExample();

    m_notation->parts()->replaceDrumset(m_instrumentKey, m_editedDrumset);
}

//---------------------------------------------------------
//   updateExample
//---------------------------------------------------------
void EditDrumsetDialog::updateExample()
{
    drumNote->clear();
    int pitch = pitchList->currentItem()->data(0, Qt::UserRole).toInt();
    if (!m_editedDrumset.isValid(pitch)) {
        return;
    }
    int line      = m_editedDrumset.line(pitch);
    NoteHeadGroup nh = m_editedDrumset.noteHead(pitch);
    int v         = m_editedDrumset.voice(pitch);
    DirectionV dir = m_editedDrumset.stemDirection(pitch);
    bool up = (DirectionV::UP == dir) || (DirectionV::AUTO == dir && line > 4);
    std::shared_ptr<Chord> chord = Factory::makeChord(gpaletteScore->dummy()->segment());
    chord->setDurationType(DurationType::V_QUARTER);
    chord->setStemDirection(dir);
    chord->setTrack(v);
    chord->setIsUiItem(true);
    Note* note = Factory::createNote(chord.get());
    note->setParent(chord.get());
    note->setTrack(v);
    note->setPitch(pitch);
    note->setTpcFromPitch();
    note->setLine(line);
    note->setPos(0.0, gpaletteScore->style().spatium() * .5 * line);
    note->setHeadType(NoteHeadType::HEAD_QUARTER);
    note->setHeadGroup(nh);
    note->setCachedNoteheadSym(SymNames::symIdByName(quarterCmb->currentData().toString()));
    chord->add(note);
    Stem* stem = Factory::createStem(chord.get());
    stem->setBaseLength(Millimetre((up ? -3.0 : 3.0) * gpaletteScore->style().spatium()));
    chord->add(stem);
    drumNote->appendElement(chord, m_editedDrumset.translatedName(pitch));
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void EditDrumsetDialog::load()
{
    std::vector<std::string> filter = { mu::trc("palette", "MuseScore drumset file") + " (*.drm)" };
    mu::io::path_t dir = notationConfiguration()->userStylesPath();
    mu::io::path_t fname = interactive()->selectOpeningFile(mu::qtrc("palette", "Load drumset"), dir, filter);

    if (fname.empty()) {
        return;
    }

    File fp(fname.toQString());
    if (!fp.open(IODevice::ReadOnly)) {
        return;
    }

    XmlReader e(&fp);
    m_editedDrumset.clear();
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            if (e.attribute("version") != Constants::MSC_VERSION_STR) {
                auto result = interactive()->warning(
                    mu::trc("palette", "Drumset file too old"),
                    mu::trc("palette", "MuseScore may not be able to load this drumset file."), {
                    IInteractive::Button::Cancel,
                    IInteractive::Button::Ignore
                }, IInteractive::Button::Cancel);

                if (result.standardButton() != IInteractive::Button::Ignore) { // covers Cancel and Esc
                    return;
                }
            }
            while (e.readNextStartElement()) {
                if (e.name() == "Drum") {
                    m_editedDrumset.load(e);
                } else {
                    e.unknown();
                }
            }
        }
    }
    fp.close();
    loadPitchesList();
    pitchList->setCurrentItem(pitchList->topLevelItem(0));
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void EditDrumsetDialog::save()
{
    std::vector<std::string> filter = { mu::trc("palette", "MuseScore drumset file") + " (*.drm)" };
    mu::io::path_t dir = notationConfiguration()->userStylesPath();
    mu::io::path_t fname = interactive()->selectSavingFile(mu::qtrc("palette", "Save drumset"), dir, filter);

    if (fname.empty()) {
        return;
    }

    File f(fname);
    if (!f.open(IODevice::WriteOnly)) {
        QString s = mu::qtrc("palette", "Opening file\n%1\nfailed: %2").arg(f.filePath().toQString()).arg(strerror(errno));
        interactive()->error(mu::trc("palette", "Open file"), s.toStdString());
        return;
    }
    valueChanged();    //save last changes in name
    XmlWriter xml(&f);
    xml.startDocument();
    xml.startElement("museScore", { { "version", Constants::MSC_VERSION_STR } });
    m_editedDrumset.save(xml);
    xml.endElement();
    if (f.hasError()) {
        QString s = mu::qtrc("palette", "Writing file failed: %1").arg(QString::fromStdString(f.errorString()));
        interactive()->error(mu::trc("palette", "Write drumset"), s.toStdString());
    }
}

//---------------------------------------------------------
//   customQuarterChanged
//---------------------------------------------------------
void EditDrumsetDialog::customQuarterChanged(int)
{
    updateExample();
}
