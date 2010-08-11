// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <cstdlib>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerShadowInset::GlyphLayerShadowInset(
	unsigned int radius, 
	unsigned int deviation
):
GlyphLayerInterfaceOffset (0, 0),
GlyphLayerInterfaceBlur   (radius, deviation) {
}
	
bool GlyphLayerShadowInset::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	// METHOD 1 ===============================================================================
	cairo_push_group(c);
	cairo_translate(c, getOffsetX(), getOffsetY());
	cairo_set_line_join(c, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(c, static_cast<double>(_radius) - 0.5f);
	cairo_glyph_path(c, glyph, 1);
	cairo_stroke(c);
	
	cairo_pattern_t* pattern = cairo_pop_group(c);
	cairo_surface_t* tmp     = createBlurredSurface(CAIRO_FORMAT_A8, pattern, width, height);

	cairo_set_source_surface(
		c,
		tmp,
		-static_cast<double>(_getBlurSize()) * 2,
		-static_cast<double>(_getBlurSize()) * 2
	);

	cairo_glyph_path(c, glyph, 1);
	cairo_clip(c);
	cairo_paint(c);
	cairo_surface_destroy(tmp);
	cairo_pattern_destroy(pattern);


#if 0
	// METHOD 2 ===============================================================================
	cairo_set_line_join(c, CAIRO_LINE_JOIN_ROUND);
	cairo_glyph_path(c, glyph, 1);
	cairo_clip_preserve(c);
	cairo_set_source_rgba(c, 1.0f, 1.0f, 1.0f, 1.0f / _radius);
	
	for(unsigned int r = _radius; r > 0; r--) {
		cairo_set_line_width(c, r * 2);
		cairo_stroke_preserve(c);
	}
#endif

	return true;
}

}
