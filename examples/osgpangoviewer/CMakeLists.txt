PROJECT(osgpangoviewer)

LINK_LIBRARIES(osgPango
	debug     osgViewer${CMAKE_DEBUG_POSTFIX}
	optimized osgViewer
	debug     osgGA${CMAKE_DEBUG_POSTFIX}
	optimized osgGA
	debug     osgDB${CMAKE_DEBUG_POSTFIX}
	optimized osgDB
)

ADD_EXECUTABLE(osgpangoviewer osgpangoviewer.cpp)

INSTALL(TARGETS osgpangoviewer DESTINATION ${CMAKE_INSTALL_PREFIX}/share/OpenSceneGraph/bin)
SET_TARGET_PROPERTIES(osgpangoviewer PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
