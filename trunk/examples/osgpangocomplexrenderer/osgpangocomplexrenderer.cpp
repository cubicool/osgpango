// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

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

struct GlyphLayerLines: public osgPango::GlyphLayer {
	virtual bool render(
		cairo_t*       c,
		cairo_glyph_t* glyph,
		unsigned int   width,
		unsigned int   height
	) {
		if(cairo_status(c) || !glyph) return false;

		cairo_surface_t* tmp = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
	
		if(cairo_surface_status(tmp)) return false;

		cairo_t* tc = cairo_create(tmp);

		if(cairo_status(tc)) return false;

		// Clear the temporary 'pattern' surface.
		cairo_save(tc);
		cairo_set_operator(tc, CAIRO_OPERATOR_CLEAR);
		cairo_rectangle(tc, 0.0f, 0.0f, width, height);
		cairo_paint(tc);
		cairo_restore(tc);

		// Now, we draw our lines to the temporary surface.
		cairo_set_line_width(tc, 1.0f);

		for(unsigned int w = 0; w < width; w += 3) {
			cairo_move_to(tc, w + 0.5f, 0.0);
			cairo_line_to(tc, w + 0.5f, height);
			cairo_stroke(tc);
		}

		for(unsigned int h = 0; h < height; h += 3) {
			cairo_move_to(tc, 0.0f, h + 0.5f);
			cairo_line_to(tc, width, h + 0.5f);
			cairo_stroke(tc);
		}

		// Use our temporary surface to fill the glyph path.
		cairo_set_source_surface(c, tmp, 0, 0);
		cairo_glyph_path(c, glyph, 1);
		cairo_fill(c);

		cairo_destroy(tc);
		cairo_surface_destroy(tmp);

		return true;
	}
};

struct GlyphRendererComplex: public osgPango::GlyphRenderer {
	GlyphRendererComplex(bool useCustomLayer = false) {
		addLayer(new osgPango::GlyphLayerShadowGaussian(17.0f));
		addLayer(new osgPango::GlyphLayerOutline(2.0f));

		if(useCustomLayer) addLayer(new GlyphLayerLines());
		
		else addLayer(new osgPango::GlyphLayer());
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

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;

int main(int argc, char** argv) {
	osgPango::Context& context = osgPango::Context::instance();

	context.init();
	context.addGlyphRenderer("complex", new GlyphRendererComplex());
	context.addGlyphRenderer("complex-lines", new GlyphRendererComplex(true));

	osgPango::TextTransform* t = new osgPango::TextTransform();

	t->addText(
		"<span font='Verdana Bold 40'>This is a cow.</span>",
		0,
		0,
		osgPango::TextOptions("complex")
	);

	t->addText(
		"<span font='Verdana Bold 40'>Yes, a cow.</span>",
		0,
		-60,
		osgPango::TextOptions("complex-lines")
	);

	t->finalize();
	t->setMatrix(osg::Matrixd::translate(osg::Vec3(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f)));
	t->setPositionAlignment(osgPango::TextTransform::POS_ALIGN_CENTER);

	osgViewer::Viewer viewer;

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = createOrthoCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
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
	viewer.setUpViewInWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);

	viewer.run();

	osgPango::Context::instance().writeCachesToPNGFiles("osgpangocustomrenderer");
	
	unsigned long bytes = osgPango::Context::instance().getMemoryUsageInBytes();
	
	osg::notify(osg::NOTICE)
		<< "Used " << (bytes / 1024.0f) / 1024.0f << "MB of Image data internally."
		<< std::endl
	;

	return 0;
}
