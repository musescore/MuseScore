import QtQuick 2.0
import MuseScore 1.0

MuseScore
{
	menuPath: "Plugins.cursor_seek"
	onRun:
	{
		openLog("cursor_seek3.log");
		logn("test script cursor_seek");

		var	cursor			= curScore.newCursor();
		logn("SELECTION: from track " + curScore.selectionFirstTrack + " to track " + curScore.selectionLastTrack);

		cursor.filter		= Segment.ChordRest;
		cursorSeek(cursor);

		// use a temporary variable as, for some unkown reason, direct assignment
		// to Cursor.filter gives wrong results with flag combinations
		var filter			= (Segment.EndBarLine + Segment.ChordRest);
		cursor.filter		= filter;
		cursorSeek(cursor);

		closeLog();
		Qt.quit();
	}

	function cursorSeek(cursor)
	{
		logn("FILTER: " + cursor.filter);
		for (var currTrack = 0; currTrack <= curScore.ntracks - 1; currTrack++)
		{
			cursor.track = currTrack;
			log2("TRACK: ", cursor.track);
			cursor.scoreStart();
			log("	Score start:");
			cursorStats(cursor);
			cursor.selectionStart();
			log("	Sel. start: ");
			cursorStats(cursor);
			cursor.selectionEnd();
			log("	Sel. end:   ");
			cursorStats(cursor);
			cursor.scoreEnd();
			log("	Score end:  ");
			cursorStats(cursor);
		}
	}

	function cursorStats(cursor)
	{
		log2("	cursor tick:  ", (cursor.segment ? cursor.tick : "NULL") );
	}
}
