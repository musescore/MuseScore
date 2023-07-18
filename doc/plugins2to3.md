\page plugin2to3 Porting MuseScore 2 plugins

This page describes changes between MuseScore 2.X and MuseScore 3 plugins API.

### Import statement
Replace **import MuseScore 1.0** with **import MuseScore 3.0** to start porting a 2.X plugin.
This is needed to distinguish between plugins for different MuseScore versions.

If your plugin uses \ref Ms::PluginAPI::FileIO "FileIO" API, replace also
**import FileIO 1.0** with **import FileIO 3.0**.

### Properties assignments
Most of the properties should remain the same as in MuseScore 2.X. Most notable differences:

- TimeSig doesn't have setSig() function anymore. \par
Replace
\code ts.setSig(numerator, denominator) \endcode
with
\code ts.timesig = fraction(numerator, denominator) \endcode

- No **pos** property available. \par
The Autoplacement engine makes it not necessary to adjust elements' position in most cases.
If this is still needed, use **offset** or **offsetX** and **offsetY** properties to adjust position offset:
\code element.offset = Qt.point(x, y); \endcode
or
\code
element.offsetX = x;
element.offsetY = y;
\endcode
where **x**, **y** are arbitrary numbers (in spatium units).

.

### Enumerations
Most of the enumerations exposed to QML plugins remain the same but some were renamed compared to MuseScore 2.X API. The renamed enumerations include:

- \ref Ms::PluginAPI::PluginAPI::Placement "Placement" — element placement above or below staff \par
**Element.ABOVE, Element.BELOW** → **Placement.ABOVE, Placement.BELOW**

- \ref Ms::PluginAPI::PluginAPI::Direction "Direction" — vertical direction \par
**MScore.UP, MScore.DOWN, MScore.AUTO** → **Direction.UP, Direction.DOWN, Direction.AUTO**

- \ref Ms::PluginAPI::PluginAPI::DirectionH "DirectionH" — horizontal direction \par
**MScore.LEFT, MScore.RIGHT, MScore.AUTO** → **DirectionH.LEFT, DirectionH.RIGHT, DirectionH.AUTO**

- \ref Ms::PluginAPI::PluginAPI::OrnamentStyle "OrnamentStyle" \par
 **MScore.DEFAULT, MScore.BAROQUE** → **OrnamentStyle.DEFAULT, OrnamentStyle.BAROQUE**

- \ref Ms::PluginAPI::PluginAPI::GlissandoStyle "GlissandoStyle" \par
**MScore.CHROMATIC, MScore.WHITE_KEYS, MScore.BLACK_KEYS, MScore.DIATONIC**
→
**GlissandoStyle.CHROMATIC** etc.

- **TextStyleType** → \ref Ms::PluginAPI::PluginAPI::Tid "Tid"

- \ref Ms::PluginAPI::PluginAPI::NoteHeadType "NoteHeadType" \par
**HEAD_AUTO, HEAD_WHOLE, HEAD_HALF, HEAD_QUARTER, HEAD_BREVIS, HEAD_TYPES** are now in **NoteHeadType** enumeration.

- \ref Ms::PluginAPI::PluginAPI::NoteHeadGroup "NoteHeadGroup" \par
Other **HEAD_\*** values are in **NoteHeadGroup** enumeration.

- \ref Ms::PluginAPI::PluginAPI::NoteValueType "NoteValueType" \par
**Note.OFFSET_VAL, Note.USER_VAL** → **NoteValueType.OFFSET_VAL, NoteValueType.USER_VAL**

