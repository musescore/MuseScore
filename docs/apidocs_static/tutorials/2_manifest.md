
Each extension must contain a **manifest.json** file. This is what is searched for to detect the extension and obtain information about it.  
  
manifest.json
```
{
    "uri": "musescore://extensions/myquickstart",
    "type": "macros",
    "title": "My quick start",
    "description": "This is a development extension for API research.",
    "thumbnail": "my_icon.png",
    "version":  "0.1",
    "vendor": "Me"
    "ui_context": "Any",
    "apiversion": 2,

    "actions": [
        {
            "path": "main.js"
        }
    ]
}
```

### uri (required) 
This is the extension identifier, used to call it.  
It must be unique and follow the pattern: `musescore://extensions/extensions_name`

### type (required) 
Type of extension
* `macros` - extension without user interface
* `form` - extension with user interface
* `composite` - extension consisting of several forms or forms and macros 

### title (required)
Title of extension. Please write a concise and clear title.

### description (required) 
Description of extension. Please write a one-sentence description to make it clear what this extension does.

### thumbnail (optional) 
Thumbnail of extension. PNG format is supported.

### category (optional)
Category of extension.

### version (optional)
Version of extension.

### vendor (optional)
About the author

### apiversion (optional)
The API version used:
* `1` - api of old plugins, deprecated, don't use this for new extensions
* `2` (default) - actual api  

### ui_context (optional)
The context in which the extension is available:  
* `ProjectOpened` (default) - Project (score) open 
* `Any` - always available

### actions (required) 
List of extension actions. 
An extension typically has one action, but can have multiple. 
Actions are displayed as menu items in `Menu->Plugins` by default.  
However, they can also be displayed as buttons on the toolbar.   

example 
```
    ...
    "actions": [
        {"code": "configure", "type": "form", "title": "Configure", "path": "configure.qml"},
        {"code": "add", "type": "macros", "title": "Add", "path": "add.js"},
        {"code": "remove", "type": "macros", "title": "Remove", "path": "remove.js"}
    ]
```

#### action: code (optional)
Action code 

#### action: type (required for type `composite`/optional for `form`/`macros`)
If an extension consists of forms and macros (type `composite`), then you must specify the type of each action. If the extension has a single action, or all actions are of the same type, then you don't need to specify it; the action type will be equivalent to the extension type.   

#### action: title (optional)  
The action title, if not specified, will be equivalent to the extension title. 

#### action: icon (optional)  
Action icon, displayed in the menu or on the toolbar.   
see {@link Qml.IconCode|IconCode}  

#### action: ui_context (optional)
The context in which the action is available.  
If not specified, then equivalent to the extension `ui_context`.

#### action: show_on_appmenu (optional)
Whether to show the action in the menu. Default is `true`.

#### action: show_on_toolbar (optional)
Whether to show the action on the toolbar. Default is `false`.

#### action: path (required)  
Path to the js script of macros or Qml form 

#### action: func (optional)  
For macros only. The name of the function to call.   
This means you can specify the same script file but different functions for different actions.  
If not specified, the default function name is `main`.
