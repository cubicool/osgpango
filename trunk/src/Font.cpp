// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <iostream>
#include <algorithm>
#include <sstream>
#include <osgPango/Font>

namespace osgPango {

PangoFontMap* Font::_map     = 0;
PangoContext* Font::_context = 0;
Font::FontMap Font::_fonts;

Font* Font::create(const std::string& descr, unsigned int w, unsigned int h) {
	std::string s = descr;

	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	
	FontMap::iterator f = _fonts.find(s);

	if(!s.size()) s = OSGPANGO_DEFAULT_FONT;

	if(f != _fonts.end()) return f->second.get();

	// At this point, we need to create a new Font so be sure and check for our
	// "Shadow" or "Outline" keywords.
	Font* font = 0;

	std::string::size_type pos1 = s.rfind(" ");

	if(pos1 != std::string::npos) {
		std::string last(s, pos1 + 1);
	
		std::string::size_type pos2 = last.rfind("=");

		// 8 characters is the minimum amount to specify an additional option...
		if(last.size() >= 8 && pos2 != std::string::npos) {
			std::string arg(last, 0, pos2);
			std::string val(last, pos2 + 1);

			font = new Font(std::string(s, 0, pos1), w, h);

			if(arg == "shadow") {
				font->_cache->_gcm          = GlyphCache::GCM_SHADOW;
				font->_cache->_shadowOffset = std::atoi(val.c_str());
			}

			else if(arg == "outline") {
				font->_cache->_gcm         = GlyphCache::GCM_OUTLINE;
				font->_cache->_outlineSize = std::atof(val.c_str());
			}

			else osg::notify(osg::WARN)
				<< "Bad argument in font: " << arg
				<< "." << std::endl
			;
		}
	}

	if(!font) font = new Font(s, w, h);

	_fonts[s] = font;

	return font;
}

Font::Font(const std::string& descr, unsigned int w, unsigned int h) {
	_descr = pango_font_description_from_string(descr.c_str());
	_cache = new GlyphCache(w, h);
}

Font::~Font() {
	pango_font_description_free(_descr);
}

Font* Font::getFont(const std::string& descr) {
	std::string s = descr;

	std::transform(s.begin(), s.end(), s.begin(), ::tolower);

	return _fonts[s].get();
}

unsigned int Font::getFontList(FontList& fl) {
	PangoFontFamily** pff = 0;
	int               num = 0;

	pango_context_list_families(_context, &pff, &num);

	if(!num || !pff) return 0;

	for(int i = 0; i < num; i++) {
		std::ostringstream ss;

		ss << pango_font_family_get_name(pff[i]) << " (";
		
		PangoFontFace** faces    = 0;
		int             numFaces = 0;

		pango_font_family_list_faces(pff[i], &faces, &numFaces);

		for(int j = 0; j < numFaces; j++) {
			ss << pango_font_face_get_face_name(faces[j]);

			if(j + 1 != numFaces) ss << ", ";
		}

		g_free(faces);

		ss << ")";

		fl.push_back(ss.str());
	}

	g_free(pff);

	return static_cast<unsigned int>(num);
}

bool Font::init(
	unsigned int                 dpi,
	osgCairo::CairoAntialias     aa,
	osgCairo::CairoHintStyle     hs,
	osgCairo::CairoSubpixelOrder spo
) {
	if(_map && _context) return false;

	if(!_map) _map = pango_cairo_font_map_new();
	
	if(_map && !_context) _context = pango_cairo_font_map_create_context(
		PANGO_CAIRO_FONT_MAP(_map)
	);

	if(!_map || !_context) return false;

	// TODO: Fix this...
	cairo_font_options_t* options = cairo_font_options_create();

	cairo_font_options_set_antialias(options, aa);
	cairo_font_options_set_hint_style(options, hs);
	cairo_font_options_set_subpixel_order(options, spo);

	pango_cairo_context_set_font_options(_context, options);
	pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(_map), dpi);

	cairo_font_options_destroy(options);

	return true;
}

}
