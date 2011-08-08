// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgPango/GlyphRenderer>

/*
	std::vector<osg::ref_ptr<GlyphLayer> > _layers;
	unsigned int                           _pixelSpacing;
	unsigned int                           _textureWidth;
	unsigned int                           _textureHeight;
	osg::Texture::FilterMode               _minFilter;
	FontGlyphCacheMap                      _gc;
	std::string                            _name;
*/

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRenderer,
	0,
	osgPango::GlyphRenderer,
	"osg::Object osgPango::GlyphRenderer"
) {
	ADD_UINT_SERIALIZER( PixelSpacing, 1 );
}

