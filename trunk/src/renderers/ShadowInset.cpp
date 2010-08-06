// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererShadowInset::GlyphRendererShadowInset(unsigned int radius) {
	addLayer(new GlyphLayerShadowInset(radius, radius * 0.5f));
	addLayer(new GlyphLayer());
}

bool GlyphRendererShadowInset::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag2");
}

}
