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
		is >> osgDB::PROPERTY("Layer") >> osgDB::BEGIN_BRACKET;

		layers.push_back(osgPango::GlyphCache::Images());

		for(unsigned int i = 0; i < imgSize; i++) {
			is >> osgDB::PROPERTY("CairoImage") >> osgDB::BEGIN_BRACKET;

			osgPango::GlyphCache::CairoTexture ct;

			osg::Texture* texture = dynamic_cast<osg::Texture*>(is.readObject());

			if(!texture) {
				// TODO: More verbose...
				OSG_WARN << "Couldn't load Image " << i << std::endl;

				return false;
			}

			osg::Image* image = texture->getImage(0);
			
			if(!image) {
				OSG_WARN << "No image..." << std::endl;

				return false;
			}

			/*
			unsigned char* data = osgCairo::createNewImageDataAsCairoFormat(
				image,
				CAIRO_FORMAT_A8
			);

			if(!data) OSG_WARN << "No data..." << std::endl;
			*/

			osgCairo::Image* newImage = new osgCairo::Image(
				image->s(),
				image->t(),
				CAIRO_FORMAT_A8,
				static_cast<const unsigned char*>(image->getDataPointer())
			);

			// *
			cairo_t* c = newImage->createContext();

			cairo_arc(c, image->s() / 2.0f, image->t() / 2.0f, 100.0f, 0.0f, 2.0f * 3.14159f);
			cairo_fill(c);
			cairo_destroy(c);
			// *

			newImage->setFileName(image->getFileName());

			// Now the previous image will be deleted, but our data has
			// been copied so it's all good.
			texture->setImage(0, newImage);

			ct.first  = newImage;
			ct.second = texture;

			layers[l].push_back(ct);

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
		os << "Layer" << osgDB::BEGIN_BRACKET << std::endl;

		// Now we iterate over our Images, which we can have a variable number of
		// depending on how numerous and how large the glyphs are.
		for(unsigned int i = 0; i < layers[l].size(); i++) {
			os << "CairoImage" << osgDB::BEGIN_BRACKET << std::endl;

			osg::Image*   image   = layers[l][i].first.get();
			osg::Texture* texture = layers[l][i].second.get();

			// TODO: SPEW ERRORS...
			if(!image->getFileName().size()) {
				OSG_WARN << "No filename!" << std::endl;

				return false;
			}

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

	unsigned int numGlyphs = is.readSize();

	is >> osgDB::BEGIN_BRACKET;

	for(unsigned int g = 0; g < numGlyphs; g++) {
		unsigned int glyphID;

		is >> glyphID >> osgDB::BEGIN_BRACKET;

		osgPango::CachedGlyph cg;

		is >> osgDB::PROPERTY("img") >> cg.img;
		is >> osgDB::PROPERTY("origin") >> cg.origin;
		is >> osgDB::PROPERTY("size") >> cg.size;
		is >> osgDB::PROPERTY("bl") >> cg.bl;
		is >> osgDB::PROPERTY("br") >> cg.br;
		is >> osgDB::PROPERTY("ur") >> cg.ur;
		is >> osgDB::PROPERTY("ul") >> cg.ul;

		is >> osgDB::END_BRACKET;

		gmap[glyphID] = cg;
	}
	
	is >> osgDB::END_BRACKET;

	return true; 
}

static bool writeGlyphMap(osgDB::OutputStream& os, const osgPango::GlyphCache& gc) {
	const osgPango::GlyphCache::GlyphMap& gmap = gc.getGlyphMap();

	os.writeSize(gmap.size());

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

	ADD_STRING_SERIALIZER(Description, "");

	ADD_VEC3F_SERIALIZER(XYH, osg::Vec3f());
	
	ADD_USER_SERIALIZER(Layers);
	ADD_USER_SERIALIZER(GlyphMap);
}

