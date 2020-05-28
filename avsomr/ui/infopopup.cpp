//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "infopopup.h"

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QBitmap>
#include <QMouseEvent>

#include "mscore/scoreview.h"

#include "avslog.h"

namespace {
static int POPUP_MARGIN{ 10 };
}

using namespace Ms;
using namespace Ms::Avs;

InfoPopup::InfoPopup()
{
    setFixedSize(220, 80);
    setWindowFlags(Qt::Popup);
    setFrameStyle(QFrame::Panel | QFrame::Raised);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::ToolTipBase);

    _recognizedCheck = new QCheckBox(this);
    QObject::connect(_recognizedCheck, &QCheckBox::stateChanged, [this](int state) {
        emit recognizedCheckedChanged(Qt::Checked == state);
    });

    _notrecognizedCheck = new QCheckBox(this);
    QObject::connect(_notrecognizedCheck, &QCheckBox::stateChanged, [this](int state) {
        emit notrecognizedCheckedChanged(Qt::Checked == state);
    });
}

//---------------------------------------------------------
//   setRecognizedChecked
//---------------------------------------------------------

void InfoPopup::setRecognizedChecked(bool arg)
{
    _recognizedCheck->setChecked(arg);
}

//---------------------------------------------------------
//   setNotRecognizedChecked
//---------------------------------------------------------

void InfoPopup::setNotRecognizedChecked(bool arg)
{
    _notrecognizedCheck->setChecked(arg);
}

//---------------------------------------------------------
//   showOnView
//---------------------------------------------------------

void InfoPopup::showOnView(ScoreView* view,
                           const AvsOmr::Info& info)
{
    IF_ASSERT(view) {
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    auto addRow = [this, layout](QCheckBox* check, const QColor& color, const QString& text) {
                      QHBoxLayout* row = new QHBoxLayout(this);

                      row->addWidget(check, 0, Qt::AlignLeft);

                      QFrame* rec = new QFrame(this);
                      rec->setFrameStyle(QFrame::Panel);
                      rec->setFixedSize(40, 20);
                      rec->setStyleSheet(QString("background-color: %1;").arg(color.name()));
                      row->addWidget(rec, 0, Qt::AlignLeft);

                      QLabel* label = new QLabel(this);
                      label->setText(text);
                      row->addWidget(label, 1);

                      layout->addLayout(row);
                  };

    addRow(_recognizedCheck, info.usedColor, QString("- recognized"));
    addRow(_notrecognizedCheck, info.freeColor, QString("- not recognized"));

    setLayout(layout);

    setParent(view);

    QObject::connect(view, &ScoreView::viewRectChanged, this, &InfoPopup::onViewChanged);
    QObject::connect(view, &ScoreView::sizeChanged, this, &InfoPopup::onViewChanged);

    updatePopup(view);

    show();
}

//---------------------------------------------------------
//   onViewChanged
//---------------------------------------------------------

void InfoPopup::onViewChanged()
{
    ScoreView* view = static_cast<ScoreView*>(sender());
    updatePopup(view);
}

//---------------------------------------------------------
//   updatePopup
//---------------------------------------------------------

void InfoPopup::updatePopup(ScoreView* view)
{
    IF_ASSERT(view) {
        return;
    }

    move(view->width() - width() - POPUP_MARGIN, view->height() - height() - POPUP_MARGIN);

    //! NOTE When scrolling the view, only a small part is redrawn,
    //! therefore the previous position of the info panel remains drawn and
    //! the panel is draws in a new position, so the panel is duplicated.
    //! To prevent this from happening while the panel is showing,
    //! we will redraw the view completely.
    view->update();
}

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void InfoPopup::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void InfoPopup::mousePressEvent(QMouseEvent* event)
{
    event->accept();
}

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void InfoPopup::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
}
