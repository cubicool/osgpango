// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <cmath>
#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererShadow::GlyphRendererShadow(int offsetX, int offsetY) {
	unsigned int xt = 0;
	unsigned int yt = 0;

	if(offsetX < 0) xt = std::abs(static_cast<double>(offsetX));

	if(offsetY < 0) yt = std::abs(static_cast<double>(offsetY));
	
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerShadow(offsetX, offsetY));
}

bool GlyphRendererShadow::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag2");
}

}
