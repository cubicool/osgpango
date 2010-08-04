// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <algorithm>
#include <osgCairo/Util>
#include <osgPango/GlyphLayerInterface>

namespace osgPango {

GlyphLayerInterfaceOffset::GlyphLayerInterfaceOffset(int xOffset, int yOffset):
_xOffset (xOffset),
_yOffset (yOffset) {
}


GlyphLayerInterfaceBlur::GlyphLayerInterfaceBlur(unsigned int radius, unsigned int deviation):
_radius    (radius),
_deviation (deviation) {
}

cairo_surface_t* GlyphLayerInterfaceBlur::createBlurredSurface(
	cairo_format_t   format,
	cairo_pattern_t* pattern,
	unsigned int     width,
	unsigned int     height
) {
	unsigned int blurSize = _getBlurSize();

	// Create a temporary small surface and then copy that to the bigger one.
	cairo_surface_t* tmp = cairo_image_surface_create(
		format,
		width + (blurSize * 4),
		height + (blurSize * 4)
	);
	
	if(cairo_surface_status(tmp)) return 0;

	cairo_t* tc = cairo_create(tmp);

	if(cairo_status(tc)) return 0;

	cairo_translate(tc, blurSize * 2, blurSize * 2);
	cairo_set_source(tc, pattern);
	cairo_paint(tc);

	cairo_destroy(tc);

	osgCairo::util::gaussianBlur(tmp, _radius, _deviation);
	osgCairo::util::writeToPNG(tmp, "GlyphLayerInterface-tmp.png");

	return tmp;
}

unsigned int GlyphLayerInterfaceBlur::_getBlurSize() const {
	return std::min<double>(
		std::max<double>(_deviation * 3.0, _radius), 
		_radius * 2.0
	);
}

}
