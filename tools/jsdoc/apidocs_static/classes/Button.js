// GENERATED, DON'T MODIFY 

/** Button - Buttons in dialogs.
    The button can be set as a regular string value, for example -"Yes", or as a structure with parameters:
    
    {
        role: "secondary",
        text: "Save and close",
        name: "saveandclose",
    }

        where role (String) is the purpose, the role of the button. Each role implies a logical meaning.
        Depending on the logical meaning, the button is displayed with the corresponding color.

        The role property can take the following values:
                    primary - the main (principal) action
                    secondary - the second most important action
                    positive - a positive action (eg <create>)
                    dismiss - cancel an action (eg <cancel all>)
                    attention - an action that requires attention (eg <delete>);            

        text property (String) - Button text;

        name property (String) - name (button identifier);


    Example of calling a dialog with buttons:
        var buttons = [
            {
                role: "secondary",
                text: "Save and close",
                name: "saveandclose",
            },
            {
                role: "dismiss",
                text: "Cancel",
                name: "cancel",
            }
        ];
        var askUserToSave = api.interactive.question(
            "Save entered data?",
            "",
            buttons
        );
         
 * @class
 */
var Button = {
    /** Yes */
    Yes: "",
    /** No */
    No: "",
    /** Ok */
    Ok: "",
    /** Cancel */
    Cancel: "",
    /** Save */
    Save: "",
    /** Continue*/
    Continue: "",
    /** Abort */
    Abort: "",
    /** Retry*/
    Retry: "",
    /** Ignore */
    Ignore: "",
    /** Close */
    Close: "",
    /** Apply */
    Apply: ""
};
