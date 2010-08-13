// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererShadowInset::GlyphRendererShadowInset(
	int          xOffset,
	int          yOffset,
	unsigned int radius,
	unsigned int deviation
) {
	if(!deviation) deviation = radius / 2;

	addLayer(new GlyphLayerShadowInset(xOffset, yOffset, radius, deviation));
	addLayer(new GlyphLayer());
}

bool GlyphRendererShadowInset::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag2");
}

}
