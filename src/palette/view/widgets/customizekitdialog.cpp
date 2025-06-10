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

#include "customizekitdialog.h"

#include "io/file.h"

#include "engraving/infrastructure/smufl.h"

#include "engraving/types/constants.h"
#include "engraving/types/symnames.h"
#include "engraving/types/typesconv.h"

#include "engraving/rw/xmlreader.h"
#include "engraving/rw/xmlwriter.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/score.h"
#include "engraving/dom/utils.h"

#include "notation/utilities/percussionutilities.h"

#include "draw/types/geometry.h"

#include <QMessageBox>

using namespace mu::palette;
using namespace muse::io;
using namespace mu::notation;
using namespace mu::engraving;

enum Column : char {
    PITCH, NOTE, SHORTCUT, NAME
};

class CustomizeKitTreeWidgetItem : public QTreeWidgetItem
{
public:
    CustomizeKitTreeWidgetItem(QTreeWidget* parent)
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
//   CustomizeKitDialog
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
        muse::draw::Painter painter(&image, "generateicon");

        const muse::RectF& bbox = CustomizeKitDialog::engravingFonts()->fallbackFont()->bbox(id, 1);
        const qreal actualSymbolScale = std::min(w / bbox.width(), h / bbox.height());
        qreal mag = std::min(defaultScale, actualSymbolScale);
        const qreal& xStShift = (w - mag * bbox.width()) / 2 - mag * bbox.left();
        const qreal& yStShift = (h - mag * bbox.height()) / 2 - mag * bbox.top();
        const muse::PointF& stPtPos = muse::PointF(xStShift, yStShift);

        painter.setPen(Color(CustomizeKitDialog::uiConfiguration()->currentTheme().values[muse::ui::FONT_PRIMARY_COLOR].toString()));
        painter.setBrush(Color::transparent);

        CustomizeKitDialog::engravingFonts()->fallbackFont()->draw(id, &painter, mag, stPtPos);

        icon.addPixmap(image);

        return SymbolIcon(id, icon);
    }
};

CustomizeKitDialog::CustomizeKitDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("CustomizeKitDialog"));

    m_notation = globalContext()->currentNotation();
    if (!m_notation) {
        return;
    }

    initDrumsetAndKey();

    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    drumNote->setGridSize(70, 80);
    drumNote->setDrawGrid(false);
    drumNote->setReadOnly(true);

    QTreeWidgetItem* itemToSelect = loadPitchesList();

    for (auto g : noteHeadNames) {
        noteHead->addItem(TConv::translatedUserName(g), int(g));
    }

    connect(pitchList, &QTreeWidget::currentItemChanged, this, &CustomizeKitDialog::itemChanged);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &CustomizeKitDialog::bboxClicked);
    connect(name, &QLineEdit::textChanged, this, &CustomizeKitDialog::nameChanged);
    connect(noteHead, &QComboBox::currentIndexChanged, this, &CustomizeKitDialog::valueChanged);
    connect(staffLine, &QSpinBox::valueChanged, this, &CustomizeKitDialog::valueChanged);
    connect(voice, &QComboBox::currentIndexChanged, this, &CustomizeKitDialog::valueChanged);
    connect(stemDirection, &QComboBox::currentIndexChanged, this, &CustomizeKitDialog::valueChanged);
    connect(shortcut, &QPushButton::clicked, this, &CustomizeKitDialog::defineShortcut);
    connect(loadButton, &QPushButton::clicked, this, &CustomizeKitDialog::load);
    connect(saveButton, &QPushButton::clicked, this, &CustomizeKitDialog::save);
    pitchList->setColumnWidth(0, 40);
    pitchList->setColumnWidth(1, 60);
    pitchList->setColumnWidth(2, 30);

    StringList validNoteheadRanges = {
        u"Noteheads",
        u"Round and square noteheads",
        u"Slash noteheads",
        u"Shape note noteheads",
        u"Shape note noteheads supplement",
        u"Techniques noteheads"
    };
    QSet excludedSyms = {
        SymId::noteheadParenthesisLeft,
        SymId::noteheadParenthesisRight,
        SymId::noteheadParenthesis,
        SymId::noteheadNull,
    };
    QList primaryNoteheads = {
        SymId::noteheadXOrnate,
        SymId::noteheadXBlack,
        SymId::noteheadXHalf,
        SymId::noteheadXWhole,
        SymId::noteheadXDoubleWhole,
        SymId::noteheadSlashedBlack1,
        SymId::noteheadSlashedHalf1,
        SymId::noteheadSlashedWhole1,
        SymId::noteheadSlashedDoubleWhole1,
        SymId::noteheadSlashedBlack2,
        SymId::noteheadSlashedHalf2,
        SymId::noteheadSlashedWhole2,
        SymId::noteheadSlashedDoubleWhole2,
        SymId::noteheadSquareBlack,
        SymId::noteheadMoonBlack,
        SymId::noteheadTriangleUpRightBlack,
        SymId::noteheadTriangleDownBlack,
        SymId::noteheadTriangleUpBlack,
        SymId::noteheadTriangleLeftBlack,
        SymId::noteheadTriangleRoundDownBlack,
        SymId::noteheadDiamondBlack,
        SymId::noteheadDiamondHalf,
        SymId::noteheadDiamondWhole,
        SymId::noteheadDiamondDoubleWhole,
        SymId::noteheadRoundWhiteWithDot,
        SymId::noteheadVoidWithX,
        SymId::noteheadHalfWithX,
        SymId::noteheadWholeWithX,
        SymId::noteheadDoubleWholeWithX,
        SymId::noteheadLargeArrowUpBlack,
        SymId::noteheadLargeArrowUpHalf,
        SymId::noteheadLargeArrowUpWhole,
        SymId::noteheadLargeArrowUpDoubleWhole
    };

    int w = quarterCmb->iconSize().width() * qApp->devicePixelRatio();
    int h = quarterCmb->iconSize().height() * qApp->devicePixelRatio();
    //default scale is 0.3, will use smaller scale for large noteheads symbols
    const qreal defaultScale = 0.3 * qApp->devicePixelRatio();

    QList<SymbolIcon> resNoteheads;
    for (SymId symId : primaryNoteheads) {
        resNoteheads.append(SymbolIcon::generateIcon(symId, w, h, defaultScale));
    }

    for (const String& range : validNoteheadRanges) {
        for (SymId symId : Smufl::smuflRanges().at(range)) {
            if (!excludedSyms.contains(symId) && !primaryNoteheads.contains(symId)) {
                resNoteheads.append(SymbolIcon::generateIcon(symId, w, h, defaultScale));
            }
        }
    }

    QComboBox* combos[] = { wholeCmb, halfCmb, quarterCmb, doubleWholeCmb };
    for (QComboBox* combo : combos) {
        for (const SymbolIcon& si : resNoteheads) {
            SymId id = si.id;
            QIcon icon = si.icon;
            combo->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            combo->addItem(icon, SymNames::translatedUserNameForSymId(id), static_cast<int>(id));
        }
    }
    wholeCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(SymId::noteheadWhole)));
    halfCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(SymId::noteheadHalf)));
    quarterCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(SymId::noteheadBlack)));
    doubleWholeCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(SymId::noteheadDoubleWhole)));

    connect(customGbox, &QGroupBox::toggled, this, &CustomizeKitDialog::customGboxToggled);
    connect(quarterCmb, &QComboBox::currentIndexChanged, this, &CustomizeKitDialog::customQuarterChanged);

    Q_ASSERT(pitchList->topLevelItemCount() > 0);
    pitchList->setCurrentItem(itemToSelect ? itemToSelect : pitchList->topLevelItem(0));

    quarterCmb->setAccessibleName(quarterLbl->text() + " " + quarterCmb->currentText());
    halfCmb->setAccessibleName(halfLbl->text() + " " + halfCmb->currentText());
    wholeCmb->setAccessibleName(wholeLbl->text() + " " + wholeCmb->currentText());
    doubleWholeCmb->setAccessibleName(doubleWholeLbl->text() + " " + doubleWholeCmb->currentText());

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

//---------------------------------------------------------
//   customGboxToggled
//---------------------------------------------------------

void CustomizeKitDialog::customGboxToggled(bool checked)
{
    noteHead->setEnabled(!checked);
    if (checked) {
        noteHead->setCurrentIndex(noteHead->findData(int(NoteHeadGroup::HEAD_CUSTOM)));
    } else {
        noteHead->setCurrentIndex(noteHead->findData(int(NoteHeadGroup::HEAD_NORMAL)));
    }
}

QTreeWidgetItem* CustomizeKitDialog::loadPitchesList()
{
    const INotationInteractionPtr interaction = m_notation->interaction();
    const mu::engraving::Note* note = dynamic_cast<mu::engraving::Note*>(interaction->contextItem());
    const int originPitch = note ? note->pitch() : -1;

    pitchList->blockSignals(true);
    pitchList->clear();
    pitchList->blockSignals(false);

    QTreeWidgetItem* itemToSelect = nullptr;

    for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
        QTreeWidgetItem* item = new CustomizeKitTreeWidgetItem(pitchList);
        item->setText(Column::PITCH, QString("%1").arg(i));
        item->setText(Column::NOTE, pitch2string(i));
        item->setText(Column::SHORTCUT, m_editedDrumset.shortcut(i));
        item->setText(Column::NAME, m_editedDrumset.translatedName(i));
        item->setData(Column::PITCH, Qt::UserRole, i);
        if (i == originPitch) {
            itemToSelect = item;
        }
    }

    pitchList->sortItems(3, Qt::SortOrder::DescendingOrder);

    return itemToSelect;
}

void CustomizeKitDialog::setEnabledPitchControls(bool enable)
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

void CustomizeKitDialog::nameChanged(const QString& n)
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
//   bboxClicked
//---------------------------------------------------------
void CustomizeKitDialog::bboxClicked(QAbstractButton* button)
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

void CustomizeKitDialog::initDrumsetAndKey()
{
    const INotationInteractionPtr interaction = m_notation->interaction();
    const INotationInteraction::HitElementContext context = interaction->hitElementContext();
    const NoteInputState& state = interaction->noteInput()->state();

    // Prefer hit context, fall back to note input...
    const bool hitContextValid = context.staff && context.element;
    const bool noteInputValid = state.staff() && state.segment();
    IF_ASSERT_FAILED(hitContextValid || noteInputValid) {
        return;
    }

    mu::engraving::Part* part = hitContextValid ? context.staff->part() : state.staff()->part();
    mu::engraving::Fraction tick = hitContextValid ? context.element->tick() : state.segment()->tick();
    mu::engraving::Instrument* inst = part ? part->instrument(tick) : nullptr;
    IF_ASSERT_FAILED(inst && inst->useDrumset()) {
        return;
    }

    m_instrumentKey = { inst->id(), part->id(), tick };

    m_originDrumset = *inst->drumset();
    m_editedDrumset = m_originDrumset;
}

void CustomizeKitDialog::apply()
{
    valueChanged();    //save last changes in name

    //! NOTE: The note input state changes inside valueChanged,
    //! so to update the state of the drumset panel view, need to notify the change
    notifyAboutNoteInputStateChanged();
}

void CustomizeKitDialog::notifyAboutNoteInputStateChanged()
{
    m_notation->interaction()->noteInput()->stateChanged().notify();
}

void CustomizeKitDialog::cancel()
{
    m_notation->parts()->replaceDrumset(m_instrumentKey, m_originDrumset);
}

//---------------------------------------------------------
//   fillCustomNoteheadsDataFromComboboxes
//---------------------------------------------------------
void CustomizeKitDialog::fillCustomNoteheadsDataFromComboboxes(int pitch)
{
    m_editedDrumset.drum(pitch).notehead = NoteHeadGroup::HEAD_CUSTOM;
    m_editedDrumset.drum(pitch).noteheads[int(NoteHeadType::HEAD_WHOLE)] = static_cast<SymId>(wholeCmb->currentData().toInt());
    m_editedDrumset.drum(pitch).noteheads[int(NoteHeadType::HEAD_QUARTER)] = static_cast<SymId>(quarterCmb->currentData().toInt());
    m_editedDrumset.drum(pitch).noteheads[int(NoteHeadType::HEAD_HALF)] = static_cast<SymId>(halfCmb->currentData().toInt());
    m_editedDrumset.drum(pitch).noteheads[int(NoteHeadType::HEAD_BREVIS)] = static_cast<SymId>(doubleWholeCmb->currentData().toInt());
}

void CustomizeKitDialog::fillNoteheadsComboboxes(bool customGroup, int pitch)
{
    if (customGroup) {
        wholeCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(m_editedDrumset.noteHeads(pitch, NoteHeadType::HEAD_WHOLE))));
        halfCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(m_editedDrumset.noteHeads(pitch, NoteHeadType::HEAD_HALF))));
        quarterCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(m_editedDrumset.noteHeads(pitch, NoteHeadType::HEAD_QUARTER))));
        doubleWholeCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(m_editedDrumset.noteHeads(pitch,
                                                                                                        NoteHeadType::HEAD_BREVIS))));
    } else {
        const auto group = m_editedDrumset.drum(pitch).notehead;
        if (group == NoteHeadGroup::HEAD_INVALID) {
            return;
        }

        wholeCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(Note::noteHead(0, group, NoteHeadType::HEAD_WHOLE))));
        halfCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(Note::noteHead(0, group, NoteHeadType::HEAD_HALF))));
        quarterCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(Note::noteHead(0, group, NoteHeadType::HEAD_QUARTER))));
        doubleWholeCmb->setCurrentIndex(quarterCmb->findData(static_cast<int>(Note::noteHead(0, group, NoteHeadType::HEAD_BREVIS))));
    }
}

//---------------------------------------------------------
//   itemChanged
//---------------------------------------------------------
void CustomizeKitDialog::itemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
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

        m_editedDrumset.drum(pitch).line = staffLine->value();
        m_editedDrumset.drum(pitch).voice = voice->currentIndex();
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

    const QString shortcutText = m_editedDrumset.shortcut(pitch);
    shortcut->setText(!shortcutText.isEmpty() ? shortcutText : muse::qtrc("shortcuts", "None"));

    staffLine->blockSignals(false);
    voice->blockSignals(false);
    stemDirection->blockSignals(false);
    noteHead->blockSignals(false);

    updateExample();
}

//---------------------------------------------------------
//   setCustomNoteheadsGUIEnabled
//---------------------------------------------------------
void CustomizeKitDialog::setCustomNoteheadsGUIEnabled(bool enabled)
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
void CustomizeKitDialog::valueChanged()
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

    m_editedDrumset.drum(pitch).line  = staffLine->value();
    m_editedDrumset.drum(pitch).voice = voice->currentIndex();
    m_editedDrumset.drum(pitch).stemDirection = DirectionV(stemDirection->currentIndex());

    updateExample();

    m_notation->parts()->replaceDrumset(m_instrumentKey, m_editedDrumset);
}

//---------------------------------------------------------
//   updateExample
//---------------------------------------------------------
void CustomizeKitDialog::updateExample()
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
    note->mutldata()->cachedNoteheadSym.set_value(static_cast<SymId>(quarterCmb->currentData().toInt()));
    chord->add(note);
    Stem* stem = Factory::createStem(chord.get());
    stem->setParent(chord.get());
    stem->setBaseLength(Spatium(up ? -3.0 : 3.0));
    engravingRenderer()->layoutItem(stem);
    chord->add(stem);
    drumNote->appendElement(chord, m_editedDrumset.translatedName(pitch));
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void CustomizeKitDialog::load()
{
    std::vector<std::string> filter = { muse::trc("palette", "MuseScore drumset file") + " (*.drm)" };
    muse::io::path_t dir = notationConfiguration()->userStylesPath();
    muse::io::path_t fname = interactive()->selectOpeningFileSync(muse::trc("palette", "Load drumset"), dir, filter);

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
                auto result = interactive()->warningSync(
                    muse::trc("palette", "Drumset file too old"),
                    muse::trc("palette", "MuseScore Studio may not be able to load this drumset file."), {
                    muse::IInteractive::Button::Cancel,
                    muse::IInteractive::Button::Ignore
                }, muse::IInteractive::Button::Cancel);

                if (result.standardButton() != muse::IInteractive::Button::Ignore) { // covers Cancel and Esc
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

void CustomizeKitDialog::save()
{
    std::vector<std::string> filter = { muse::trc("palette", "MuseScore drumset file") + " (*.drm)" };
    muse::io::path_t dir = notationConfiguration()->userStylesPath();
    muse::io::path_t fname = interactive()->selectSavingFileSync(muse::trc("palette", "Save drumset"), dir, filter);

    if (fname.empty()) {
        return;
    }

    File f(fname);
    if (!f.open(IODevice::WriteOnly)) {
        QString s = muse::qtrc("palette", "Opening file\n%1\nfailed: %2").arg(f.filePath().toQString(), strerror(errno));
        interactive()->error(muse::trc("palette", "Open file"), s.toStdString());
        return;
    }
    valueChanged();    //save last changes in name
    XmlWriter xml(&f);
    xml.startDocument();
    xml.startElement("museScore", { { "version", Constants::MSC_VERSION_STR } });
    m_editedDrumset.save(xml);
    xml.endElement();
    if (f.hasError()) {
        QString s = muse::qtrc("palette", "Writing file failed: %1").arg(QString::fromStdString(f.errorString()));
        interactive()->error(muse::trc("palette", "Write drumset"), s.toStdString());
    }
}

//---------------------------------------------------------
//   customQuarterChanged
//---------------------------------------------------------
void CustomizeKitDialog::customQuarterChanged(int)
{
    updateExample();
}

void CustomizeKitDialog::defineShortcut()
{
    QTreeWidgetItem* item = pitchList->currentItem();
    if (!item) {
        return;
    }

    const int originPitch = item->data(Column::PITCH, Qt::UserRole).toInt();
    if (!PercussionUtilities::editPercussionShortcut(m_editedDrumset, originPitch)) {
        return;
    }

    const QString editedShortcutText = m_editedDrumset.shortcut(originPitch);
    shortcut->setText(!editedShortcutText.isEmpty() ? editedShortcutText : muse::qtrc("shortcuts", "None"));
    item->setText(Column::SHORTCUT, !editedShortcutText.isEmpty() ? editedShortcutText : QString());
    // TODO: Update the item of the conflict shortcut too (#26226)

    valueChanged();
}
