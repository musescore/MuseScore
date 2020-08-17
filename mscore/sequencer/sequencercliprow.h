#ifndef __SEQUENCER_CLIP_ROW_H__
#define __SEQUENCER_CLIP_ROW_H__

namespace Ms {
class Score;
class Staff;
class Chord;

class SequencerClipRow : public QWidget
{
    Q_OBJECT

    Staff * _staff;

    int _maxPitch = 127;
    int _minPitch = 0;
    float _pixPerWhole = 60; //default length of whole note
    float _xZoom = 1;
    int _rowHeight = 40;

public:
    SequencerClipRow(Staff* staff, QWidget* parent = nullptr);
    ~SequencerClipRow();

    float xZoom() { return _xZoom; }
    void setXZoom(float value) { _xZoom = value; rebuild(); }
    int rowHeight() { return _rowHeight; }
    void setRowHeight(int value) { _rowHeight = value; rebuild(); }

private:
    virtual void paintEvent(QPaintEvent* p);
    void paintChord(QPainter& p, QBrush& brush, Chord* chord);

    void rebuild();
    void addChord(Chord* chrd);
};
}

#endif // __SEQUENCER_CLIP_ROW_H__
