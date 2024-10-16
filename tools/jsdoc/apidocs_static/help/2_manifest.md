# Extension manifest 

**In progress**

Each extension must contain a **manifest.json** file. This is what is searched for to detect the extension and obtain information about it.  
  
manifest.json
```
{
    "uri": "musescore://extensions/colornotes",
    "type": "macros",
    "title": "Color Notes",
    "description": "This plugin colors notes in the selection depending on their pitch as per the Boomwhackers convention",
    "category": "color-notes",
    "thumbnail": "color_notes.png",
    "version":  "3.5",
    "vendor": "Muse"
    "ui_context": "Any",
    "apiversion": 1,

    "main": "main.js"
}
```

* uri (required) - This is the extension identifier, used to call it. It must be unique and follow the pattern: `musescore://extensions/extensions_name`
* type (required) - Type of extension
  * macros - extension without user interface
  * form - extension with user interface
  * complex - extension consisting of several forms or forms and macros 
* title (required) - Title of extension  
* description (required) - Description of extension
* category (optional)- [Category of extension](#Categories) 
* thumbnail (optional) - Thumbnail of extension. PNG format is supported.
* version (optional) - Version of extension.
* vendor (optional) - About the author
* ui_context (optional) - The context in which the extension is available
  * ProjectOpened (default) - Project (score) open 
  * Any - always available
* apiversion (optional)- required api version
  * 1 - api of old plugins, deprecated, don't use this for new extensions
  * 2 (default) - actual api   
* main (required) - Main entry point, .js or .qml file

## Categories 

*TODO*

