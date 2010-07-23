// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <cmath>
#include <cstdlib>
#include <limits>
#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayer::GlyphLayer(cairo_format_t format):
_imageFormat(format) {
}

bool GlyphLayer::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	cairo_glyph_path(c, glyph, 1);
	cairo_fill(c);

	return true;
}

osg::Vec4 GlyphLayer::getExtraGlyphExtents() const {
	return osg::Vec4(0.0, 0.0, 0.0, 0.0);
}

GlyphLayerOutline::GlyphLayerOutline(unsigned int outline):
_outline(outline) {
}

bool GlyphLayerOutline::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;
	
	cairo_set_line_join(c, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(c, (_outline * 2) - 0.5f);
	cairo_glyph_path(c, glyph, 1);
	cairo_stroke_preserve(c);
	cairo_fill(c);
	
	return true;
}

osg::Vec4 GlyphLayerOutline::getExtraGlyphExtents() const {
	return osg::Vec4(_outline, _outline, _outline * 2, _outline * 2);
}

GlyphLayerShadowOffset::GlyphLayerShadowOffset(int offsetX, int offsetY): 
_xOffset(offsetX),
_yOffset(offsetY) {
}

bool GlyphLayerShadowOffset::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;

	cairo_save(c);

	if(_xOffset > 0) cairo_translate(c, _xOffset, 0.0f);

	if(_yOffset > 0) cairo_translate(c, 0.0f, _yOffset);

	GlyphLayer::render(c, glyph, width, height);

	cairo_restore(c);
	cairo_set_operator(c, CAIRO_OPERATOR_CLEAR);

	if(_xOffset < 0) cairo_translate(c, std::abs(static_cast<double>(_xOffset)), 0.0f);

	if(_yOffset < 0) cairo_translate(c, 0.0f, std::abs(static_cast<double>(_yOffset)));

	GlyphLayer::render(c, glyph, width, height);

	return true;
}

osg::Vec4 GlyphLayerShadowOffset::getExtraGlyphExtents() const {
	return osg::Vec4(
		0.0f,
		0.0f,
		std::abs(static_cast<double>(_xOffset)),
		std::abs(static_cast<double>(_yOffset))
	);
}

GlyphLayerShadowGaussian::GlyphLayerShadowGaussian(int offsetX, 
                                                   int offsetY, 
                                                   unsigned int radius, 
                                                   unsigned int deviation):
GlyphLayerShadowOffset(offsetX, offsetY),
_radius(radius),
_deviation(deviation) {
}
	
bool GlyphLayerShadowGaussian::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;
	
	unsigned int blurSize = _getBlurSize();

	cairo_translate(c, -static_cast<double>(blurSize) + getOffsetX(), -static_cast<double>(blurSize) + getOffsetY());

	double add = blurSize * 4.0f;
	
	// Create a temporary small surface and then copy that to the bigger one.
	cairo_surface_t* tmp = cairo_image_surface_create(CAIRO_FORMAT_A8, width + add, height + add);
	
	if(cairo_surface_status(tmp)) return false;

	cairo_t* tc = cairo_create(tmp);

	if(cairo_status(tc)) return false;

	cairo_scaled_font_t* sf = cairo_get_scaled_font(c);

	cairo_set_scaled_font(tc, sf);
	cairo_set_line_join(tc, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(tc, static_cast<double>(_radius) - 0.5f);
	cairo_translate(tc, blurSize * 2, blurSize * 2);
	cairo_glyph_path(tc, glyph, 1);
	cairo_stroke_preserve(tc);
	cairo_fill(tc);

	if(_deviation > std::numeric_limits<double>::epsilon())
		osgCairo::util::gaussianBlur(tmp, _radius, _deviation);
	
	cairo_set_source_surface(c, tmp, -static_cast<double>(blurSize), -static_cast<double>(blurSize));
	cairo_paint(c);

	return true;
}
	
osg::Vec4 GlyphLayerShadowGaussian::getExtraGlyphExtents() const {
	double offset   = std::max<double>(std::abs(getOffsetX()), std::abs(getOffsetY()));
	double blursize = _getBlurSize();
	
	return osg::Vec4(
		blursize + offset, 
		blursize + offset, 
		(blursize + offset) * 2.0, 
		(blursize + offset) * 2.0
	);
}

unsigned int GlyphLayerShadowGaussian::_getBlurSize() const {
	return std::min<double>(
		std::max<double>(_deviation * 3.0, _radius), 
		_radius * 2.0
	);
}
}
