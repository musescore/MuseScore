# Adding a new language

Steps to follow after we receive a [request for a new language](https://github.com/musescore/MuseScore/wiki/Help-translate-MuseScore-Studio#adding-a-new-language):

1. Add the new language [on Transifex](https://app.transifex.com/musescore/musescore/languages/).

2. Run `tools/translations/tx_pull.sh` to fetch the `musescore_LANG.ts` and `instruments_LANG.ts` files.

3. Add the language name and/or code to these files:

    - `share/locale/languages.json`
    - `buildscripts/packaging/macOS/Info.plist.in`

4. Commit the changes and submit a PR.
