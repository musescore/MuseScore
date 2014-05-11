Filename convention
=====================

**part-_{1}_-_{2}_.mscx**

_{1}_ which element is tested

* `empty`     	 -> just notes
* `all` 		 -> all or as much as possible elements
* _element name_ -> tests for a specific element (fingering, symbol etc...)


{2} action - which action is tested for the given element

* _empty string_ =  no parts created 
* `parts` == parts created
* _actions for elements_ == (add, uadd, uradd, del, udel, urdel) for elements
* _actions for all_ == ()


Linked Parts
================

1. create
2. add
3. add undo
4. add redo
5. remove
6. remove undo
7. remove redo

```
1 234 567 ELEMENT
x xxx xxx SYMBOL
- --- --- TEXT
- --- --- INSTRUMENT_NAME
- --- --- BAR_LINE
- --- --- STEM_SLASH
- --- --- LINE
- --- --- BRACKET
- --- --- ARPEGGIO
- --- --- ACCIDENTAL
x --- --- NOTE
x --- --- STEM
x --- --- CLEF
x --- --- KEYSIG
x --- --- TIMESIG
x --- --- REST
x xxx xxx BREATH
- --- --- GLISSANDO
- --- --- REPEAT_MEASURE
- --- --- IMAGE
- --- --- TIE
- --- --- ARTICULATION
- --- --- CHORDLINE
- --- --- DYNAMIC
- --- --- BEAM
- --- --- HOOK
- --- --- LYRICS
- --- --- FIGURED_BASS
- --- --- MARKER
- --- --- JUMP
x xxx xxx FINGERING
- --- --- TUPLET
- --- --- TEMPO_TEXT
- --- --- STAFF_TEXT
- --- --- REHEARSAL_MARK
- --- --- INSTRUMENT_CHANGE
- --- --- HARMONY
- --- --- FRET_DIAGRAM
- --- --- BEND
- --- --- TREMOLOBAR
- --- --- VOLTA
- --- --- SLUR_SEGMENT
- --- --- HAIRPIN_SEGMENT
- --- --- OTTAVA_SEGMENT
- --- --- TRILL_SEGMENT
- --- --- TEXTLINE_SEGMENT
- --- --- VOLTA_SEGMENT
- --- --- LAYOUT_BREAK
- --- --- SPACER
- --- --- STAFF_STATE
- --- --- LEDGER_LINE
- --- --- NOTEHEAD
- --- --- NOTEDOT
- --- --- TREMOLO
- --- --- MEASURE
- --- --- STAFF_LINES
- --- --- RUBBERBAND
- --- --- TAB_DURATION_SYMBOL
- --- --- FSYMBOL

- --- --- HAIRPIN
- --- --- OTTAVA
- --- --- PEDAL
- --- --- TRILL
- --- --- TEXTLINE
- --- --- SLUR

- --- --- HBOX
x --- --- VBOX
- --- --- TBOX
- --- --- FBOX
- --- --- ACCIDENTAL_BRACKET
```