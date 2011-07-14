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
#include <osgPango/ShaderGenerator>
#include <osgPango/ShaderManager>

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
	GlyphRendererComplex() {
		addLayer(new osgPango::GlyphLayerShadowBlur(0.0f, 0.0f, 10, 5.0));
		addLayer(new osgPango::GlyphLayerOutline(2.0f));
		addLayer(new GlyphLayerLines());

		unsigned int liv2[] = {1, 2};
		unsigned int liv3[] = {2};

		osgPango::ShaderManager& sm = osgPango::ShaderManager::instance();

		sm.addShaderSource(
			"my-shader-pass-2",
			osg::Shader::FRAGMENT,
			osgPango::createLayerIndexShader(
				3,
				osgPango::LayerIndexVector(liv2, liv2 + 2)
			)
		);
		
		sm.addShaderSource(
			"my-shader-pass-3",
			osg::Shader::FRAGMENT,
			osgPango::createLayerIndexShader(
				3,
				osgPango::LayerIndexVector(liv3, liv3 + 1)
			)
		);

		osg::notify(osg::NOTICE)
			<< sm.getShader("my-shader-pass-2")->getShaderSource()
			<< std::endl
		;

		osg::notify(osg::NOTICE)
			<< sm.getShader("my-shader-pass-3")->getShaderSource()
			<< std::endl
		;
	}

	virtual unsigned int getNumPasses() const {
		return 3;
	}

	bool updateOrCreateState(int pass, osg::Geode* geode) const {
		if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

		osg::StateSet* state = geode->getOrCreateStateSet();
		
		osg::Program* program = dynamic_cast<osg::Program*>(
			state->getAttribute(osg::StateAttribute::PROGRAM)
		);

		if(!program) return false;
		
		osgPango::ShaderManager& sm = osgPango::ShaderManager::instance();

		osg::Shader* frag = 0;
	
		// Blurred shadow.
		if(pass == 0) frag = sm.getShader("osgPango-frag1");
		
		// Outline + base glyph.
		else if(pass == 1) frag = sm.getShader("my-shader-pass-2");

		// Write to depth only with base.
		else if(pass == 2) {
			frag = sm.getShader("my-shader-pass-3");
			
			state->removeAttribute(osg::StateAttribute::DEPTH);
			state->setAttribute(new osg::ColorMask(false, false, false, false));
			state->setMode(GL_BLEND, osg::StateAttribute::OFF);	
			state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
		} 

		if(frag) program->addShader(frag);

		return true;
	}
};

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

	osgPango::TextTransform* t = new osgPango::TextTransform(osgPango::Text::COLOR_MODE_PALETTE_ONLY);

	osgPango::ColorPalette cp;

	cp.push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	cp.push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	cp.push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

	t->setColorPalette(cp);
	t->setGlyphRenderer("complex");
	t->addText("<span font='Verdana Bold 40'>This is a cow.</span>", 0, 0);
	t->addText("<span font='Verdana Bold 40'>Yes, a cow.</span>", 0, -60);
	t->finalize();
	t->setMatrix(osg::Matrixd::translate(osg::Vec3(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f)));
	t->setPositionAlignment(osgPango::TextTransform::POS_ALIGN_CENTER_CENTER);

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

	// context.writeCachesToPNGFiles("osgpangocomplexrenderer");
	
	unsigned long bytes = context.getMemoryUsageInBytes();
	
	osg::notify(osg::NOTICE)
		<< "Used " << (bytes / 1024.0f) / 1024.0f << "MB of Image data internally."
		<< std::endl
	;

	return 0;
}
