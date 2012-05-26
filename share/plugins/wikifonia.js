//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Test plugin 2
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
//    This is ECMAScript code (ECMA-262 aka "Java Script")
//

//---------------------------------------------------------
//    init
//    this function will be called on startup of
//    mscore
//---------------------------------------------------------

function init()
      {



      }

//---------------------------------------------------------
//    run
//    this function will be called when activating the
//    plugin menu entry
//---------------------------------------------------------

function run()
      {
	var loader = new QUiLoader(null);
	var file = new QFile("authentification2.ui");
	file.open(QIODevice.OpenMode(QIODevice.ReadOnly, QIODevice.Text));

	var form = loader.load(file,null);
qs_break();
	try
       	{
var fun = function() {
print("toto"); }
;
		//form.buttonBox.accepted.connect(fun);	
form.accepted.connect(fun);
		form.buttonBox.rejected.connect(reject);
	}
	catch(e) {
		print(e);
	}

	form.show();

      }

function accept()
      {

	print("accept");
      }


function reject()
      {

        print("reject");
      }


var mscorePlugin = {
      menu: 'Wikifonia.Export ...',
      init: init,
      run:  run
      };

return mscorePlugin;

