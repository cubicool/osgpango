// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id: Bevel.cpp 127 2010-09-03 01:40:46Z cubicool $

#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerBevel::GlyphLayerBevel(
	double bevelWidth,
	double bevelStep,
	double azimuth,
	double elevation,
	double height,
	double ambient,
	double diffuse
):
GlyphLayer  (),
_bevelWidth (bevelWidth),
_bevelStep  (bevelStep),
_azimuth    (azimuth),
_elevation  (elevation),
_height     (height),
_ambient    (ambient),
_diffuse    (diffuse) {
}

bool GlyphLayerBevel::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	cairo_surface_t* bumpmap = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
	cairo_t*         cr      = cairo_create(bumpmap);

	cairo_set_scaled_font(cr, cairo_get_scaled_font(c));
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_glyph_path(cr, glyph, 1);

	cairo_antialias_t antialias = cairo_get_antialias(cr);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_clip_preserve(cr);
	
	cairo_set_source_rgba(cr, 1.0f, 1.0f, 1.0f, 1.0);
	cairo_fill_preserve(cr);
	
	cairo_set_antialias(cr, antialias);

	for(double l = _bevelWidth; l > 0.0f; l -= _bevelStep) {
		cairo_set_source_rgba(cr, 1.0f, 1.0f, 1.0f, l / _bevelWidth);
		cairo_set_line_width(cr, l);
		cairo_stroke_preserve(cr);
	}

	//for(double l = _bevelWidth; l > 0.0f; l -= _bevelStep) {
	//	cairo_set_source_rgba(cr, 1.0f, 1.0f, 1.0f, 1.0f - (l / _bevelWidth));
	//	cairo_set_line_width(cr, l);
	//	cairo_stroke_preserve(cr);
	//}
	//cairo_fill_preserve(cr);
	
	cairo_destroy(cr);

	cairo_surface_t* lightmap = osgCairo::util::createEmbossedSurface(
		bumpmap,
		_azimuth,
		_elevation,
		_height,
		_ambient,
		_diffuse
	);

	cairo_glyph_path(c, glyph, 1);
	cairo_save(c);
	cairo_clip_preserve(c);
	cairo_set_operator(c, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_surface(c, lightmap, 0, 0);
	cairo_paint(c);
	cairo_restore(c);
	
	// cairo_surface_write_to_png(bumpmap, "bumpmap.png");
	// cairo_surface_write_to_png(lightmap, "lightmap.png");

	cairo_surface_destroy(bumpmap);
	cairo_surface_destroy(lightmap);

	return true;
}

}
