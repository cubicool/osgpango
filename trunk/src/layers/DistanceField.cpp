// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerDistanceField::GlyphLayerDistanceField() {
}

bool GlyphLayerDistanceField::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_A8, 4096, 4096);
	cairo_t*         cr      = cairo_create(surface);

	// cairo_translate(cr, 0.0f, 0.0f);
	// double aspectRatio = static_cast<double>(width) / static_cast<double>(height);

	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_scaled_font(cr, cairo_get_scaled_font(c));
	cairo_scale(cr, 32, 32);
	cairo_glyph_path(cr, glyph, 1);
	cairo_fill(cr);

	cairo_surface_t* distanceField = osgCairo::util::createDistanceField(surface, 200, 32);

	// osgCairo::util::writeToPNG(surface, "surface.png");
	// osgCairo::util::writeToPNG(distanceField, "distanceField.png");

	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	cairo_set_source_surface(c, distanceField, 0, 0);
	cairo_paint(c);

	cairo_surface_destroy(distanceField);

	return true;
}

osg::Vec4 GlyphLayerDistanceField::getExtraGlyphExtents() const {
	return osg::Vec4(10.0f, 10.0f, 20.0f, 20.0f);
}

}
