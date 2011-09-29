// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <cmath>
#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererShadow::GlyphRendererShadow(int offsetX, int offsetY) {
	addLayer(new GlyphLayer());
	addLayer(new GlyphLayerShadow(offsetX, offsetY));
}

GlyphRendererShadow::GlyphRendererShadow(
	const GlyphRendererShadow& grs,
	const osg::CopyOp&         copyOp
):
GlyphRenderer(grs, copyOp) {
}

bool GlyphRendererShadow::updateOrCreateState(int pass, osg::Geode* geode) const {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag2");
}

}
