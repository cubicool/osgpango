PROJECT(osgpangoguiviewer)

# osgPango gets the proper debug postfix automatically since it's part of the
# current project, but we need to add the postfix for the other libs.
LINK_LIBRARIES(osgPango
               debug     OpenThreads${CMAKE_DEBUG_POSTFIX}
               optimized OpenThreads
               debug     osgDB${CMAKE_DEBUG_POSTFIX}
               optimized osgDB
)

ADD_EXECUTABLE(osgpangoguiviewer osgpangoguiviewer.cpp)

INSTALL(TARGETS osgpangoguiviewer DESTINATION ${CMAKE_INSTALL_PREFIX}/share/OpenSceneGraph/bin)
SET_TARGET_PROPERTIES(osgpangoguiviewer PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
