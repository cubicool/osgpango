#include <cmath>
#include <osgCairo/Util>
#include <osgPango/GlyphLayer>

namespace osgPango {

bool GlyphLayer::render(
	osgCairo::Surface*      si,
	const osgCairo::Glyph&  g,
	unsigned int            w,
	unsigned int            h
) {
	if(!si) return false;
	
	si->glyphPath(g);
	si->fill();

	return true;
}

osg::Vec4 GlyphLayer::getExtraGlyphExtents() const {
	return osg::Vec4(0.0, 0.0, 0.0, 0.0);
}

GlyphLayerOutline::GlyphLayerOutline(unsigned int outline):
_outline(outline) {
}

bool GlyphLayerOutline::render(
	osgCairo::Surface*      si,
	const osgCairo::Glyph&  g,
	unsigned int            w,
	unsigned int            h
) {
	if(!si) return false;

	si->setLineJoin(CAIRO_LINE_JOIN_ROUND);
	si->setLineWidth((_outline * 2) - 0.5f);
	si->setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
	si->translate(_outline, _outline);
	si->glyphPath(g);
	si->strokePreserve();
	si->fill();
	si->setOperator(CAIRO_OPERATOR_CLEAR);
	
	GlyphLayer::render(si, g, w, h);

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
	osgCairo::Surface* si,
	const osgCairo::Glyph& g,
	unsigned int w,
	unsigned int h
) {
	if(!si) return false;

	si->save();

	if(_xOffset > 0) si->translate(_xOffset, 0.0f);

	if(_yOffset > 0) si->translate(0.0f, _yOffset);

	GlyphLayer::render(si, g, w, h);

	si->restore();
	si->setOperator(CAIRO_OPERATOR_CLEAR);

	if(_xOffset < 0) si->translate(std::fabs(_xOffset), 0.0f);

	if(_yOffset < 0) si->translate(0.0f, std::fabs(_yOffset));

	GlyphLayer::render(si, g, w, h);

	return true;
}

osg::Vec4 GlyphLayerShadowOffset::getExtraGlyphExtents() const {
	return osg::Vec4(0.0f, 0.0f, std::fabs(_xOffset), std::fabs(_yOffset));
}

GlyphLayerShadowGaussian::GlyphLayerShadowGaussian(unsigned int radius):
_radius(radius) {
}
	
bool GlyphLayerShadowGaussian::render(
	osgCairo::Surface* si,
	const osgCairo::Glyph& g,
	unsigned int w,
	unsigned int h
) {
	if(!si) return false;

	double add = _radius * 4.0f;
	
	// Create a temporary small surface and then copy that to the bigger one.
	osgCairo::Surface tmp(w + add, h + add, CAIRO_FORMAT_ARGB32);

	if(!tmp.createContext()) return false;
 
	osgCairo::CairoScaledFont* sf = si->getScaledFont();

	tmp.setScaledFont(sf);
	tmp.setLineJoin(CAIRO_LINE_JOIN_ROUND);
	tmp.setLineWidth(_radius - 0.5f);
	tmp.setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
	tmp.translate(_radius * 2, _radius * 2);
	tmp.glyphPath(g);
	tmp.strokePreserve();
	tmp.fill();

	osgCairo::util::gaussianBlur(&tmp, _radius);
	
	tmp.setOperator(CAIRO_OPERATOR_CLEAR);
	
	GlyphLayer::render(&tmp, g, w, h);

	si->setSourceSurface(&tmp, 0, 0);
	si->paint();

	return true;
}
	
osg::Vec4 GlyphLayerShadowGaussian::getExtraGlyphExtents() const {
	return osg::Vec4(_radius * 2, _radius * 2, _radius * 4, _radius * 4);
}

}
