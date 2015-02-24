This plugin adds the note names of all notes in either the current selection or all voices of all staves in the entire score as staff text (above/below the staff) and uses note names according to the locale MuseScore is configured for, rather than just the English note names C, D, E, F, G, A, B.
So the output changes with the setting of Menu -> Edit -> Preferences... -> General -> Language, resp. if that is set to 'System', the output depends on the language setting of your PC.
It also names notes with sharps, double sharps and double flats and the notename moves aside a bit, if it would otherwise collide with the note.

Available locales: English, German, Dutch, Japanese, Italian, French, Spanish, Portuguese, Russian, Romainan, Danish, Norwegian, Swedish, Polish, Slovak, Czech and Greek. Translations to more languages/locales are welcome

The double sharp and double flat notes as well as Fb, Cb, E# and B# still need translation into Spanish, Portuguese, Russian, Romanian and Greek, help is more than welcome.

If you also want it to show courtesy- and microtonal accidentals, change `false` to `true` in the plugin code. Note however, that none of these have yet been translated, and their 'clear text' names can be rather long (e.g. "mirrored-flat-slash").

If you want a separator different from ",", change the corresponding variable in the plugin code, you can also change it to "\n" to get the note names stacked vertically, but in that case most probably also need to modify the position it gets printed.

The idea for this plugin stems from a [discussion in the forum] (http://musescore.org/en/node/16786), the microtonal extension from [another discussion in the forum] (http://musescore.org/en/node/16870).
