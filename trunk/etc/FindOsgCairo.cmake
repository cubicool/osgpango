# Locate osgCairo
# This module defines
# OSGCAIRO_LIB_DIR
# OSGCAIRO_INCLUDE_DIR, where to find the headers
#


FIND_PATH(OSGCAIRO_INCLUDE_DIR osgCairo/Surface
    $ENV{OSGCAIRODIR}/include
    $ENV{OSGCAIRODIR}
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
  SET(LIBOSGCAIRO "osgCairo.lib")
  SET(LIBOSGCAIRODEBUG "osgCairod.lib")
ELSE (WIN32)
  SET(LIBOSGCAIRO "libosgCairo.so")
  SET(LIBOSGCAIRODEBUG "libosgCairod.so")
ENDIF (WIN32)

FIND_PATH(OSGCAIRO_LIB_DIR ${LIBOSGCAIRO} ${LIBOSGCAIRODEBUG}
    $ENV{OSGCAIRODIR}/lib
    $ENV{OSGCAIRODIR}
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

SET(OSGCAIRO_FOUND "NO")
IF(OSGCAIRO_LIB_DIR AND OSGCAIRO_INCLUDE_DIR)
    SET(OSGCAIRO_FOUND "YES")
ENDIF(OSGCAIRO_LIB_DIR AND OSGCAIRO_INCLUDE_DIR)
