//===========================================================================
// New Retrograde
// https://github.com/ellejohara/newretrograde
//
// Copyright (C) 2020 Astrid Lydia Johannsen (ellejohara)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 3
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE
//===========================================================================

import QtQuick 2.0
import MuseScore 3.0

MuseScore {
	title: "New Retrograde"
	description: "Takes a selection of notes and reverses them."
	version: "1.0"
    categoryCode: "composing-arranging-tools"
    thumbnailName: "retrograde.png"

    function retrogradeSelection() {
		var cursor = curScore.newCursor(); // get the selection
		cursor.rewind(2); // go to the end of the selection

		if(!cursor.segment) { // if nothing selected
			console.log('nothing selected'); // say "nothing selected"
            quit(); // then quit
		} else {
			var endTick = cursor.tick; // mark the selection end tick
			cursor.rewind(1); // go to the beginning of the selection
			var startTick = cursor.tick; // mark the selection start tick
		}
		//console.log(startTick + ' - ' + endTick); // display selection start and end ticks

		var noteArray = []; // create a blank array


		while(cursor.segment && cursor.tick < endTick) { // while in the selection
			var e = cursor.element; // put current element into variable e
			if(e) { // if e exists
				if(e.type == Element.CHORD) { // if e is a note or chord
					var pitches = []; // put the note pitches of each chord into an array
					var notes = e.notes;
					for(var i = 0; i < notes.length; i++) { // iterate through each note in chord
						var note = notes[i]; // get the note pitch number
						pitches.push(note.pitch); // put pitch number into variable
					}
				}

				if(e.type == Element.REST) { // if e is a rest
					var pitches = 'REST'; // "REST" as pitch
				}

				var numer = e.duration.numerator; // numerator of duration
				var denom = e.duration.denominator; // denominator of duration

				noteArray.push([pitches, numer, denom]);
			}
			cursor.next(); // move to next tick
		}

		noteArray.reverse(); // this does the retrograde (reverse array)
		cursor.rewind(1); // go back to beginning of selection

		// this section rewrites the selection with the reversed array
		for(var i = 0; i < noteArray.length; i++) {
			var noteDur = noteArray[i];
			var pitches = noteDur[0]; // get note and chord pitches
			var numer = noteDur[1]; // duration numerator
			var denom = noteDur[2]; // duration denominator

			// set the duration
			cursor.setDuration(numer, denom);

			// if there is only a single note
			if(pitches.length == 1) {
				cursor.addNote(pitches[0]); // add note
			}

			// if there is a chord or rest
			if(pitches.length > 1) {

				// if rest
				if(pitches === 'REST') {
					cursor.addRest () // add rest
				} else {

					// if chord
					for(var j = 0; j < pitches.length; j++) {
						var pitch = pitches[j];
						if(j == 0) { // loop through each pitch of chord
							// write the root note in a new cursor position
							cursor.addNote(pitch); // root of chord
						} else {
							// write the notes to the same cursor position
							cursor.addNote(pitch, cursor); // remainder of notes in chord
						}
					}
				}
			}

		} // end for
	}


	onRun: {
        curScore.startCmd()
		retrogradeSelection();
        curScore.endCmd()

        quit()
	}
}
