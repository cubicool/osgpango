// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererShadowBlur::GlyphRendererShadowBlur(unsigned int radius) {
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerShadowBlur(0.0f, 0.0f, radius, radius * 0.5f));
}

bool GlyphRendererShadowBlur::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag2");
}

}
