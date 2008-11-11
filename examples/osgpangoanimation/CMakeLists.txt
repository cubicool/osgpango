PROJECT(osgpangoanimation)

# osgPango gets the proper debug postfix automatically since it's part of the
# current project, but we need to add the postfix for the other libs.
LINK_LIBRARIES(osgPango
               debug     OpenThreads${CMAKE_DEBUG_POSTFIX}
               optimized OpenThreads)

ADD_EXECUTABLE(osgpangoanimation osgpangoanimation.cpp)

INSTALL(TARGETS osgpangoanimation DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
SET_TARGET_PROPERTIES(osgpangoanimation PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
