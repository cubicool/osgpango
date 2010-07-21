// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <cmath>
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
#ifdef WIN32
	// TODO: why need to setup this format on win ?
	setCairoImageFormat(CAIRO_FORMAT_ARGB32);
#endif
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

GlyphLayerShadowGaussian::GlyphLayerShadowGaussian(unsigned int radius):
_radius(radius) {
}
	
bool GlyphLayerShadowGaussian::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph) return false;
	
	cairo_translate(c, -static_cast<double>(_radius * 2), -static_cast<double>(_radius * 2));

	double add = _radius * 4.0f;
	
	// Create a temporary small surface and then copy that to the bigger one.
	cairo_surface_t* tmp = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width + add, height + add);
	
	if(cairo_surface_status(tmp)) return false;

	cairo_t* tc = cairo_create(tmp);

	if(cairo_status(tc)) return false;

	cairo_scaled_font_t* sf = cairo_get_scaled_font(c);

	cairo_set_scaled_font(tc, sf);
	cairo_set_line_join(tc, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_width(tc, _radius - 0.5f);
	cairo_translate(tc, _radius * 2, _radius * 2);
	cairo_glyph_path(tc, glyph, 1);
	cairo_stroke_preserve(tc);
	cairo_fill(tc);

	osgCairo::util::gaussianBlur(tmp, _radius);
	
	cairo_set_source_surface(c, tmp, 0, 0);
	cairo_paint(c);

	return true;
}
	
osg::Vec4 GlyphLayerShadowGaussian::getExtraGlyphExtents() const {
	// TODO: This is TOO LARGE, but it's safe. We need to find out how to REALLY calucuate the right
	// extents.
	return osg::Vec4(_radius * 2, _radius * 2, _radius * 4, _radius * 4);
}

}
