PROJECT(osgpangocustomrenderer)

LINK_LIBRARIES(osgPango)

ADD_EXECUTABLE(osgpangocustomrenderer osgpangocustomrenderer.cpp)

INSTALL(TARGETS osgpangocustomrenderer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
