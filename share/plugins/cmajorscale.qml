import QtQuick 2.0
import MuseScore 3.0

MuseScore {
    version: "1.0"
    description: "Generates a one-octave C major scale starting at the cursor position"
    title: "C Major Scale Generator"
    categoryCode: "composing-arranging-tools"
    requiresScore: true

    // C Major scale intervals (MIDI pitches relative to C4 = 60)
    property var cMajorScale: [60, 62, 64, 65, 67, 69, 71, 72]  // C D E F G A B C

    onRun: {
        if (!curScore) {
            console.log("No score opened")
            quit()
            return
        }

        // Start the modification command for undo support
        curScore.startCmd()

        // Create a cursor
        var cursor = curScore.newCursor()

        // Check if there's a selection, otherwise start from beginning
        cursor.rewind(Cursor.SELECTION_START)
        if (!cursor.segment) {
            // No selection, start from the beginning of the score
            cursor.rewind(Cursor.SCORE_START)
        }

        // Get the current track (staff and voice)
        var startTrack = cursor.track

        // Set note duration to quarter notes (1/4)
        cursor.setDuration(1, 4)

        // Generate the C major scale
        for (var i = 0; i < cMajorScale.length; i++) {
            // Make sure we're on the right track
            cursor.track = startTrack

            // Add the note (pitch value, addToChord = false)
            cursor.addNote(cMajorScale[i], false)

            // Move to next position (unless it's the last note)
            if (i < cMajorScale.length - 1) {
                cursor.next()
            }
        }

        // End the modification command
        curScore.endCmd()

        console.log("C Major scale generated successfully!")
        quit()
    }
}
