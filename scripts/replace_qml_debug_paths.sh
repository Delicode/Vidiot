#!/bin/bash
#
# For each Qt frameworks .dylib file, runs install_name_tool so that they will try to find lib_debug instead of lib

for FILE in $(find . -path '*.dylib' ! -name '*.plist' ! -name 'Resources')
do
    echo "Handling file  " $FILE
    LIBNAME=$(echo $FILE | rev | cut -d'/' -f1 | rev)

    # Extract only the lines that contain @executable_path and not debug
    for LINE in $(otool -L $FILE | grep '@executable_path' | grep 'Qt' | grep -v 'debug' | awk '{ print $1}' )
    do
        # Get only the @rpath component of the line
        LIBPATH=$(echo $LINE | awk '{ print $1}')
        echo $LIBPATH
        LIBNAME2=$(echo $RPATH | rev | cut -d'/' -f1 | rev)
        # Append _debug to the end of all libs
        DEBUGPATH=$(echo $LIBPATH |sed 's/$/_debug/')
        if [ "$LIBNAME" == "$LIBNAME2" ]; then
            # No need to change the libs own name as it was already done in modify_bundle 
            echo "Not converting the lib itself"
        else
            install_name_tool -change $LIBPATH $DEBUGPATH $FILE
            echo "Changed file " $FILE " rpath " $LIBPATH " to " $DEBUGPATH " libnames " $LIBNAME ", " $LIBNAME2
        fi

    done
done

