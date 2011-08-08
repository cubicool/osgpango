// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>

USE_SERIALIZER_WRAPPER(osgPango_GlyphRenderer)

extern "C" void wrapper_serializer_library_osgPango(void) {
	// When is this called?
	OSG_NOTICE << "wrapper_serializer_library_osgPango" << std::endl;
}

