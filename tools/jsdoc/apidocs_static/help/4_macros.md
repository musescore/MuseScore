# Extension Macros 

**In progress**

Macros are extensions that do not have a user interface, there is only a script that does something. Usually this is a way to automate some actions.  
For macros extensions, we need to specify the appropriate type and file name using .js in the extension manifest.   
Like this:  
manifest.json
```
{
    "uri": "musescore://extensions/example1",
    "type": "macros",
    ...

    "main": "main.js"
}
```   

The entry point is a function named main   
main.js

```
function main() {
    api.log.info("called main from example 2")
}
```