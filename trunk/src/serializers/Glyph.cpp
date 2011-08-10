// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgPango/Glyph>

// ----------------------------------------------------------------------------------------- Layers
static bool checkLayers(const osgPango::GlyphCache& gc) {
	const osgPango::GlyphCache::Layers& layers = gc.getLayers();

	return layers.size() != 0;
} 

static bool readLayers(osgDB::InputStream& is, osgPango::GlyphCache& gc) {
	osgPango::GlyphCache::Layers& layers = gc.getLayers();

	unsigned int layerSize = is.readSize();
	unsigned int imgSize   = is.readSize();

	is >> osgDB::BEGIN_BRACKET;

	for(unsigned int l = 0; l < layerSize; l++) {
		unsigned int layerNum;

		is >> osgDB::PROPERTY("Layer") >> layerNum >> osgDB::BEGIN_BRACKET;

		for(unsigned int i = 0; i < imgSize; i++) {
			unsigned int imgNum;

			is >> osgDB::PROPERTY("Image") >> imgNum >> osgDB::BEGIN_BRACKET;

			osg::Texture* texture = dynamic_cast<osg::Texture*>(is.readObject());

			if(texture)

			is >> osgDB::END_BRACKET;
		}

		is >> osgDB::END_BRACKET;
	}

	is >> osgDB::END_BRACKET;

	return true; 
}

static bool writeLayers(osgDB::OutputStream& os, const osgPango::GlyphCache& gc) {
	const osgPango::GlyphCache::Layers& layers = gc.getLayers();

	os.writeSize(layers.size());
	os.writeSize(layers[0].size());

	os << osgDB::BEGIN_BRACKET << std::endl;

	// We have one Layer per "effect", essentially.
	for(unsigned int l = 0; l < layers.size(); l++) {
		os << "Layer" << l << osgDB::BEGIN_BRACKET << std::endl;

		// Now we iterate over our Images, which we can have a variable number of
		// depending on how numerous and how large the glyphs are.
		for(unsigned int i = 0; i < layers[l].size(); i++) {
			os << "Image" << i << osgDB::BEGIN_BRACKET << std::endl;

			osg::Image*   image   = layers[l][i].first.get();
			osg::Texture* texture = layers[l][i].second.get();

			// TODO: SPEW ERRORS...
			if(!image->getFileName().size()) return false;

			os.setWriteImageHint(osgDB::OutputStream::WRITE_EXTERNAL_FILE);
			os.writeObject(texture);

			os << osgDB::END_BRACKET << std::endl;
		}

		os << osgDB::END_BRACKET << std::endl;
	}
	
	os << osgDB::END_BRACKET << std::endl;

	return true; 
}

// --------------------------------------------------------------------------------------- GlyphMap
static bool checkGlyphMap(const osgPango::GlyphCache& gc) {
	const osgPango::GlyphCache::GlyphMap& gmap = gc.getGlyphMap();

	return gmap.size() != 0;
} 

static bool readGlyphMap(osgDB::InputStream& is, osgPango::GlyphCache& gc) {
	osgPango::GlyphCache::GlyphMap& gmap = gc.getGlyphMap();
	
	return true; 
}

static bool writeGlyphMap(osgDB::OutputStream& os, const osgPango::GlyphCache& gc) {
	const osgPango::GlyphCache::GlyphMap& gmap = gc.getGlyphMap();

	os << osgDB::BEGIN_BRACKET << std::endl;

	for(
		osgPango::GlyphCache::GlyphMap::const_iterator i = gmap.begin();
		i != gmap.end();
		i++
	) {
		os << i->first << osgDB::BEGIN_BRACKET << std::endl;

		const osgPango::CachedGlyph& cg = i->second;

		os << "img" << cg.img << std::endl;
		os << "origin" << cg.origin << std::endl;
		os << "size" << cg.size << std::endl;
		os << "bl" << cg.bl << std::endl;
		os << "br" << cg.br << std::endl;
		os << "ur" << cg.ur << std::endl;
		os << "ul" << cg.ul << std::endl;

		os << osgDB::END_BRACKET << std::endl;
	}

	os << osgDB::END_BRACKET << std::endl;

	return true; 
}

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphCache,
	new osgPango::GlyphCache(),
	osgPango::GlyphCache,
	"osg::Object osgPango::GlyphCache"
) {
	ADD_UINT_SERIALIZER(Hash, 0);

	ADD_VEC3F_SERIALIZER(XYH, osg::Vec3f());
	
	ADD_USER_SERIALIZER(Layers);
	ADD_USER_SERIALIZER(GlyphMap);
}

