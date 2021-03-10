#!/bin/sh

set -e
LC_ALL=C
LANGUAGE=C
unset LANGUAGE
export LC_ALL
cd "$(dirname "$0")"

jq -f smufl2sym-cmd-debug.jq \
    -r <../fonts/smufl/glyphnames.json \
    >smufl2sym-out-debug

{
	echo '    // SMuFL standard symbol IDs {{{'
	jq -f smufl2sym-cmd-SymId.jq \
	    -r <../../fonts/smufl/glyphnames.json
	echo '    // SMuFL standard symbol IDs }}}'
} >smufl2sym-out-symid.h-SymId

{
	echo '    // SMuFL standard symbol names {{{'
	jq -f smufl2sym-cmd-symNames.jq \
	    -r <../../fonts/smufl/glyphnames.json
	echo '    // SMuFL standard symbol names }}}'
} >smufl2sym-out-symnames.cpp-symNames

{
	echo '    // SMuFL standard symbol user names {{{'
	jq -f smufl2sym-cmd-symUserNames.jq \
	    --slurpfile tr smufl2sym-in-trans.json \
	    -r <../../fonts/smufl/glyphnames.json
	echo '    // SMuFL standard symbol user names }}}'
} >smufl2sym-out-symnames.cpp-symUserNames

echo 'All done!'
