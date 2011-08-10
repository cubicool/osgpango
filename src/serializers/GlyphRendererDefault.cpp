// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPango/GlyphRenderer>

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRendererDefault,
	new osgPango::GlyphRendererDefault(),
	osgPango::GlyphRendererDefault,
	"osg::Object osgPango::GlyphRenderer osgPango::GlyphRendererDefault"
) {
}

