#! /bin/sh

#  Copyright (C) 2009, 2010 Christian Egli
#  Copyright (C) 2016 Bert Frees

# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved. This file is offered as-is,
# without any warranty.

# Use this script to regenerate Makefile.am if you must. I recommend
# against it for the reasons explained in
# http://www.gnu.org/software/hello/manual/automake/Wildcards.html.
# It's easy to pick up some spurious files that you did not mean to
# distribute.
 
OUTFILE=Makefile.am.new
COMMAND="ls | grep -v Makefile | grep -v README | grep -v maketablelist.sh | grep -v '.*~$' | sort -df | sed -e 's/$/ \\\\/' -e 's/^/	/' -e '\$s/\\\\$//'"

cat <<EOF >$OUTFILE
# generate the list of tables as follows:
# $ ${COMMAND}
table_files = \\
EOF

eval "$COMMAND" >> $OUTFILE

cat <<'EOF' >>$OUTFILE

tablesdir = $(datadir)/liblouis/tables
tables_DATA = $(table_files)
EXTRA_DIST = $(table_files)
EOF

