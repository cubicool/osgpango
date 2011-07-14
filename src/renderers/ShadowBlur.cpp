// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererShadowBlur::GlyphRendererShadowBlur(
	int          xOffset,
	int          yOffset,
	unsigned int radius,
	unsigned int deviation
) {
	if(!deviation) deviation = radius / 2;

	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerShadowBlur(xOffset, yOffset, radius, deviation));
}

bool GlyphRendererShadowBlur::updateOrCreateState(int pass, osg::Geode* geode) const {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag2");
}

}
