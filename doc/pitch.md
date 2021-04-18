\page pitch Note Pitch values

Note pitches are internally expressed with their respective MIDI pitch values. This corresponds to the absolute height of the note, regardless of the way it is actually written (enharmony; to retrieve the actual enharmony employed for a note, use its \ref tpc "Tonal Pitch Class" instead).
The pitch is used by the \ref Ms::PluginAPI::Note::pitch "Note.pitch" property.

### Note Pitch Values

Note \ Octave	| -1	| 0 	| 1 	| 2 	| 3 	| 4 				| 5 	| 6 	| 7 	| 8 	| 9
----------------|------:|------:|------:|------:|------:|------------------:|------:|------:|------:|------:|------:
C				| 0 	| 12	| 24	| 36	| 48	| 60 (middle C) 	| 72 	| 84 	| 96 	| 108 	| 120
C♯				| 1 	| 13	| 25	| 37	| 49	| 61 				| 73 	| 85 	| 97 	| 109 	| 121
D				| 2 	| 14	| 26	| 38	| 50	| 62 				| 74 	| 86 	| 98 	| 110 	| 122
D♯				| 3 	| 15	| 27	| 39	| 51	| 63 				| 75 	| 87 	| 99 	| 111 	| 123
E				| 4 	| 16	| 28	| 40	| 52	| 64 				| 76 	| 88 	| 100 	| 112 	| 124
F				| 5 	| 17	| 29	| 41	| 53	| 65 				| 77 	| 89 	| 101 	| 113 	| 125
F♯				| 6 	| 18	| 30	| 42	| 54	| 66 				| 78 	| 90 	| 102 	| 114 	| 126
G				| 7 	| 19	| 31	| 43	| 55	| 67 				| 79 	| 91 	| 103 	| 115 	| 127
G♯				| 8 	| 20	| 32	| 44	| 56	| 68 				| 80 	| 92 	| 104 	| 116 	| --
A				| 9 	| 21	| 33	| 45	| 57	| 69 				| 81 	| 93 	| 105 	| 117 	| --
A♯				| 10	| 22	| 34	| 46	| 58	| 70 				| 82 	| 94 	| 106 	| 118 	| --
B				| 11	| 23	| 35	| 47	| 59	| 71 				| 83 	| 95 	| 107 	| 119 	| --
