// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgDB/WriteFile>
#include <osgPango/Context>
#include <osgPango/String>

// This will be our special class that will act like text but only cache stuff for us.
// It derives from osgPango::ContextDrawable so that it the Context can properly pass
// PangoGlyphString objects to it...
class CacheContextDrawable: public osgPango::ContextDrawable {
public:
	CacheContextDrawable(osgPango::GlyphRenderer* renderer):
	_gr(renderer) {
	}

	virtual void drawGlyphs(PangoFont* font, PangoGlyphString* glyphs, int x, int y) {
		if(!_gr) return;

		osgPango::GlyphCache* gc = _gr->getOrCreateGlyphCache(font);

		if(!gc) return;

		for(int i = 0; i < glyphs->num_glyphs; i++) {
			PangoGlyphInfo* gi = glyphs->glyphs + i;

			if((gi->glyph & PANGO_GLYPH_UNKNOWN_FLAG)) continue;

			const osgPango::CachedGlyph* cg = gc->getCachedGlyph(gi->glyph);

			if(!cg) cg = gc->createCachedGlyph(font, gi);

			if(!cg) continue;
		}
	}

	void cacheString(
		osgPango::String::Encoding encoding,
		const std::string&         str,
		const std::string&         descr = ""
	) {
		if(!str.size()) return;

		osgPango::String text;

		PangoLayout* layout = pango_layout_new(
			osgPango::Context::instance().getPangoContext()
		);

		text.set(str, encoding);

		std::string utf8 = text.createUTF8EncodedString();

		if(descr.empty()) pango_layout_set_markup(layout, utf8.c_str(), -1);

		else {
			pango_layout_set_font_description(
				layout,
				pango_font_description_from_string(descr.c_str())
			);

			pango_layout_set_text(layout, utf8.c_str(), -1);
		}

		osgPango::Context::instance().drawLayout(this, layout, 0, 0);
	}
	
private:
	osg::ref_ptr<osgPango::GlyphRenderer> _gr;
};

int main(int argc, char** argv) {
	osgPango::Context::instance().init();

	osgPango::GlyphRenderer* renderer = new osgPango::GlyphRendererDefault();

	renderer->setName("renderer");
	renderer->setTextureSize(osg::Vec2s(512, 256));

	CacheContextDrawable cache(renderer);

	cache.cacheString(
		OSGPANGO_ENCODING,
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"1234567890",
		"Sans 100px"
	);

	if(!osgDB::writeObjectFile(*renderer, "renderer.osgt")) {
		OSG_NOTICE << "Failed to create the cache." << std::endl;

		return 1;
	}

	return 0;
}

