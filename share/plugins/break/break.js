//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Break X measures plugin
//	This plugin will add a line break every X measures in the selection or in 
//  in the full score if no selection
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
      var loader = new QUiLoader(null);
      var file   = new QFile(pluginPath + "/break.ui");
      file.open(QIODevice.OpenMode(QIODevice.ReadOnly, QIODevice.Text));
      form = loader.load(file, null);
      form.buttonBox.accepted.connect(accept);
      form.exec();
      }

//---------------------------------------------------------
//    accept
//    called when user presses "Accept" button
//---------------------------------------------------------

function accept()
    {
      var value = form.mSpinBox.value;
      
      var cursor   = new Cursor(curScore);
      cursor.staff = 0;
      cursor.voice = 0;
      cursor.goToSelectionEnd();
      var endTick = cursor.tick(); // if no selection, go to end of score
      cursor.goToSelectionStart();
      if (cursor.eos()) {
            cursor.rewind();       // if no selection, start at beginning of score
      }

      var i = 1;
      curScore.startUndo();
      while (cursor.tick() < endTick) {
        
            var m = cursor.measure();
            if(value != 0){
                if (i % value == 0) {
                      m.lineBreak = true;    
                }
                else {
                      m.lineBreak = false;
                }
            }else{
                m.lineBreak = false;
            }
            cursor.nextMeasure();
            i++;
      }
      curScore.endUndo();
    }

//---------------------------------------------------------
//    menu:  defines were the function will be placed
//           in the MuseScore menu structure
//---------------------------------------------------------

var mscorePlugin = {
      menu: 'Plugins.Break Every X Measures',
      init: init,
      run:  run
      };

mscorePlugin;
