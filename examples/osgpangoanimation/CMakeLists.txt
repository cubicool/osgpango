PROJECT(osgpangoanimation)

LINK_LIBRARIES(osgPango)

ADD_EXECUTABLE(osgpangoanimation osgpangoanimation.cpp)

INSTALL(TARGETS osgpangoanimation DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
