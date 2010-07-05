#include <osg/TexMat>
#include <osg/TexEnvCombine>
#include <osg/AlphaFunc>
#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osgPango/GeometryStatePolicy>

namespace osgPango {

BaseGeometryState::BaseGeometryState() :
_alpha(1.0f) {
}


void TextEnvGeometryState::apply(osg::Geode * geode) const {
	geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	geode->getOrCreateStateSet()->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.01f));
	geode->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
}

void TextEnvGeometryState::apply(osg::Geometry *obj, const GlyphGeometryState &gs) const {
	osg::StateSet*      state = obj->getOrCreateStateSet();

	state->setTextureAttributeAndModes(0, gs.texture, osg::StateAttribute::ON);
	state->setTextureAttributeAndModes(1, gs.effectsTexture, osg::StateAttribute::ON);

	osg::TexEnvCombine* te0   = new osg::TexEnvCombine();

	state->setTextureAttributeAndModes(
		gs.effectsTexture ? 1 : 0,
		gs.texture,
		osg::StateAttribute::ON
	);

	// This is the color of the border...
	te0->setConstantColor(osg::Vec4(gs.effectsTexture ? gs.effectsColor : gs.color, 1.0f));

	// RGB setup for te0.
	te0->setCombine_RGB(osg::TexEnvCombine::MODULATE);
	te0->setSource0_RGB(osg::TexEnvCombine::CONSTANT);
	te0->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
	te0->setOperand1_RGB(osg::TexEnvCombine::SRC_ALPHA);

	// Alpha setup for te0.
	te0->setCombine_Alpha(osg::TexEnvCombine::REPLACE);
	te0->setSource0_Alpha(osg::TexEnvCombine::TEXTURE0);
	te0->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);

	state->setTextureAttributeAndModes(0, te0, osg::StateAttribute::ON);

	if(gs.effectsTexture) {
		osg::TexEnvCombine* te1 = new osg::TexEnvCombine();

		// This is the color of the text...
		te1->setConstantColor(osg::Vec4(gs.color, 1.0f));

		// RGB setup for te1.
		te1->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
		te1->setSource0_RGB(osg::TexEnvCombine::CONSTANT);
		te1->setSource1_RGB(osg::TexEnvCombine::PREVIOUS);
		te1->setSource2_RGB(osg::TexEnvCombine::TEXTURE1);
		te1->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
		te1->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
		te1->setOperand2_RGB(osg::TexEnvCombine::SRC_ALPHA);

		// Alpha setup for te1.
		te1->setCombine_Alpha(osg::TexEnvCombine::ADD);
		te1->setSource0_Alpha(osg::TexEnvCombine::TEXTURE1);
		te1->setSource1_Alpha(osg::TexEnvCombine::PREVIOUS);
		te1->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);
		te1->setOperand1_Alpha(osg::TexEnvCombine::SRC_ALPHA);

		state->setTextureAttributeAndModes(
			0,
			gs.effectsTexture,
			osg::StateAttribute::ON
		);

		state->setTextureAttributeAndModes(1, te1, osg::StateAttribute::ON);
	}

	state->setTextureAttributeAndModes(
		gs.effectsTexture ? 2 : 1,
		gs.texture,
		osg::StateAttribute::ON
	);

	// The ALPHA stuff.
	osg::TexEnvCombine* te2 = new osg::TexEnvCombine();
	
	te2->setConstantColor(osg::Vec4(0.0f, 0.0f, 0.0f, _alpha));

	te2->setCombine_RGB(osg::TexEnvCombine::REPLACE);
	te2->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
	te2->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
	te2->setCombine_Alpha(osg::TexEnvCombine::MODULATE);
	te2->setSource0_Alpha(osg::TexEnvCombine::CONSTANT);
	te2->setSource1_Alpha(osg::TexEnvCombine::PREVIOUS);
	te2->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);
	te2->setOperand1_Alpha(osg::TexEnvCombine::SRC_ALPHA);

	state->setTextureAttributeAndModes(
		gs.effectsTexture ? 2 : 1,
		te2,
		osg::StateAttribute::ON
	);

	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.01f));
}


const char* VERTEX_SHADER =
		"#version 120\n"
		"varying vec4 pangoTexCoord;"
		"void main() {"
		"	pangoTexCoord = gl_MultiTexCoord0;"
		"	gl_Position = ftransform();"
		"}"
		;
	
	const char* FRAGMENT_SHADER =
		"varying vec4 pangoTexCoord;"
		"uniform vec3 pangoColor[8];"
		"uniform sampler2D pangoTex;"
		"uniform float pangoAlpha;"
		"void main() {"
		"	float alpha = texture2D(pangoTex, pangoTexCoord.st).a;"
		"	gl_FragColor = vec4(pangoColor[0].rgb, alpha * pangoAlpha);"
		"}"
		;

GLSLGeometryState::GLSLGeometryState(): 
_vertexShader(VERTEX_SHADER),
_fragmentShader(FRAGMENT_SHADER) {

}


void GLSLGeometryState::apply(osg::Geode * geode) const {
	osg::Program* program = new osg::Program();
	osg::Shader*  vert    = new osg::Shader(osg::Shader::VERTEX, _vertexShader);
	osg::Shader*  frag    = new osg::Shader(osg::Shader::FRAGMENT, _fragmentShader);

	vert->setName("pangoRendererVert");
	frag->setName("pangoRendererFrag");
	program->setName("pangoRenderer");

	program->addShader(vert);
	program->addShader(frag);

	osg::StateSet* state = geode->getOrCreateStateSet();

	state->setAttributeAndModes(program);
	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.01f));
	state->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
	
	state->getOrCreateUniform("pangoAlpha", osg::Uniform::FLOAT)->set(_alpha);
	state->getOrCreateUniform("pangoTex", osg::Uniform::INT)->set(0);
}

void GLSLGeometryState::apply(osg::Geometry *geometry, const GlyphGeometryState &gs) const {
	osg::Uniform* pangoColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "pangoColor", 8);

	pangoColor->setElement(0, gs.color);
	pangoColor->setElement(1, gs.effectsColor);
	
	osg::StateSet* state = geometry->getOrCreateStateSet();

	state->setTextureAttributeAndModes(0, gs.texture, osg::StateAttribute::ON);
	state->setTextureAttributeAndModes(1, gs.effectsTexture, osg::StateAttribute::ON);
	state->addUniform(pangoColor);
}

}