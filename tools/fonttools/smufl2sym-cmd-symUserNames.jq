to_entries[] | [
	"    ",
	if (.key | in($tr[0])) then "QT_TRANSLATE_NOOP(\"engraving/sym\", \"" else "\"" end,
	(.value.description | gsub("\""; "\\\"")),
	if (.key | startswith("accidentalWyschnegradsky")) then " (Wyschnegradsky)" else "" end,
	if (.key | in($tr[0])) then "\")," else "\"," end
] | join("")
