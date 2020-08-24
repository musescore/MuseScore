Notes:

Today:
    - musescore cs vs cv->score/drawingScore
    - move various public member variables to private
    - preferences (almost done, only got to load them)
    - tests (almost done, maybe combine the tests?)
    - rename
    - cursor when entering notes and undoing (sometimes its on the wrong score)

Tests:
    - compressed album file Test (almost done)

Week 3
(1 - 5):
    Bugfixes and refactoring.
    - Check all TODOs
    - Add the new members of MasterScore/Score to clone functions and save/write?
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

Bugs:
 - Fix all the places where inActiveAlbum should be inActiveAlbum and the dominantScore is the one in the scoreview (e.g. the teleporting stuff, sequencer setScoreView)
 - Crash when editing title of part
 - crashes related to text (are this all on master?)
