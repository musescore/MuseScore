#include "sequencertrackheader.h"
#include "ui_sequencertrackheader.h"

#include "libmscore/staff.h"
#include "libmscore/part.h"

namespace Ms {
//---------------------------------------------------------
//   SequencerTrackHeader
//---------------------------------------------------------

SequencerTrackHeader::SequencerTrackHeader(Staff* staff, QWidget* parent)
    : QWidget(parent),
    _staff(staff)
{
    setupUi(this);

    Part* part = _staff->part();
    QString name = part->partName();
    trackName->setText(name);
    int col = part->color();

    QString style = QStringLiteral("background: #%1").arg(col, 6, 16, QLatin1Char('0'));
    colorSwatch->setStyleSheet(style);
}

//---------------------------------------------------------
//   ~SequencerTrackHeader
//---------------------------------------------------------

SequencerTrackHeader::~SequencerTrackHeader()
{
}
}
