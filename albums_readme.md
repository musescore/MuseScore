Notes:

Today:
    - Improve the draw/don't draw front cover option.
    - Contents page -> pages + improved alignment.
    - Separate .album from .msca/.mscaz in the open options.
    - Changing tab while playing back a movement changes the scoreview to the wrong movement(fixed?).

Tests:
    - compressed album file Test (almost done)
    - old .album files

Week 3
(1 - 5):
    Bugfixes and refactoring.
    - Check all TODOs
    - Add the new members of MasterScore/Score to clone functions and save/write?
    Preferences.
    Decide if I allow only one active album or not. If not rename, partOfActiveAlbum to partOfAlbum or something.
(6 - 7):
    - Git cleanup.

Week 4:
    Mass changes:
     - Mass changes, Actions menu, used by both albums and groups of normal scores (multible tabs selected or opened scores)
     - Easy way to select multiple scores.
     - Actions:
        - Add - Replace instruments.
        - Change style.
        - Change footers/headers.

Week 5+ (further improvements):
    Mixer for album-mode (with all the instruments).
    Parts with different instrumentation.
    Timeline for the entire Temporary Album Score.
    Albums inside an Album.
    Tools to split a score into multiple scores (an album).
    Improved composer/lyricist handling. Give the user an option in the inspector to include (or not) that string in the front cover.
    Add new movement with the same instrumentation as the last one in the album


 Refactor and Code Quality:
 - Decouple albums and multi movement scores.
 - Refactor scoreview and mscore code.
 - Something better than cv->drawingScore()->title() == "Temporary Album Score"
 - MuseScore cs vs scoreView->score and drawingScore

Investigate:
 - Disabled score->doLayout in ScoreView::paintEvent, did I break something???
 - Closing tab without clicking (clicking changes MuseScore::cv to the current scoreview) crashes because it thinks that it should close a tab in the second tab bar (probably fixed)

Bugs:
 - Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff, sequencer setScoreView)
 - Crash when editing title of part
 - crashes related to text (are this all on master?)
