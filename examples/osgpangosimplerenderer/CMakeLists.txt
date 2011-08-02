PROJECT(osgpangosimplerenderer)

LINK_LIBRARIES(osgPango
	debug     osgViewer${CMAKE_DEBUG_POSTFIX}
	optimized osgViewer
	debug     osgGA${CMAKE_DEBUG_POSTFIX}
	optimized osgGA
	debug     osgDB${CMAKE_DEBUG_POSTFIX}
	optimized osgDB
)

ADD_EXECUTABLE(osgpangosimplerenderer osgpangosimplerenderer.cpp)

INSTALL(TARGETS osgpangosimplerenderer DESTINATION ${CMAKE_INSTALL_PREFIX}/share/OpenSceneGraph/bin)
SET_TARGET_PROPERTIES(osgpangosimplerenderer PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
