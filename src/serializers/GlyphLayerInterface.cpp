// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgPango/GlyphRenderer>

static bool checkTextureSize(const osgPango::GlyphRenderer& gr) {
}

static bool readTextureSize(osgDB::InputStream& is, osgPango::GlyphRenderer& gr) {
}

static bool writeTextureSize(osgDB::OutputStream& os, const osgPango::GlyphRenderer& gr) {
}

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphLayerInterface_Offset,
	0,
	osgPango::GlyphRenderer,
	"osg::Object osgPango::GlyphRenderer"
) {
	ADD_UINT_SERIALIZER(PixelSpacing, 1);
	
	ADD_USER_SERIALIZER(TextureSize);
	ADD_USER_SERIALIZER(MinFilterMode);
	ADD_USER_SERIALIZER(FontGlyphCacheMap);
}

