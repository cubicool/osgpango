// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgPango/GlyphRenderer>

namespace osgPango {

GlyphRendererDefault::GlyphRendererDefault() {
	addLayer(new GlyphLayer());
}

GlyphRendererDefault::GlyphRendererDefault(
	const GlyphRendererDefault& gr,
	const osg::CopyOp&          copyOp
):
GlyphRenderer(gr, copyOp) {
}

bool GlyphRendererDefault::updateOrCreateState(int pass, osg::Geode* geode) const {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	return _setFragmentShader(geode, "osgPango-frag1");
}

}
