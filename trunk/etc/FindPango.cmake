# Locate pango
# This module defines
# PANGO_LIBRARY_DIRS
# PANGO_INCLUDE_DIRS, where to find the headers
#


FIND_PATH(PANGO_INCLUDE_DIRS pango/pango.h
    $ENV{PANGODIR}/include
    $ENV{PANGODIR}
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
  SET(LIBPANGO "pango-1.0.lib")
ELSE (WIN32)
  SET(LIBPANGO "libpango-1.0.so")
ENDIF (WIN32)

FIND_PATH(PANGO_LIBRARY_DIRS ${LIBPANGO}
    $ENV{PANGODIR}/lib
    $ENV{PANGODIR}
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

SET(PANGO_FOUND "NO")
IF(PANGO_LIBRARY_DIRS AND PANGO_INCLUDE_DIRS)
    SET(PANGO_FOUND "YES")
ENDIF(PANGO_LIBRARY_DIRS AND PANGO_INCLUDE_DIRS)
