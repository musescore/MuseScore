#include "sequencereditor.h"

#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "sequencertrackheader.h"
#include "sequencercliprow.h"

namespace Ms {
//---------------------------------------------------------
//   SequencerEditor
//---------------------------------------------------------

SequencerEditor::SequencerEditor(QWidget* parent)
    : QDockWidget(qApp->translate("Sequencer", "Sequencer"), parent)
{
    setupUi(this);

    _headerAreaLayout = new QVBoxLayout(headerColumn);
    _clipAreaLayout = new QVBoxLayout(clipColumn);

    connect(noteScroll->verticalScrollBar(), &QScrollBar::valueChanged, headerScroll->verticalScrollBar(), &QScrollBar::setValue);
}

//---------------------------------------------------------
//   ~SequencerEditor
//---------------------------------------------------------

SequencerEditor::~SequencerEditor()
{
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void SequencerEditor::setScore(Score* score)
{
    _score = score;
    rebuild();
}

//---------------------------------------------------------
//   rebuild
//---------------------------------------------------------

void SequencerEditor::rebuild()
{
    if (_headerAreaTearaway) {
        delete _headerAreaTearaway;
        _headerAreaTearaway = nullptr;
    }

    if (_clipAreaTearaway) {
        delete _clipAreaTearaway;
        _clipAreaTearaway = nullptr;
    }

    //nameArea
    if (!_score) {
        return;
    }

    //Headers
    _headerAreaTearaway = new QWidget(this);
    _headerAreaLayout->addWidget(_headerAreaTearaway);
    QVBoxLayout* _headerTearawayLayout = new QVBoxLayout(_headerAreaTearaway);

    for (Staff* staff: _score->staves()) {
        SequencerTrackHeader* header = new SequencerTrackHeader(staff, this);
        _headerTearawayLayout->addWidget(header);

        header->setMinimumSize(QSize(0, _rowHeight));
        header->setMaximumSize(QSize(16777215, _rowHeight));
    }

    //Clips
    _clipAreaTearaway = new QWidget(this);
    _clipAreaLayout->addWidget(_clipAreaTearaway);
    QVBoxLayout* _clipTearawayLayout = new QVBoxLayout(_clipAreaTearaway);

    for (Staff* staff: _score->staves()) {
        SequencerClipRow* widget = new SequencerClipRow(staff, this);
        _clipTearawayLayout->addWidget(widget);
        widget->setRowHeight(_rowHeight);
    }
}
}
