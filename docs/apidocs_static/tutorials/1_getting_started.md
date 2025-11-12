
### Adding a new non-visual extension  

Create a new folder in the custom extensions folder:  
* Windows - `C:\Users\username\AppData\Local\MuseScore\MuseScore4\extensions`  
* MacOS -   
* Linux - `$HOME/.local/share/MuseScore/MuseScore4/extensions`  

for example `myquickstart`. 

In this folder, create a new file `manifest.json`  
Fill out the manifest as follows:   
```
{
    "uri": "musescore://extensions/myquickstart",
    "type": "macros",
    "title": "My quick start",
    "description": "This is a development extension for API research.",

    "path": "main.js"
}
```

Create a new file `main.js`  
Fill out the manifest as follows:  
```
function main() {
    api.log.info("called main from myquickstart")
    api.interactive.info("myquickstart", "called MAIN from myquickstart")
}
```
   
Launch MuseScore and enable your extension on the plugins page.  
Open a project (some a score or create new) and call your extension from the plugins menu.   
A dialog should be displayed with information from your extension.   

Congratulations! You've created and added a new extension!   

### Adding a new visual extension 

Create a new folder in the custom extensions folder:  
* Windows - `C:\Users\username\AppData\Local\MuseScore\MuseScore4\extensions`  
* MacOS -   
* Linux - `$HOME/.local/share/MuseScore/MuseScore4/extensions`  

for example `myquickstart_v`. 

In this folder, create a new file `manifest.json`  
Fill out the manifest as follows:   
```
{
    "uri": "musescore://extensions/myquickstart_v",
    "type": "form",
    "title": "My quick start visual",
    "description": "This is a development extension for API research.",

    "path": "Main.qml"
}
```

Create a new file `Main.qml`  
Fill out the manifest as follows:  
```
import QtQuick

import MuseApi.Extensions
import MuseApi.Controls

ExtensionBlank {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: api.theme.backgroundPrimaryColor

    StyledTextLabel {
        id: label1
        anchors.centerIn: parent
        text: "Quick start"
    }

    FlatButton {
        id: btn1
        anchors.top: label1.bottom
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter

        text: "Click me"

        onClicked: {
            api.interactive.info("Quick start", "Clicked on button")
        }
    }
}
```
   
Launch MuseScore and enable your extension on the plugins page.   
Open a project (some a score or create new) and call your extension from the plugins menu.   
A dialog with you form.   

Congratulations! You've created and added a new visual extension!   
