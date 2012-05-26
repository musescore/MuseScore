//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Abc 2 xml calls a webservice at http://abc2xml.appspot.com to convert
//  an abc tune to MusicXML and open it with MuseScore for further editing
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

var outFile;
var reqId;
var defaultOpenDir = QDir.homePath();

function run()
      {
        var filename = QFileDialog.getOpenFileName(this, "MuseScore: Load ABC File", defaultOpenDir, "ABC file (*.abc)");
        if(filename){
          //read abc file
          var file = new QFile(filename);
          var line;
          if ( file.open(QIODevice.ReadOnly) ) {       
              // file opened successfully
              var t = new QTextStream( file ); // use a text stream
              var content ="";
              
              do {
                line = t.readLine(); // line of text excluding '\n'
                // do something with the line
                content +=line; 
                content +='\n';    // add the missing '\n'
              } while (line);        
              // Close the file
              file.close();
          }
          
          //print(content);
          
          //encode file content for url
          var encodedContent = QUrl.toPercentEncoding(content).toString();
          
          var url = "/abcrenderer?abc=" + encodedContent;
                    
          //print(url);
          
          //call the webservice and save to temporary file        
          outFile = new QTemporaryFile(QDir.tempPath()+"/abc_XXXXXX.xml");
          outFile.open();
          var http = new QHttp();
          http.setHost("abc2xml.appspot.com", 80);
          http.requestFinished.connect(outFile,finished);
          reqId = http.get(url,outFile);
          
        }
      }

//---------------------------------------------------------
// display a message box with error message
//---------------------------------------------------------
function errorMessage(){
      mb = new QMessageBox();
      mb.setWindowTitle("Error: abc2xml conversion");
      mb.text = "An error occured during the conversion.<br />Try it manually at: <a href=\"http://abc2xml.appspot.com\">http://abc2xml.appspot.com</a>";
      mb.exec();
}

//---------------------------------------------------------
// get finished handler
//---------------------------------------------------------
function finished(id ,error){
  print("finished");
  print(id);
  if (error){
    errorMessage();
    return;
  }
  if (id == reqId){
    outFile.flush();
    outFile.close();
    if(outFile.size() > 200){
      var score   = new Score();
      score.load(outFile.fileName());    
    }else{
      errorMessage();
    }  
  }
}

QByteArray.prototype.toString = function()
{
   ts = new QTextStream( this, QIODevice.ReadOnly );
   return ts.readAll();
}

//---------------------------------------------------------
//    menu:  defines were the function will be placed
//           in the MuseScore menu structure
//---------------------------------------------------------

var mscorePlugin = {
      menu: 'Plugins.ABC Import',
      init: init,
      run:  run
      };

mscorePlugin;

