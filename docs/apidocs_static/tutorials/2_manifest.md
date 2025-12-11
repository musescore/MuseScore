
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

    "path": "main.js"
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

### thumbnail (optional) - Thumbnail of extension. PNG format is supported.


* category (optional)- [Category of extension](#Categories) 
* version (optional) - Version of extension.
* vendor (optional) - About the author
* ui_context (optional) - The context in which the extension is available
  * ProjectOpened (default) - Project (score) open 
  * Any - always available
* apiversion (optional)- required api version
  * 1 - api of old plugins, deprecated, don't use this for new extensions
  * 2 (default) - actual api   
* path (required) - path to main entry point, .js or .qml file



