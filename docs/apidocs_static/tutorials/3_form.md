# Extension Forms 

**In progress**

Forms are extensions that have a user interface. To create UI, Qml is used. For form extensions, we need to specify the appropriate type and file name using .qml in the extension manifest.   
Like this:  
manifest.json
```
{
    "uri": "musescore://extensions/example1",
    "type": "form",
    ...

    "actions": [
        {
            "path": "Main.qml"
        }
    ]
}
```

To create a UI, use `ExtensionBlank` as the root element. For this we need to do `import MuseApi.Controls`. 
Example of a simple interface:   
```
import QtQuick

import MuseApi.Controls
import MuseApi.Log
import MuseApi.Interactive

ExtensionBlank {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    Component.onCompleted: {
        Log.info("Component.onCompleted from example1")
    }

    StyledTextLabel {
        id: label1
        text: "This is example1"
    }

    FlatButton {
        id: btn1
        anchors.top: label1.bottom

        text: "Click me"

        onClicked: {
            Interactive.info("Example1", "Clicked on Btn1")
        }
    }
}

```