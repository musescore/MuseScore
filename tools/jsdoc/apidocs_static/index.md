# Extensions

**In progress**

Extensions are a way to extend the basic functionality and add new features to the application.  
  
The application has built-in extensions, we can find them at Home - Plugins. We can also install third-party extensions.   
   
*TODO how install third-party*
   
Before the extension will work, we need to enable it. This can be done at Home - Plugins. Enabled extensions will be available for calling from the Menu - Plugins.  

   
## Development
     
Now we have two types of extensions:
* **forms** are extensions that have a user interface through which we can control the extension logic.
* **macros** are extensions without a user interface, having only a script that performs some actions (usually some kind of automation)  
  
There is also a special type - **complex**. This is an extension that can consist of several forms, or a form and macros.

Each extension consists of a {@tutorial 2_manifest} file that describes the extension - its identifier, type, title, icon, scripts, config and etc.
Qml is used to create {@tutorial 3_form} user interfaces, and javascript is used to write {@tutorial 3_macros scripts. Api is provided to access the application, and API UI is provided UI controls.   

