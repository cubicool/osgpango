// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osg/Geode>
#include <osgPango/GlyphRenderer>

namespace osgPango {

// 16 = 1:1
// 32 = 2:1
// 64 = 4:1
GlyphRendererDistanceField::GlyphRendererDistanceField(GlyphRendererDistanceField::Mode mode) {
	setTextureSize(1024, 1024);

	addLayer(new GlyphLayerDistanceField());
}

bool GlyphRendererDistanceField::updateOrCreateState(int pass, osg::Geode* geode) const {
	if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

	updateScaleState(4.0f, geode->getOrCreateStateSet());

	return _setFragmentShader(geode, "osgPango-distanceField");
}

const char* PANGO_DFMIN_NAME = "pangoDFMin";
const char* PANGO_DFMAX_NAME = "pangoDFMax";

void GlyphRendererDistanceField::updateScaleState(osg::Vec3::value_type scale, osg::StateSet* state) {
	float pdfMin = std::max(0.0f, 0.5f - 0.07f / (scale / 2.0f));
	float pdfMax = std::min(0.5f + 0.07f / (scale / 2.0f), 1.0f);

	osg::Uniform* pdfMinUniform = state->getUniform(PANGO_DFMIN_NAME);
	osg::Uniform* pdfMaxUniform = state->getUniform(PANGO_DFMAX_NAME);
	
	if(!pdfMinUniform) {
		pdfMinUniform = new osg::Uniform(PANGO_DFMIN_NAME, pdfMin);

		state->addUniform(pdfMinUniform);
	}

	else pdfMinUniform->set(pdfMin);
	
	if(!pdfMaxUniform) {
		pdfMaxUniform = new osg::Uniform(PANGO_DFMAX_NAME, pdfMax);

		state->addUniform(pdfMaxUniform);
	}

	else pdfMaxUniform->set(pdfMax);
}

}
