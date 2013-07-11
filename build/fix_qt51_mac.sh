# Official Qt 5.1 package on Mac has a bug. Frameworks and plugin Library
# reports path with // when running otool -L. macdeployqt fails to operate 
# on these files. Run this script with fix_qt51_mac.sh QTDIRINSTALL to 
# to change the linked path to single /

QTDIR=$1
for F in `find $QTDIR/lib $QTDIR/plugins $QTDIR/qml  -perm 755 -type f` 
do 
	for P in `otool -L $F | awk '{print $1}'`
	do   
	    if [[ "$P" == *//* ]] 
	    then 
	        PSLASH=$(echo $P | sed 's,//,/,g')
	        install_name_tool -change $P $PSLASH $F
	    fi 
	 done
done