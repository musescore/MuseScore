//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Test plugin
//
//  Copyright (C)2009 Werner Schweer and others
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
// error handling is mostly omitted!
//

//---------------------------------------------------------
//    init
//---------------------------------------------------------

function init()
      {
      };

var form;

//---------------------------------------------------------
//    run
//    create gui from qt designer generated file test4.ui
//---------------------------------------------------------

function run()
      {
      if (typeof curScore === 'undefined')
            return;
      var loader = new QUiLoader(null);
      var file   = new QFile(pluginPath + "/removenotes.ui");
      file.open(QIODevice.OpenMode(QIODevice.ReadOnly, QIODevice.Text));
      form = loader.load(file, null);
      form.buttonBox.accepted.connect(accept);
      form.show();
      };

//---------------------------------------------------------
//    removeNote
//    from all chords with three notes remove note with
//    index "idx"
//---------------------------------------------------------

function removeNote(idx)
      {
      var cursor = new Cursor(curScore);
      cursor.staff = 0;
      cursor.voice = 0;
      cursor.rewind();
      curScore.startUndo();

      while (!cursor.eos()) {
            if (cursor.isChord()) {
                  var chord = cursor.chord();
                  if (chord.notes == 3) {
                        chord.removeNote(idx);
                        }
                  }
            cursor.next();
            }
      curScore.endUndo();
      };

//---------------------------------------------------------
//    accept
//    called when user presses "Accept" button
//---------------------------------------------------------

function accept()
      {
      var idx = -1;
      if (form.topNote.checked)
            idx = 0;
      else if (form.middleNote.checked)
            idx = 1;
      else if (form.bottomNote.checked)
            idx = 2;
      if (idx != -1)
            removeNote(idx);
      }

var mscorePlugin = {
      menu: 'Plugins.Remove Notes',
      init: init,
      run:  run
      };

mscorePlugin;

