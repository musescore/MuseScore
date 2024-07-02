to_entries[] | [
	"    ",
	if (.key | in($tr[0])) then "muse::TranslatableString(\"engraving/sym\", \"" else "muse::TranslatableString::untranslatable(\"" end,
	(.value.description | gsub("\""; "\\\"")),
	if (.key | startswith("accidentalWyschnegradsky")) then " (Wyschnegradsky)" else "" end,
	"\"),"
] | join("")
