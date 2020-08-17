#ifndef __SEQUENCER_EDITOR_H__
#define __SEQUENCER_EDITOR_H__

#include "ui_sequencereditor.h"

namespace Ms {
class Score;
class SequencerClipRow;

class SequencerEditor : public QDockWidget, public Ui::SequencerEditor
{
    Q_OBJECT

    Score * _score;
    QVBoxLayout* _headerAreaLayout = nullptr;
    QWidget* _headerAreaTearaway = nullptr;
    QVBoxLayout* _clipAreaLayout = nullptr;
    QWidget* _clipAreaTearaway = nullptr;

    int _rowHeight = 40;

public:
    explicit SequencerEditor(QWidget* parent = nullptr);
    ~SequencerEditor();

    void setScore(Score* score);
private:

    void rebuild();
};
}

#endif // __SEQUENCER_EDITOR_H__
