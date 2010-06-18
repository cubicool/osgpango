# Locate glib
# This module defines
# GLIB_LIBRARY_DIRS
# GLIB_INCLUDE_DIRS, where to find the headers
#

FIND_PATH(GLIB_INCLUDE_DIRS glib.h
    $ENV{GLIBDIR}/include
    $ENV{GLIBDIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
)

IF (WIN32)
  SET(LIBGLIB "gobject-2.0.lib")
ELSE (WIN32)
  SET(LIBGLIB "libgobject-2.0.so")
ENDIF (WIN32)

FIND_PATH(GLIB_LIBRARY_DIRS ${LIBGLIB}
    $ENV{GLIBDIR}/lib
    $ENV{GLIBDIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

SET(GLIB_FOUND "NO")
IF(GLIB_LIBRARY_DIRS AND GLIB_INCLUDE_DIRS)
    SET(GLIB_FOUND "YES")
ENDIF(GLIB_LIBRARY_DIRS AND GLIB_INCLUDE_DIRS)
