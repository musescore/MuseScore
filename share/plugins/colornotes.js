//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Color notehead plugin
//	Noteheads are colored according to pitch. User can change to color by
//  modifying the colors array. First element is C, second C# etc...
//
//  Copyright (C)2008 Werner Schweer and others
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
// This is ECMAScript code (ECMA-262 aka "Java Script")
//


var colors = [new QColor(226,28,72),new QColor(242,102,34),new QColor(249,157,28),
new QColor(255,204,51),new QColor(255,243,43),new QColor(188,216,95),
new QColor(98,188,71),new QColor(0,156,149),new QColor(0,113,187),
new QColor(94,80,161),new QColor(141,91,166),new QColor(207,62,150)];


//---------------------------------------------------------
//    init
//    this function will be called on startup of mscore
//---------------------------------------------------------

function init()
      {
      // print("test script init");
      }

//-------------------------------------------------------------------
//    run
//    this function will be called when activating the
//    plugin menu entry
//
//    global Variables:
//    pluginPath - contains the plugin path; file separator is "/"
//-------------------------------------------------------------------

function run()
      {
      if (typeof curScore === 'undefined')
            return;
      var cursor = new Cursor(curScore);
      for (var staff = 0; staff < curScore.staves; ++staff) {
            cursor.staff = staff;
            for (var v = 0; v < 4; v++) {
                  cursor.voice = v;
                  cursor.rewind();  // set cursor to first chord/rest

                  while (!cursor.eos()) {
                        if (cursor.isChord()) {
                              var chord = cursor.chord();
                              var n     = chord.notes;
                              for (var i = 0; i < n; i++) {
                                    var note   = chord.note(i);
                                    note.color = new QColor(colors[note.pitch % 12]);
                                    }
                              }
                        cursor.next();
                        }
                  }
            }
      }

//---------------------------------------------------------
//    menu:  defines were the function will be placed
//           in the MuseScore menu structure
//---------------------------------------------------------

var mscorePlugin = {
      menu: 'Plugins.Color Notes',
      init: init,
      run:  run
      };

mscorePlugin;

