\page ticklength Tick Length values

Note and rest values are expressed in an internal unit, to express their mutual relationships and to make them independent from actual *tempo*. This unit is used by the following object properties:
- \ref Ms::PluginAPI::PluginAPI::division (defines a number of ticks per quarter note)
- \ref Ms::PluginAPI::Segment::tick
- Other properties (to be added to this list)

### Tick Lengths

All tick lengths in this table are defined through the \ref Ms::PluginAPI::PluginAPI::division "division" property. Raw numeric values are better to be avoided.

Duration									| Ticks value
--------------------------------------------|------------------------
whole note (semibreve)						| `4 * division`
double-dotted half note						| `3.5 * division`
dotted half note							| `3 * division`
triplet whole note (1/3 of breve)			| `8 * division / 3`
half note (minim)							| `2 * division`
double-dotted crochet						| `1.75 * division`
dotted crochet								| `1.5 * division`
triplet half note (1/3 of semibreve)		| `4 * division / 3`
1/4 note (crochet)							| `division`
double-dotted quaver						| `0.875 * division`
dotted quaver								| `0.75 * division`
triplet crochet (1/3 of minim)				| `2 * division / 3`
1/8 note (quaver)							| `division / 2`
double-dotted semiquaver					| `0.4375 * division`
dotted semiquaver							| `0.375 * division`
triplet quaver (1/3 of crochet)				| `division / 3`
1/16 note (semiquaver)						| `division / 4`
double-dotted demi-semiquaver				| `0.21875 * division`
dotted demi-semiquaver						| `0.1875 * division`
triplet semiquaver (1/3 of quaver)			| `division / 6`
1/32 note (demi-semiquaver)					| `division / 8`
dotted hemi-demi-semiquaver					| `0.09375 * division`
triplet demi-semiquaver (1/3 of semiquaver)	| `division / 12`
1/64 note (hemi-demi-semiquaver)			| `division / 16`
