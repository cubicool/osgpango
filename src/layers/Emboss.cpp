// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id: Emboss.cpp 127 2010-09-03 01:40:46Z cubicool $

#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerEmboss::GlyphLayerEmboss(double azimuth, double elevation):
_azimuth   (azimuth),
_elevation (elevation) {
}
	
bool GlyphLayerEmboss::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
	cairo_t*         cr      = cairo_create(surface);

	cairo_glyph_path(cr, glyph, 1);
	cairo_fill(cr);

	/*
	cairo_surface_t* emboss = osgCairo::util::createEmbossedSurface(surface, _azimuth, _elevation);

	if(!emboss) return false;
	*/

	// osgCairo::util::writeToPNG(surface, "emboss.png");

	cairo_set_source_surface(c, surface, 0, 0);
	cairo_paint(c);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	// cairo_surface_destroy(emboss);

	return true;
}

}
