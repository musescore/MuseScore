//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Test plugin
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
//    This is ECMAScript code (ECMA-262 aka "JavaScript")
//

//---------------------------------------------------------
//    init
//    this function will be called on startup of
//    MuseScore
//---------------------------------------------------------

function init()
      {
      // print("test script init");
      }

//---------------------------------------------------------
//    run
//    this function will be called when activating the
//    plugin menu entry
//
//    global Variables:
//    pluginPath - contains the plugin path; file separator
//                 is "/"
//---------------------------------------------------------

function run()
      {
      if (typeof curScore === 'undefined')
            return;
      print("version:        ", mscoreVersion);
	  // print statements appear in the "Debug Output" tab
	  // when the Script Debugger is enabled in MuseScore:
	  // Help > Enable Script Debugger
      print("major version:  ", mscoreMajorVersion);
      print("minor version:  ", mscoreMinorVersion);
      print("update version: ", mscoreUpdateVersion);
      print("division:       ", division);
      print("plugin path: ", pluginPath);
      print("score name:  ", curScore.name);
      print("staves:      ", curScore.staves);
      var mb = new QMessageBox();
      mb.setWindowTitle("MuseScore: Test Plugin");
      mb.text = "Hello MuseScore!";
      mb.exec();
      }

//---------------------------------------------------------
//    menu:  defines where the function will be placed
//           in the menu structure
//---------------------------------------------------------

var mscorePlugin = {
      majorVersion: 1,
      minorVersion: 1,
      menu: 'Plugins.Test',
      init: init,
      run:  run
      };

mscorePlugin;

