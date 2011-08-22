// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <algorithm>
#include <osg/Notify>
#include <osgCairo/Util>
#include <osgPango/GlyphLayerInterface>

namespace osgPango {

GlyphLayerInterfaceOffset::GlyphLayerInterfaceOffset(int xOffset, int yOffset):
_xOffset (xOffset),
_yOffset (yOffset) {
}

GlyphLayerInterfaceBlur::GlyphLayerInterfaceBlur(double radius, double deviation):
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

	if(_deviation > 0.0f) osgCairo::gaussianBlur(tmp, _radius, _deviation);

	return tmp;
}

unsigned int GlyphLayerInterfaceBlur::_getBlurSize() const {
	return std::min(std::max(_deviation * 3, _radius), _radius * 2);
}

GlyphLayerInterfaceEmboss::GlyphLayerInterfaceEmboss(double azimuth, double elevation):
_azimuth   (azimuth),
_elevation (elevation) {
}

cairo_surface_t* GlyphLayerInterfaceEmboss::createEmbossedSurface(
	cairo_surface_t* surface,
	unsigned int     width,
	unsigned int     height
) {
	osg::notify(osg::WARN) << "Not yet implemented." << std::endl;

	return 0;
}

}
