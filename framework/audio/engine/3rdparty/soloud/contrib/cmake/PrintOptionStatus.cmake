# Parameters:
# OPTION: boolean option.
# TEXT - option meaning text, printed between "[+/-]" and "enabled/disabled".
# PREPEND - optional parameter, count of spaces inserted before output.
#	Useful for indenting relative options.

MACRO (PRINT_OPTION_STATUS OPTION TEXT)
	SET (PREPEND "")
	IF (${ARGC} GREATER 2)
		IF (${ARGV2} GREATER 0)
			FOREACH (A RANGE 1 ${ARGV2})
				SET (PREPEND "${PREPEND} ")
			ENDFOREACH (A)
		ENDIF (${ARGV2} GREATER 0)
	ENDIF (${ARGC} GREATER 2)
	IF (${OPTION})
		MESSAGE ("${PREPEND}[+] ${TEXT} enabled")
	ELSE (${OPTION})
		MESSAGE ("${PREPEND}[-] ${TEXT} disabled")
	ENDIF (${OPTION})
ENDMACRO (PRINT_OPTION_STATUS)
