#!/bin/bash
#
# For each Qt PlugIn dylib, replaces the @rpath with @executable_path
# This is necessary if Qt / Xcode builds the application so that all libraries
# are trying to find other libraries using rpath
# Run in the PlugIn directory, ie.
# cd ~/Products/Debug/Products/Delicode_NI_mate.app/Contents/PlugIns/imageformats
# cp ~/Products/Installers/Mac/replace_rpaths.sh ~/Products/Debug/Products/Delicode_NI_mate.app/Contents/PlugIns/imageformats
# sh replace_rpaths.sh
# Make sure the path was changed by running otool -L filename

# Get all .dylib files that are a Qt framework in a specific subdir
for FILE in $(find . -path './*dylib' ! -name '*.plist' ! -name 'Resources')
do
    echo "Handling file  " $FILE
    LIBNAME=$(echo $FILE | rev | cut -d'/' -f1 | rev)

    # Extract only the lines that contain @rpath
    for LINE in $(otool -L $FILE | grep '@rpath' | awk '{ print $1}' )
    do
        # Get only the @rpath component of the line
        RPATH=$(echo $LINE | awk '{ print $1}')
        LIBNAME2=$(echo $RPATH | rev | cut -d'/' -f1 | rev)

        # Replace @rpath with @executable_path
        EXEPATH=$(echo $RPATH |sed 's/rpath/executable_path\/..\/Frameworks/')
        if [ $LIBNAME = $LIBNAME2 ]; then
            # dylib cannot change its own rpath using the change command, but its id can be changed
            install_name_tool -id $EXEPATH $FILE
            echo "Changed id for file " $FILE " rpath " . $RPATH . " file " $FILE " libnames " $LIBNAME ", " $LIBNAME2
        else
            install_name_tool -change $RPATH $EXEPATH $FILE
            echo "Changed file " $FILE " rpath " $RPATH " to " $EXEPATH " libnames " $LIBNAME ", " $LIBNAME2
        fi
    done
done

