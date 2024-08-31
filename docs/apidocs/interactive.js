// GENERATED, DON'T MODIFY 

/**
 * User interaction - informational messages, error messages, questions and other dialogs.
 * @namespace
 */

const interactive = {
    /**
     * Show information message
     * @param {String} text Text
     */
    info(text) {},

    /**
     * Show warning message
     * @param {String} text Text
     */
    warn(text) {},

    /**
     * Show error message
     * @param {String} text Text
     */
    error(text) {},

    /**
     * Ask a question
     * @param {String} [title] Title
     * @param {String} text Text of question
     * @param {ButtonList} buttons Buttons
     * @return {Button} Clicked button
     */
    question(title, text, buttons) {},
};
