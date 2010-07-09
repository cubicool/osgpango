// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <sstream>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/AlphaFunc>
#include <osg/Depth>
#include <osg/ColorMask>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgPango/Context>

const std::string LOREM_IPSUM(
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
	"fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
	"culpa qui officia deserunt mollit anim id est laborum."
);

struct GlyphRendererComplex: public osgPango::GlyphRenderer{
	GlyphRendererComplex() {
		addLayer(new osgPango::GlyphLayerShadowGaussian(20.0));
		addLayer(new osgPango::GlyphLayerOutline(2.0));
		addLayer(new osgPango::GlyphLayer());
	}

	virtual unsigned int getNumPasses() const { 
		return 3;
	}

	bool updateOrCreateState(int pass, osg::Geode* geode) {
		if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

		osg::StateSet* state   = geode->getOrCreateStateSet();
		osg::Program*  program = dynamic_cast<osg::Program*>(state->getAttribute(osg::StateAttribute::PROGRAM));

		if(!program) return false;
		
		switch(pass) {
			// blur shadow
			case 0: {
					const char* GET_FRAGMENT =
						"#version 120\n"
						"vec4 osgPango_GetFragment(vec4 coord, sampler2D textures[8], vec3 colors[8], float alpha) {"
						"	float tex0   = texture2D(textures[0], coord.st).a;"
						"	vec3 color0  = vec3(0.0, 0.0, 0.0) * tex0;"
						"	float alpha0 = tex0;"
						"	return vec4(color0, alpha0 * alpha);"
						"}"
						;

					osg::Shader* frag = new osg::Shader(osg::Shader::FRAGMENT, GET_FRAGMENT);

					program->addShader(frag);
				} 
				break;
			
			// outline + base
			case 1: {
					const char* GET_FRAGMENT =
						"#version 120\n"
						"vec4 osgPango_GetFragment(vec4 coord, sampler2D textures[8], vec3 colors[8], float alpha) {"
						"	float tex0   = texture2D(textures[1], coord.st).a;"
						"	float tex1   = texture2D(textures[2], coord.st).a;"
						"	vec3 color0  = vec3(1.0, 1.0, 1.0) * tex0;"
						"	vec3 color1  = vec3(0.0, 0.4, 1.0) * tex1 + color0 * (1.0 - tex1);"
						"	float alpha0 = tex0;"
						"	float alpha1 = tex0 + tex1;"
						"	return vec4(color1, alpha1 * alpha);"
						"}"
						;
					
					osg::Shader* frag = new osg::Shader(osg::Shader::FRAGMENT, GET_FRAGMENT);

					program->addShader(frag);
				} 
				break;

			// write to depth only with base
			case 2: {
					const char* GET_FRAGMENT =
						"#version 120\n"
						"vec4 osgPango_GetFragment(vec4 coord, sampler2D textures[8], vec3 colors[8], float alpha) {"
						"	float tex0   = texture2D(textures[2], coord.st).a;"
						"	vec3 color0  = vec3(0.0, 0.0, 0.0) * tex0;"
						"	float alpha0 = tex0;"
						"	return vec4(color0, alpha0 * alpha);"
						"}"
						;

					osg::Shader* frag = new osg::Shader(osg::Shader::FRAGMENT, GET_FRAGMENT);

					program->addShader(frag);
					
					state->removeAttribute(osg::StateAttribute::DEPTH);
					state->setAttribute(new osg::ColorMask(false, false, false, false));
					state->setMode(GL_BLEND, osg::StateAttribute::OFF);	
					state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
				} 
				break;
		}
		return true;
	}
};

osg::Geometry* createGeometry(osg::Image* image) {
	static osg::Vec3 pos(50.0f, 50.0f, -0.8f);

	osg::Texture2D* texture = new osg::Texture2D();
	osg::Geometry*  geom    = osg::createTexturedQuadGeometry(
		pos,
		osg::Vec3(image->s(), 0.0f, 0.0f),
		osg::Vec3(0.0f, image->t(), 0.0f),
		0.0f,
		0.0f, 
		1.0f,
		1.0f
	);

	texture->setImage(image);
	texture->setDataVariance(osg::Object::DYNAMIC);

	osg::StateSet* state = geom->getOrCreateStateSet();

	state->setTextureAttributeAndModes(
		0,
		texture,
		osg::StateAttribute::ON
	);

	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	pos += osg::Vec3(image->s() + 50.0f, 0.0f, 0.1f);

	return geom;
}

osg::Matrix createInvertedYOrthoProjectionMatrix(float width, float height) {
	osg::Matrix m = osg::Matrix::ortho2D(0.0f, width, 0.0f, height);
	osg::Matrix s = osg::Matrix::scale(1.0f, -1.0f, 1.0f);
	osg::Matrix t = osg::Matrix::translate(0.0f, -height, 0.0f);

	return t * s * m;
}

osg::Camera* createOrthoCamera(float width, float height) {
	osg::Camera* camera = new osg::Camera();

	camera->getOrCreateStateSet()->setMode(
		GL_LIGHTING,
		osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF
	);

	camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, width, 0.0f, height));
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);

	return camera;
}

osg::Camera* createInvertedYOrthoCamera(float width, float height) {
	osg::Camera* camera = createOrthoCamera(width, height);

	camera->setProjectionMatrix(createInvertedYOrthoProjectionMatrix(width, height));

	return camera;
}

int main(int argc, char** argv) {
	osgPango::Context& context = osgPango::Context::instance();

	context.init();
	context.addGlyphRenderer("complex", new GlyphRendererComplex());

	osgPango::TextTransform* t = new osgPango::TextTransform();

	std::ostringstream os;
	
	os << "<span font='Verdana Bold 50'>" << LOREM_IPSUM << "</span>";

	t->addText(os.str().c_str(), 0, 0, osgPango::TextOptions(
		"complex",
		osgPango::TextOptions::TEXT_ALIGN_CENTER,
		1230
	));

	t->finalize();
	// t->setPosition(t->getOriginTranslated());

	osgViewer::Viewer viewer;

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = createOrthoCamera(1280, 1024);
	osg::Node*   node   = osgDB::readNodeFile("cow.osg");
	
        viewer.addEventHandler(new osgViewer::StatsHandler());
        viewer.addEventHandler(new osgViewer::WindowSizeHandler());
        viewer.addEventHandler(new osgGA::StateSetManipulator(
                viewer.getCamera()->getOrCreateStateSet()
        ));

	camera->addChild(t);

	group->addChild(node);
	group->addChild(camera);

	viewer.setSceneData(group);
	viewer.getCamera()->setClearColor(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
	viewer.setUpViewInWindow(50, 50, 1280, 1024);

	viewer.run();

	// osgPango::Context::instance().writeCachesToPNGFiles("osgpangocustomrenderer");

	return 0;
}
