
We use the [jsdoc](https://github.com/jsdoc/jsdoc) tool to create API documentation.  
  
What does the documentation consist of:
* Static documentation - here is a tutorial and description of classes
* Description of API methods - located in the API implementation files. 
* Snapshots - here is the generated documentation for each version.
* index.html [docs/index.html] - this is the entry point for https://musescore.github.io 
* Tools for generation - here you will find a documentation extractor for API methods, a documentation generator (jsdoc) and automation scripts.
* Generation and update process - see description below  

### Static documentation

location: [docs/apidocs_static](https://github.com/musescore/MuseScore/tree/master/docs/apidocs_static)  

This is where static documentation is stored in the jsdoc format.
* index.md - home page of documentation
* Tutorials - see [jsdoc documentation about tutorials](https://jsdoc.app/about-tutorials). 


### Description of API methods 

Descriptions of API methods are located in `cpp` files where the implementation of the methods is located.   
The description is in `jsdoc` format (see [https://jsdoc.app](https://jsdoc.app)), but the first line should indicate that this is APIDOC documentation.  
Example:  
```
/** APIDOC namespace: log
 * Write messages to log and console
 * @namespace
 */
LogApi::LogApi(api::IApiEngine* e)
    : ApiObject(e)
{
}

/** APIDOC method
 * Write error message with default tag
 * @param {String} message Message
 */
void LogApi::error(const QString& message)
{
    error("Api", message);
}

/** APIDOC method
 * Write error message with tag
 * @param {String} tag Tag
 * @param {String} message Message
 */
void LogApi::error(const QString& tag, const QString& message)
{
    LOGE_T(tag.toStdString())() << message;
}
```
   
When generating documentation, we first extract this documentation from the `cpp` files.   
   
### Snapshots  

location: [docs/snapshots](https://github.com/musescore/MuseScore/tree/master/docs/snapshots) 

To be able to view the documentation for each version, we generate snapshots of the documentation for each version and place them in the appropriate folder, for example: docs/snapshots/4.5, docs/snapshots/4.6

### index.html 

location: [docs/index.html](https://github.com/musescore/MuseScore/blob/master/docs/index.html) - this is the entry point for https://musescore.github.io  
   
How does this work: 
* The organization has a repository [musescore.github.io](https://github.com/musescore/musescore.github.io) - this is a repository for site musescore.github.io. 
* Opening [musescore.github.io](https://musescore.github.io) by default, without any suffixes, will open [index.html](https://github.com/musescore/musescore.github.io/blob/main/index.html) in this repository.
* Right now, this index.html simply do a redirect to the MuseScore repository. i.e. by adding the repository name to musescore.github.io, like [musescore.github.io/MuseScore](https://musescore.github.io/MuseScore/), the index.html from this repository will be opened.
* GitHub first looks for index.html in the repository root; if it's not there, it looks for index.html in the `./docs` directory. 
* This is how our `docs/index.html` opens.

After generating a snapshot for a new version, we need to add a link to it in this `index.html`, like others. 

### Tools for generation   
  
location: [tools/jsdoc ](https://github.com/musescore/MuseScore/tree/master/tools/jsdoc)  

List of tools: 
* template - template for `jsdoc`
* jsdoc_extractor.js - this is a documentation extractor from `cpp` files. It extracts documentation and stores it in temporary, fake `js` files so that `jsdoc` can process them.  
* conf.json - file of configuration for `jsdoc` 
* jsdoc_install.sh - script of `jsdoc` install 
* jsdoc_run.sh - script for executing the generation process  
   
This tool requires [node.js](https://nodejs.org) and npm to be installed to work. To run scripts on Windows, use GitBash.  

The generated documentation is placed in a temporary dir `gen_apidoc`.

### Generation and update process

When developing a new version of an application, adding new APIs or changing current ones, or for those that don't have documentation, we need to write documentation, just like any other change in the repository.   
   
After releasing a new version of the application, we need to generate a snapshot of the API documentation for it.  

Process: 
1. Install [node.js](https://nodejs.org) and npm 
2. Run `bash ./tools/jsdoc/jsdoc_install.sh`  
3. Run `bash ./tools/jsdoc/jsdoc_run.sh`  
4. Look the result, open it in browser index.html from `./tools/jsdoc/gen_apidoc/`  
5. Copy dir `./tools/jsdoc/gen_apidoc/` to `docs/snapshots/x.x` 
6. Add a link to the new version in `docs/index.html`