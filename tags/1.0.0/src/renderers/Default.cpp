// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererDefault::GlyphRendererDefault() {
	addLayer(new GlyphLayer());
}

bool GlyphRendererDefault::updateOrCreateState(int pass, osg::Geode* geode) {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag1");
}

}
