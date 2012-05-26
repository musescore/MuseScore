//=============================================================================
// MuseScore
// Linux Music Score Editor
// $Id:$
//
// Test plugin
//
// Copyright (C)2009 Werner Schweer and others
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

var colors = [new QColor(0,0,0),new QColor(255,0,0),new QColor(0,255,0),
new QColor(0,0,255),new QColor(128,0,0),new QColor(255,0,128),
new QColor(0,128,0),new QColor(0,0,160),new QColor(0,113,187),
new QColor(94,80,161),new QColor(141,91,166),new QColor(207,62,150)];

//
// error handling is mostly omitted!
//

//---------------------------------------------------------
// init
//---------------------------------------------------------

function init()
      {
      };

var form;

//---------------------------------------------------------
// run
// create gui from qt designer generated file test4.ui
//---------------------------------------------------------

function run()
      {
      var loader = new QUiLoader(null);
      var file = new QFile(pluginPath + "/bbscolornotes.ui");
      file.open(QIODevice.OpenMode(QIODevice.ReadOnly, QIODevice.Text));
      form = loader.load(file, null);
      form.buttonBox.accepted.connect(accept);
      form.show();
      };

//---------------------------------------------------------
// removeNote
// from all chords with three notes remove note with
// index "idx"
//---------------------------------------------------------

function colorNote(idx)
      {
      var cursor = new Cursor(curScore);
      for (var staff = 0; staff < curScore.staves; ++staff) {
            cursor.staff = staff;
            for (var v = 0; v < 4; v++) {
                  cursor.voice = v;
                  cursor.rewind(); // set cursor to first chord/rest
                  var chosenColor = 0;
                  if(idx < 1) {
                        chosenColor = ((4 *staff) + v) % 12;
                        }

                  while (!cursor.eos()) {
                        if (cursor.isChord()) {
                              for (var i = 0; i < cursor.chord().notes(); i++) {
                                    var note = cursor.chord().note(i);
                                    note.color = new QColor(colors[chosenColor]);
                                    }
                              }
                        cursor.next();
                        }
                  }
            }
      };

//---------------------------------------------------------
// accept
// called when user presses "Accept" button
//---------------------------------------------------------

function accept()
      {
      var idx = -1;
      if (form.colorHead.checked)
            idx = 0;
      else if (form.allBlack.checked)
            idx = 1;
      if (idx != -1)
            colorNote(idx);
      }

var mscorePlugin = {
      menu: 'Plugins.BBS Color Notes',
      init: init,
      run: run
      };

mscorePlugin;

