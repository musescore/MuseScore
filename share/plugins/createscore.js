//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Test plugin
//
//  Copyright (C)2008-2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

//
//    This is ECMAScript code (ECMA-262 aka "Java Script")
//

//---------------------------------------------------------
//    init
//    this function will be called on startup of
//    mscore
//---------------------------------------------------------

function init()
      {
      // print("test script init");
      };


function addNote(cursor, pitch, duration)
      {
      var chord     = new Chord();
      chord.tickLen = duration;
      var note      = new Note();
      note.pitch    = pitch;

      chord.addNote(note);
      cursor.add(chord);
      cursor.next();
      };

function run()
      {
      var score   = new Score();
      score.name  = "Test-Score";
      score.title = "Test-Score";
      score.appendPart("Piano");    // create two staff piano part
      score.appendMeasures(5);      // append five empty messages
      var cursor = new Cursor(score);
      cursor.staff = 0;
      cursor.voice = 0;
      cursor.rewind();

      addNote(cursor, 60, 480);
      addNote(cursor, 62, 480);
      addNote(cursor, 64, 480);
      addNote(cursor, 65, 480);
      };

//---------------------------------------------------------
//    menu:  defines were the function will be placed
//           in the menu structure
//---------------------------------------------------------

var mscorePlugin = {
      menu: 'Plugins.Create Score',
      init: init,
      run:  run
      };

mscorePlugin;

