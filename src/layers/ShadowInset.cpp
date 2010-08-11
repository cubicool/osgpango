// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <cstdlib>
#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerShadowInset::GlyphLayerShadowInset(
	unsigned int radius, 
	unsigned int deviation
):
// TODO: This is temporary, just for testing! (THE BUILTIN OFFSET)
GlyphLayerInterfaceOffset (-3, -3),
GlyphLayerInterfaceBlur   (radius, deviation) {
}
	
bool GlyphLayerShadowInset::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;
	
	/*
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
	*/

	/*
	// METHOD 2 ===============================================================================
	cairo_set_line_join(c, CAIRO_LINE_JOIN_ROUND);
	cairo_glyph_path(c, glyph, 1);
	cairo_clip_preserve(c);
	cairo_set_source_rgba(c, 1.0f, 1.0f, 1.0f, 1.0f / _radius);
	
	for(unsigned int r = _radius; r > 0; r--) {
		cairo_set_line_width(c, r * 2);
		cairo_stroke_preserve(c);
	}
	*/

	// METHOD 3 ===============================================================================
	// This method isn't quite there YET, but we're getting close.
	cairo_push_group(c);
	cairo_glyph_path(c, glyph, 1);
	cairo_set_line_width(c, _radius);
	cairo_stroke(c);

	cairo_pattern_t* blur = osgCairo::util::displacedBlur(c, cairo_pop_group(c), _radius);

	cairo_push_group(c);
	cairo_glyph_path(c, glyph, 1);
	cairo_fill(c);

	cairo_pattern_t* mask = cairo_pop_group(c);

	cairo_matrix_t matrix;

	cairo_get_matrix(c, &matrix);

	cairo_matrix_translate(
		&matrix,
		(_radius / 2.0f) - getOffsetX(),
		(_radius / 2.0f) - getOffsetY()
	);
	
	cairo_pattern_set_matrix(blur, &matrix);

	cairo_set_source(c, blur);
	cairo_mask(c, mask);
	
	cairo_pattern_destroy(blur);
	cairo_pattern_destroy(mask);

	return true;
}

}
