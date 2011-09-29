// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPango/GlyphRenderer>
#include <osgPango/GlyphLayer>
#include <osgPango/Serialize>

OFFSET_CHECK (osgPango::GlyphRendererShadow, 1)
OFFSET_READ  (osgPango::GlyphRendererShadow, 1)
OFFSET_WRITE (osgPango::GlyphRendererShadow, 1)

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRendererShadow,
	new osgPango::GlyphRendererShadow(),
	osgPango::GlyphRendererShadow,
	"osg::Object osgPango::GlyphRenderer osgPango::GlyphRendererShadow"
) {
	ADD_USER_SERIALIZER(Offset);
}

