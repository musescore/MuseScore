# An introduction to using Bravura Text

*Version 1.1, 30 October 2015*

Bravura Text is a [SMuFL-compliant] [1] font containing musical symbols intended for use in text-based applications such as word processors, text editors, desktop publishers, and it can also be used on the web.

Bravura Text is licensed under the [SIL Open Font License] [2], which means it is free to use, redistribute, and modify, but please do respect the conditions set out in the license.

## Glyph repertoire
Bravura Text is a reference implementation for SMuFL, the Standard Music Font Layout, and as such contains all of the symbols defined in the SMuFL specification. [Download the SMuFL specification] [3] for a complete list of the symbols in Bravura Text, including a description and code point.

SMuFL uses the Private Use Area of Unicode's Basic Multilingual Plane (U+E000 through U+FFFF) to encode all of the included symbols. This means that the symbols cannot be typed by using the alphanumeric keys on your computer's keyboard on their own.

## Setting up for Unicode input
Detailed instructions for how to enable Unicode input on all operating systems are beyond the scope of this document. SIL provides [an excellent page] [4] with links to information and tools for Windows, OS X and Linux. In this document some basic instructions are included, but you may need to refer to other resources for help with your operating system and application of choice.

### Windows
There are multiple ways to type Unicode characters on Windows. Here are three methods you can try:

#### Method 1: hexadecimal input followed by `Alt`
On Windows, some applications, such as Microsoft Word (part of the Microsoft Office suite) and WordPad (included with every Windows installation), have support for converting a typed alphanumeric sequence into a single Unicode character.

The Unicode code point for a G (treble) clef in Bravura Text is U+E050, which is a hexadecimal number (57424 in decimal). To enter this character in Word or WordPad, type `E050` followed by `Alt+X` (hold the `Alt` key and then hit the `X` key). The typed characters **E050** will be replaced by a single character, which may display as an empty rectangle or a rectangle containing **?**. Select this character and change the font to Bravura Text, and the G clef appears.

Although Unicode code points are normally written in the form U+E050, you can ignore the "U+" part when entering the code point in this and the following method.

#### Method 2: `Alt` + hexadecimal input
If the application you are using does not support the kind of conversion described above, you can enable Unicode input by holding `Alt`, then typing `+` **on the numeric keypad**, then typing the hexadecimal number on the numeric keypad, before finally releasing `Alt`. For example, to type the G clef (U+E050), hold `Alt`, type keypad `+`, then `E050`, then release `Alt`. Again, if Bravura Text is not the selected font, you may see an empty rectangle or a rectangle containing **?**. Select this character and change the font to Bravura Text, and the G clef appears.

#### Method 3: Character Map
The built-in Character Map application can copy any Unicode character to the system clipboard so that it can be pasted into another application. To run Character Map, click the `Start` button or hit the `Windows` key on your keyboard, then type `charmap` and hit `Return`. Character Map will run.

In Character Map, choose **Bravura Text** from the **Font** menu at the top of the window. Switch on the **Advanced view** checkbox at the bottom of the window, which makes extra options appear. Choose **Unicode** from the **Character set** menu, then choose **Unicode Subrange** from the bottom of the **Group by** menu. A further pop-up window appears, captioned **Group By**, showing a list of Unicode subranges; choose **Private Use Characters** from the bottom of the list. (You can then close the **Group By** window if you wish.)

The grid that occupies the main part of the Character Map window now displays all of the symbols in Bravura Text (unfortunately, it is not possible to enlarge the display, so they are rather small). Once you have located the character you require, either select it with the mouse and click **Select** or double-click it, and it is added to the **Characters to copy** edit control. Click **Copy** to copy the contents of **Characters to copy** to the clipboard, then switch to the application into which you want to paste the text, and choose `Edit` ▸ `Paste`, or type `Ctrl+V`.

#### Other methods
You may wish to experiment with third-party utilities that can assist with locating and inserting Unicode characters. No recommendation or warranty is implied by listing these free utilities, which you may try at your own risk:

* [CatchChar] [5] allows you to add commonly-used Unicode characters to a menu that can be triggered from within any application via a keyboard shortcut.
* [BabelMap] [6] is an advanced alternative to Windows's built-in Character Map application.

Other commercial (non-free) utilities are also available, including [PopChar] [7].

### OS X
For Mac computers running OS X, the simplest method to insert arbitrary Unicode characters is using the provided Unicode Hex Input input method. To enable it:

* In System Preferences, choose **Keyboard**.
* On the **Keyboard** tab, switch on **Show Keyboard & Character Viewers in menu bar**. When switched on, you will see a national flag corresponding to your computer keyboard's normal language and/or layout appear in the menu bar to the left of the Spotlight icon.
* On the **Input Sources** tab, click **+**, and in the sheet that appears, select **Others** in the left-hand list, then select **Unicode Hex Input** in the right-hand list, then click **Add** to close the sheet.
* Ensure **Show Input menu in menu bar** is switched on, then close System Preferences.

The Unicode Hex Input input method works in the majority of OS X applications. To try it out, for example, open a new text document in TextEdit. Bravura Text does not appear by default in the font menu in TextEdit's toolbar, because it is not an English-language font, so to choose Bravura Text you must show the Fonts panel by choosing `Format` ▸ `Font` ▸ `Show Fonts` or typing `⌘T`. In the Fonts panel, choose **All fonts** under **Collection**, then choose **Bravura Text** under **Family**, then close the Fonts panel (type `⌘T` again).

Bravura Text is now the chosen font (and will now appear in the font menu in TextEdit's toolbar for this document). To type a G (treble) clef, which has the code point U+E050, first choose the **Unicode Hex Input** input method from the input menu in the menu bar. Now hold down `Alt` and type `E050` (do not type the "U+" prefix), then release `Alt`.

If you want to switch between your normal language input method and the Unicode Hex Input method quickly, you can assign a system keyboard shortcut in the **Keyboard** pane of System Preferences. Choose the **Shortcuts** tab, then in the left-hand list choose **Input Sources**. Switch on the checkbox for either or both of **Select the previous input source** and **Select the next input source**, and assign a keyboard shortcut. The default shortcut suggested by OS X is used for Spotlight by default, so you may wish to assign another shortcut, e.g. `^Space` (`^` is the symbol that corresponds to the `Ctrl` key).

## Usage notes for Bravura Text

### Scale factor
Bravura Text is scaled such that the height of a five-line staff (e.g. U+E014) is approximately the same as the height of an upper case letter in a regular text font at the same point size. It is designed to be used both in-line, i.e. in the middle of a run of text at the same point size, and on its own, typically at a larger point size. As such, all symbols in Bravura Text are scaled appropriately to be drawn at the correct size on a five-line staff.

One exception to this is for the bold italic letters used for dynamics, which are scaled such that they are approximately the same size as a lower case letter in a regular text font at the same point size.

### Zero-width characters
Symbols depicting staves and staff lines in Bravura Text have zero width. This is to allow other musical symbols to be printed on top of them.

Staff line symbols are provided in three widths: narrow (one space wide); normal (two spaces wide); and wide (three spaces wide).

The space is the normal unit of measurement when working with printed music notation. One space is the vertical distance between the middle of one staff line and the middle of the staff line above; as such a five-line staff is four spaces tall.

Time signature digits also have zero width, to allow them to be positioned above one another. Leger line glyphs similarly have zero width.

All other glyphs have zero side-bearings, i.e. the advance width of each glyph is exactly equal to the bounding box of its symbol.

### Space characters
In order to insert space between symbols, use the following keys:

* Typing `Space` advances the input position by half a space.
* Typing `-` (hyphen) advances the input position by one space.
* Typing `=` (equals) advances the input position by two spaces.

### Default vertical positioning
Many glyphs in Bravura Text are provided at multiple vertical positions, so that they can appear at different staff positions. With the exception of common clefs, all glyphs that can be drawn at different vertical positions are positioned by default such that they will appear on the middle line of a five-line staff glyph (e.g. U+E014). The common clefs are positioned by default as follows:

* G clef (e.g. U+E050): as for a treble clef, i.e. the bottom of the lower loop aligned with the bottom staff line
* F clef (e.g. U+E061): as for a bass clef, i.e. the two dots positioned either side of the second-highest staff line
* C clef (e.g. U+E058): as for an alto clef, i.e. the center of the clef positioned on the middle staff line

The following ranges of glyphs (as defined in the SMuFL specification) can be moved to different vertical positions on the staff:

* Leger lines (within the **Staves** range)
* All noteheads (within the **Noteheads**, **Slash noteheads**, **Round and square noteheads**, **Note clusters**, **Note name noteheads ranges**)
* Precomposed notes with stems and flags (within the **Individual notes** and **Beamed groups of notes** ranges)
* Precomposed stems (within the **Stems** range)
* Flags for notes (within the **Flags** range)
* All accidentals (within the **Standard accidentals**, **Gould arrow quarter-tone accidentals**, **Stein-Zimmermann accidentals**, **Extended Stein-Zimmermann accidentals**, **Sims accidentals**, **Johnston accidentals for Just Intonation**, **Extended Helmholtz-Ellis JI accidentals**, all Sagittal ranges, **Wyschnegradsky accidentals**, **Arel-Ezgi-Uzdilek (AEU) accidentals**, **Turkish folk music accidentals**, **Persian accidentals**, **Other accidentals** ranges)
* Articulations (within the **Articulations** range)
* Fermatas, caesuras, breathmarks (within the **Holds and pauses** range)
* Rests (within the **Rests** range)
* 1-, 2- and 4-bar repeat indicators (within the **Bar repeats** range)
* Medieval and Renaissance clefs, prolation and mensuration signs, noteheads and stems, individual notes, plainchant single- and multi-note forms, plainchant articulations, and accidentals (within the **Medieval and Renaissance...** ranges)
* **Kievan square notation**

### Altering vertical position
Bravura Text uses OpenType ligatures to modify the default vertical position of symbols. In OpenType fonts, ligatures are a kind of glyph substitution, where two or more glyphs are replaced with another single glyph. This is commonly used in text fonts to produce an elegant appearance for particular combinations of letters, such as "fi" or "fl".

In Bravura Text, ligatures are used to adjust the vertical position of individual symbols. First, you enter the code point corresponding to the amount by which you want to change the vertical position, and then you enter the code point for the symbol itself. Provided the application you are using supports OpenType ligatures, you should see the symbol appear at the desired vertical position.

The code points to use to raise or lower the position of symbols are as follows (the pitch names shown in parentheses correspond to a five-line staff with a treble clef; the default vertical position for movable symbols is therefore B4):

* Raise by one staff position (C5): U+EB90
* Raise by two staff positions (D5): U+EB91
* Raise by three staff positions (E5): U+EB92
* Raise by four staff positions (F5): U+EB93
* Raise by five staff positions (G5): U+EB94
* Raise by six staff positions (A5): U+EB95
* Raise by seven staff positions (B5): U+EB96
* Raise by eight staff positions (C6): U+EB97
* Lower by one staff position (A4): U+EB98
* Lower by two staff positions (G4): U+EB99
* Lower by three staff positions (F4): U+EB9A
* Lower by four staff positions (E4): U+EB9B
* Lower by five staff positions (D4): U+EB9C
* Lower by six staff positions (C4): U+EB9D
* Lower by seven staff positions (B3): U+EB9E
* Lower by eight staff positions (A3): U+EB9F

So to position, say, a black notehead at the G4 staff position, you would first enter U+EB99 (lower by two staff positions) followed immediately by U+E0A4 (the black notehead).

Noteheads positioned outside the staff (i.e. raised or lowered by six or more staff positions) will not automatically show leger lines, so those must be added separately *before* the notehead (since they have zero width), and raised or lowered by the same number of staff positions.

Special code points are provided to shift time signature digits to the correct vertical position:

* Position as numerator (top number): U+E09E
* Position as denominator (bottom number): U+E09F

To enter the time signature 4/4, you would first enter U+E09E (position as numerator), followed immediately by U+E084 (time signature 4), then U+E09F (position as denominator), followed by U+E084 again. Finally, advance the input position by inputting one or more spaces.

### Further information
Detailed technical support is not available for the use of Bravura Text, but if you encounter any problems using this font, please use the [**smufl-discuss** mailing list] [8] to contact the SMuFL community about your problem.



[1]: http://www.smufl.org/ "SMuFL"
[2]: http://scripts.sil.org/	"SIL Open Font License"
[3]: http://www.smufl.org/files/smufl-0.9.pdf "SMuFL 0.9"
[4]: http://scripts.sil.org/cms/scripts/page.php?item_id=inputtoollinks "SIL Unicode input resources"
[5]: http://helpingthings.com/index.php/insert-unicode-characters "CatchChar"
[6]: http://www.babelstone.co.uk/Software/BabelMap.html "BabelMap"
[7]: http://www.ergonis.com/products/popcharwin/
[8]: http://www.smufl.org/discuss "SMuFL mailing lists"
