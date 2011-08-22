// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerDistanceField::GlyphLayerDistanceField(
	unsigned int scanSize,
	unsigned int blockSize,
	double       padding
):
_scanSize  (scanSize),
_blockSize (blockSize),
_padding   (padding) {
}

bool GlyphLayerDistanceField::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	unsigned int w = (width * _blockSize) + (_padding * _blockSize * 2);
	unsigned int h = (height * _blockSize) + (_padding * _blockSize * 2);

	// It distanceField needs to be square.
	if(w > h) h = w;
	
	else if(h > w) w = h;

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_A8, w, h);
	cairo_t*         cr      = cairo_create(surface);

	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_set_scaled_font(cr, cairo_get_scaled_font(c));
	cairo_scale(cr, _blockSize, _blockSize);
	cairo_translate(cr, _padding, _padding);
	cairo_glyph_path(cr, glyph, 1);
	cairo_fill(cr);

	cairo_surface_t* distanceField = osgCairo::createDistanceField(
		surface,
		_scanSize,
		_blockSize
	);

	// osgCairo::util::writeToPNG(distanceField, "distanceField.png");
	// osgCairo::util::writeToPNG(surface, "surface.png");

	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	if(!distanceField) {
		OSG_WARN << "Unable to call osgCairo::util::createDistanceField." << std::endl;

		return false;
	}

	cairo_status_t err = cairo_surface_status(distanceField);

	if(cairo_surface_status(distanceField)) {
		OSG_WARN
			<< "Unable to call osgCairo::util::createDistanceField; error was: " 
			<< cairo_status_to_string(err) << "."
			<< std::endl
		;

		return false;
	}

	cairo_set_source_surface(c, distanceField, -_padding, -_padding);
	cairo_paint(c);

	/*
	cairo_set_line_width(c, 1.0f);
	cairo_set_source_rgba(c, 1.0f, 1.0f, 1.0f, 1.0f);
	cairo_rectangle(c, -_padding + 0.5f, -_padding + 0.5f, (width + _padding * 2) - 0.5f, (height + _padding * 2) - 0.5f);
	cairo_stroke(c);
	*/

	cairo_surface_destroy(distanceField);

	return true;
}

osg::Vec4 GlyphLayerDistanceField::getExtraGlyphExtents() const {
	return osg::Vec4(_padding, _padding, _padding * 2.0f, _padding * 2.0f);
}

}
