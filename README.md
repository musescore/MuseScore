# Musescore NC Version

**Musescore NC Version** is a customized version of the open-source music notation software, specifically tailored for development using **Gemini CLI** within the **Antigravity** IDE.

## What We Updated

We have made specific improvements to the codebase to enhance robustness and developer experience:

1.  **Fixed Soundfont Download Script**: Updated `buildscripts/cmake/DownloadSoundFont.cmake` to correctly check the status of auxiliary file downloads (License, Changelog, Readme). It now logs warnings instead of silently failing if these files cannot be retrieved.
2.  **Integrated Gemini CLI Guide**: Added comprehensive documentation on how to leverage the Gemini CLI for finding bugs, understanding code, and generating fixes.

## Development Guide: Gemini CLI in Antigravity

This version is designed to be developed using the **Gemini CLI** integrated into the **Antigravity** IDE. This setup allows for AI-assisted coding, debugging, and documentation.

### 1. Setup

The Gemini CLI comes pre-integrated in Antigravity.

*   **Activation**: When you first open the terminal, accept the prompt to connect Gemini CLI if it appears.
*   **Verification**: Run `gemini --version` to ensure the tool is active.

### 2. Basic Commands

Use these commands to interact with the codebase:

*   **`gemini chat "<question>"`**: Ask general questions about the project or logic.
*   **`gemini explain <file_path>`**: Get a detailed explanation of what a specific file does.
*   **`gemini code "<instruction>"`**: Generate code snippets or entire functions based on your requirements.
*   **`gemini debug`**: Analyze errors and get suggestions for fixes.

### 3. Workflow Example

To see this workflow in action, you can look at how we fixed the bug in `buildscripts/cmake/DownloadSoundFont.cmake`.

1.  **Identify**: We used `grep "TODO"` to find a task: `## TODO check STATUS of downloads`.
2.  **Understand**: We used `gemini explain` to understand the script's purpose.
3.  **Fix**: We used `gemini code` to generate the CMake logic for checking download statuses.
4.  **Verify**: We reviewed and applied the code to ensure robustness.

For a more detailed walkthrough, please refer to the [DEVELOPMENT_WITH_GEMINI.md](DEVELOPMENT_WITH_GEMINI.md) file included in this repository.

## Original Features

Musescore NC Version retains all the powerful features of the original MuseScore Studio:

- WYSIWYG design
- TrueType font support for high-quality scaling
- Fast note entry
- MusicXML, MIDI, and MEI import/export
- Integrated sequencer and software synthesizer

## License

Musescore NC Version is licensed under GPL version 3.0. See [license file](LICENSE.txt) in the same directory.
