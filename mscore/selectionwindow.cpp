//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "selectionwindow.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/select.h"
#include "palette/palettewidget.h"
#include "scoreaccessibility.h"

namespace Ms {
// see `SelectionFilter::canSelect()` in libmscore/select.cpp
// and `enum class SelectionFilterType` in libmscore/select.h,
// keep in sync with the latter!
static const char* labels[] = {
    QT_TRANSLATE_NOOP("selectionfilter", "All"),
    QT_TRANSLATE_NOOP("selectionfilter", "Voice 1"),
    QT_TRANSLATE_NOOP("selectionfilter", "Voice 2"),
    QT_TRANSLATE_NOOP("selectionfilter", "Voice 3"),
    QT_TRANSLATE_NOOP("selectionfilter", "Voice 4"),
    QT_TRANSLATE_NOOP("selectionfilter", "Dynamics"),
    QT_TRANSLATE_NOOP("selectionfilter", "Hairpins"),
    QT_TRANSLATE_NOOP("selectionfilter", "Fingerings"),
    QT_TRANSLATE_NOOP("selectionfilter", "Lyrics"),
    QT_TRANSLATE_NOOP("selectionfilter", "Chord Symbols"),
    QT_TRANSLATE_NOOP("selectionfilter", "Other Text"),
    QT_TRANSLATE_NOOP("selectionfilter", "Articulations"),
    QT_TRANSLATE_NOOP("selectionfilter", "Ornaments"),
    QT_TRANSLATE_NOOP("selectionfilter", "Slurs"),
    QT_TRANSLATE_NOOP("selectionfilter", "Figured Bass"),
    QT_TRANSLATE_NOOP("selectionfilter", "Ottavas"),
    QT_TRANSLATE_NOOP("selectionfilter", "Pedal Lines"),
    QT_TRANSLATE_NOOP("selectionfilter", "Other Lines"),
    QT_TRANSLATE_NOOP("selectionfilter", "Arpeggios"),
    QT_TRANSLATE_NOOP("selectionfilter", "Glissandi"),
    QT_TRANSLATE_NOOP("selectionfilter", "Fretboard Diagrams"),
    QT_TRANSLATE_NOOP("selectionfilter", "Breath Marks"),
    QT_TRANSLATE_NOOP("selectionfilter", "Tremolos"),
    QT_TRANSLATE_NOOP("selectionfilter", "Grace Notes")
};

static const size_t numLabels = sizeof(labels) / sizeof(labels[0]);

SelectionListWidget::SelectionListWidget(QWidget* parent)
    : QListWidget(parent)
{
    setAccessibleName(tr("Selection filter"));
    setAccessibleDescription(tr("Use Tab and Backtab (Shift+Tab) to move through the check boxes"));
    setFrameShape(QFrame::NoFrame);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setFocusPolicy(Qt::TabFocus);
    setTabKeyNavigation(true);

    for (size_t row = 0; row < numLabels; row++) {
        QListWidgetItem* listItem = new QListWidgetItem(this);
        listItem->setData(Qt::UserRole, row == 0 ? QVariant(-1) : QVariant(1 << (row - 1)));
        listItem->setCheckState(Qt::Unchecked);
        addItem(listItem);
    }
    retranslate();
}

void SelectionListWidget::retranslate()
{
    for (size_t row = 0; row < numLabels; row++) {
        QListWidgetItem* listItem = item(int(row));
        listItem->setText(qApp->translate("selectionfilter", labels[row]));
        listItem->setData(Qt::AccessibleTextRole, qApp->translate("selectionfilter", labels[row]));
    }
}

void SelectionListWidget::focusInEvent(QFocusEvent* e)
{
    setCurrentRow(0);
    QListWidget::focusInEvent(e);
}

SelectionWindow::SelectionWindow(QWidget* parent, Score* score)
    : QDockWidget(parent)
{
    setObjectName("SelectionWindow");
    setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
    _score = score;

    _listWidget = new SelectionListWidget;
    setWidget(_listWidget);

    //?MuseScore::restoreGeometry(this);

    updateFilteredElements();
    connect(_listWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(changeCheckbox(QListWidgetItem*)));
    retranslate();
}

void SelectionWindow::retranslate()
{
    setWindowTitle(tr("Selection Filter"));
    _listWidget->retranslate();
}

SelectionWindow::~SelectionWindow()
{
    //if (isVisible()) {
    //      MuseScore::saveGeometry(this);
    //      }
}

//---------------------------------------------------------
//   updateFilteredElements
//---------------------------------------------------------

void SelectionWindow::updateFilteredElements()
{
    if (!_score) {
        return;
    }
    int filter = _score->selectionFilter().filtered();
    bool all = true;
    bool none = true;
    _listWidget->blockSignals(true);
    for (int row = 1; row < _listWidget->count(); row++) {
        QListWidgetItem* item = _listWidget->item(row);
        if (filter & 1 << (row - 1)) {
            if (item->checkState() != Qt::Checked) {
                item->setCheckState(Qt::Checked);
            }
            none = false;
        } else {
            if (item->checkState() != Qt::Unchecked) {
                item->setCheckState(Qt::Unchecked);
            }
            all = false;
        }
    }
    QListWidgetItem* item = _listWidget->item(0);
    Qt::CheckState state = all ? Qt::Checked : (none ? Qt::Unchecked : Qt::PartiallyChecked);
    if (item->checkState() != state) {
        item->setCheckState(state);
    }
    _listWidget->blockSignals(false);
}

//---------------------------------------------------------
//   changeCheckbox
//---------------------------------------------------------

void SelectionWindow::changeCheckbox(QListWidgetItem* item)
{
    if (!_score) {
        return;
    }
    int type = item->data(Qt::UserRole).toInt();
    bool set = false;
    item->checkState() == Qt::Checked ? set = true : set = false;
    if (type > 0) {
        _score->selectionFilter().setFiltered(static_cast<SelectionFilterType>(type), set);
    } else {
        for (size_t row = 1; row < numLabels; row++) {
            _score->selectionFilter().setFiltered(static_cast<SelectionFilterType>(1 << (row - 1)), set);
        }
    }
    _score->startCmd();
    if (_score->selection().isRange()) {
        _score->selection().updateSelectedElements();
    }
    updateFilteredElements();
    _score->setUpdateAll();
    _score->endCmd();
    ScoreAccessibility::instance()->updateAccessibilityInfo();
}

//---------------------------------------------------------
//   showSelectionWindow
//---------------------------------------------------------

void MuseScore::showSelectionWindow(bool visible)
{
    QAction* a = getAction("toggle-selection-window");
    if (selectionWindow == 0) {
        selectionWindow = new SelectionWindow(this,this->currentScore());
        connect(selectionWindow, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
        addDockWidget(Qt::LeftDockWidgetArea,selectionWindow);
        if (paletteWidget && paletteWidget->isVisible()) {
            tabifyDockWidget(paletteWidget, selectionWindow);
        }
    }
    reDisplayDockWidget(selectionWindow, visible);
    a->setChecked(visible);
    if (visible) {
        selectionWindow->raise();
    }
}

void SelectionWindow::closeEvent(QCloseEvent* ev)
{
    emit closed(false);
    QWidget::closeEvent(ev);
}

void SelectionWindow::hideEvent(QHideEvent* ev)
{
    //MuseScore::saveGeometry(this);
    QWidget::hideEvent(ev);
}

void SelectionWindow::setScore(Score* score)
{
    _score = score;
    updateFilteredElements();
}

QSize SelectionWindow::sizeHint() const
{
    return QSize(170 * guiScaling, 170 * guiScaling);
}

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void SelectionWindow::changeEvent(QEvent* event)
{
    QDockWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
}
}
