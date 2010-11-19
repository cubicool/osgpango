// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id: Emboss.cpp 121 2010-08-13 22:05:01Z cubicool $

#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererEmboss::GlyphRendererEmboss(double azimuth, double elevation) {
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerEmboss(azimuth, elevation));
}

bool GlyphRendererEmboss::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag2");
}

}
