// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <sstream>
#include <osgPango/Context>

namespace osgPango {

Context Context::_context;

Context& Context::instance() {
	return _context;
}

bool Context::init(
	unsigned int                 dpi,
	osgCairo::CairoAntialias     aa,
	osgCairo::CairoHintStyle     hs,
	osgCairo::CairoSubpixelOrder spo
) {
	if(_pfMap && _pContext) return false;

	if(!_pfMap) _pfMap = pango_cairo_font_map_new();
	
	if(_pfMap && !_pContext) _pContext = pango_cairo_font_map_create_context(
		PANGO_CAIRO_FONT_MAP(_pfMap)
	);

	if(!_pfMap || !_pContext) return false;

	// TODO: Fix this...
	cairo_font_options_t* options = cairo_font_options_create();

	cairo_font_options_set_antialias(options, aa);
	cairo_font_options_set_hint_style(options, hs);
	cairo_font_options_set_subpixel_order(options, spo);

	pango_cairo_context_set_font_options(_pContext, options);
	pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(_pfMap), dpi);

	cairo_font_options_destroy(options);

	return true;
}

unsigned int Context::getFontList(FontList& fl, bool faces) {
	if(!_pContext) return false;

	PangoFontFamily** pff = 0;
	int               num = 0;

	pango_context_list_families(_pContext, &pff, &num);

	if(!num || !pff) return 0;

	for(int i = 0; i < num; i++) {
		std::ostringstream ss;

		ss << pango_font_family_get_name(pff[i]);
		
		if(faces) {
			ss << " (";
		
			PangoFontFace** faces    = 0;
			int             numFaces = 0;

			pango_font_family_list_faces(pff[i], &faces, &numFaces);

			for(int j = 0; j < numFaces; j++) {
				ss << pango_font_face_get_face_name(faces[j]);

				if(j + 1 != numFaces) ss << ", ";
			}

			g_free(faces);

			ss << ")";
		}

		fl.push_back(ss.str());
	}

	g_free(pff);

	return static_cast<unsigned int>(num);
}

bool Context::addGlyphRenderer(const std::string& key, GlyphRenderer* gr) {
	_grMap[key] = gr;

	return true;
}

GlyphCache* Context::getGlyphCache(PangoFont* font, const std::string& renderer) {
	GlyphCacheFontMapKey key(font, renderer);

	GlyphCache* gc = _gcfMap[key].get();

	if(!gc) {
		gc = new GlyphCache();

		_gcfMap[key] = gc;
	}

	return gc;
}

Context::Context():
_pfMap    (0),
_pContext (0) {
}

}
