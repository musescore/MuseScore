# Extension Macros  

Macros are extensions that do not have a user interface, there is only a script that does something. Usually this is a way to automate some actions.  
For macros extensions, we need to specify the appropriate type and file name using .js in the extension manifest.   
Like this:  
manifest.json
```
{
    "uri": "musescore://extensions/example1",
    "type": "macros",
    ...

    "actions": [
        {
            "path": "main.js"
        }
    ]
}
```   

In the manifest you can specify the name of the function that will be called; if not specified, then the `main` function will be called by default. You can also use the various services provided, which are described in the documentation.     
  
main.js
```
const Log = require("MuseApi.Log");
const Interactive = require("MuseApi.Interactive");

function main() {
    Log.info("called main from example 2")
    Interactive.info("Quick start", "called main from example 2")
}
```