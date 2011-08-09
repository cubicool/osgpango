// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgPango/GlyphRenderer>

// ------------------------------------------------------------------------------------ TextureSize
static bool checkTextureSize(const osgPango::GlyphRenderer& gr) {
	const osg::Vec2s& ts = gr.getTextureSize();

	return true;

	/*
	return (
		ts.x() != osgPango::DEFAULT_TEXTURE_WIDTH || 
		ts.y() != osgPango::DEFAULT_TEXTURE_HEIGHT
	);
	*/
}

static bool readTextureSize(osgDB::InputStream& is, osgPango::GlyphRenderer& gr) {
	osg::Vec2s size;
	
	is >> size;

	gr.setTextureSize(size);

	return true;
}

static bool writeTextureSize(osgDB::OutputStream& os, const osgPango::GlyphRenderer& gr) {
	os << gr.getTextureSize() << std::endl;

	return true;
}

// ---------------------------------------------------------------------------------- MinFilterMode
static bool checkMinFilterMode(const osgPango::GlyphRenderer& gr) {
	return true;

	// return gr.getMinFilterMode() != osg::Texture::LINEAR;
} 

static bool readMinFilterMode(osgDB::InputStream& is, osgPango::GlyphRenderer& gr) {
	DEF_GLENUM(mode);
	
	is >> mode; 
	
	gr.setMinFilterMode(static_cast<osg::Texture::FilterMode>(mode.get()));
	
	return true; 
}

static bool writeMinFilterMode(osgDB::OutputStream& os, const osgPango::GlyphRenderer& gr) {
	os << GLENUM(gr.getMinFilterMode()) << std::endl; 

	return true; 
}

// ------------------------------------------------------------------------------ FontGlyphCacheMap
static bool checkFontGlyphCacheMap(const osgPango::GlyphRenderer& gr) {
	const osgPango::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();

	return true;

	// return fgcm.size() != 0;
} 

static bool readFontGlyphCacheMap(osgDB::InputStream& is, osgPango::GlyphRenderer& gr) {
	osgPango::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();
	
	return true; 
}

static bool writeFontGlyphCacheMap(osgDB::OutputStream& os, const osgPango::GlyphRenderer& gr) {
	const osgPango::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();

	os << osgDB::BEGIN_BRACKET << std::endl;

	for(
		osgPango::GlyphRenderer::FontGlyphCacheMap::const_iterator i = fgcm.begin();
		i != fgcm.end();
		i++
	) {
		os.writeObject(i->second.get());
	}
	
	os << osgDB::END_BRACKET << std::endl;

	return true; 
}

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRenderer,
	0,
	osgPango::GlyphRenderer,
	"osg::Object osgPango::GlyphRenderer"
) {
	ADD_UINT_SERIALIZER(PixelSpacing, 1);
	
	ADD_USER_SERIALIZER(TextureSize);
	ADD_USER_SERIALIZER(MinFilterMode);
	ADD_USER_SERIALIZER(FontGlyphCacheMap);
}

