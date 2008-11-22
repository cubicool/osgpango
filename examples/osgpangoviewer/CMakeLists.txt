PROJECT(osgpangoviewer)

# osgPango gets the proper debug postfix automatically since it's part of the
# current project, but we need to add the postfix for the other libs.
LINK_LIBRARIES(osgPango
               debug     OpenThreads${CMAKE_DEBUG_POSTFIX}
               optimized OpenThreads
               debug     osgGA${CMAKE_DEBUG_POSTFIX}
               optimized osgGA
               debug     osgDB${CMAKE_DEBUG_POSTFIX}
               optimized osgDB)

ADD_EXECUTABLE(osgpangoviewer osgpangoviewer.cpp)

INSTALL(TARGETS osgpangoviewer DESTINATION ${CMAKE_INSTALL_PREFIX}/share/openscenegraph/bin)
SET_TARGET_PROPERTIES(osgpangoviewer PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
