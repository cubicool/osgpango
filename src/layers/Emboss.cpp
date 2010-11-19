// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id: Emboss.cpp 127 2010-09-03 01:40:46Z cubicool $

#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerEmboss::GlyphLayerEmboss(double azimuth, double elevation):
_azimuth   (azimuth),
_elevation (elevation) {
}

osg::Vec4 GlyphLayerEmboss::getExtraGlyphExtents() const {
	return osg::Vec4(3.0f, 3.0f, 6.0f, 6.0f);
}

bool GlyphLayerEmboss::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_A8, width + 6, height + 6);
	cairo_t*         cr      = cairo_create(surface);

	cairo_set_scaled_font(cr, cairo_get_scaled_font(c));
	cairo_translate(cr, 3.0f, 3.0f);
	cairo_glyph_path(cr, glyph, 1);
	cairo_fill(cr);

	cairo_surface_t* emboss = osgCairo::util::createEmbossedSurface(
		surface,
		_azimuth,
		_elevation
	);

	if(!emboss) return false;

	// osgCairo::util::writeToPNG(emboss, "emboss.png");

	cairo_set_source_surface(c, emboss, -3.0f, -3.0f);
	cairo_paint(c);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	cairo_surface_destroy(emboss);

	return true;
}

}
