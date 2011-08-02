PROJECT(osgpangodistancefield)

LINK_LIBRARIES(osgPango
	debug     osgViewer${CMAKE_DEBUG_POSTFIX}
	optimized osgViewer
	debug     osgGA${CMAKE_DEBUG_POSTFIX}
	optimized osgGA
)

ADD_EXECUTABLE(osgpangodistancefield osgpangodistancefield.cpp)

INSTALL(TARGETS osgpangodistancefield DESTINATION ${CMAKE_INSTALL_PREFIX}/share/OpenSceneGraph/bin)
SET_TARGET_PROPERTIES(osgpangodistancefield PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
