#ifndef __SEQUENCER_TRACK_HEADER_H__
#define __SEQUENCER_TRACK_HEADER_H__

#include <QWidget>

#include "ui_sequencertrackheader.h"

namespace Ms {
class Staff;

class SequencerTrackHeader : public QWidget, public Ui::SequencerTrackHeader
{
    Q_OBJECT

    Staff * _staff;

public:
    explicit SequencerTrackHeader(Staff* staff, QWidget* parent = nullptr);
    ~SequencerTrackHeader();
};
}

#endif // __SEQUENCER_TRACK_HEADER_H__
