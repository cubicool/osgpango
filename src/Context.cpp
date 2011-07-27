// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <algorithm>
#include <sstream>
#include <osgPango/Context>

namespace osgPango {

const unsigned int DEFAULT_CACHE_WIDTH  = 256;
const unsigned int DEFAULT_CACHE_HEIGHT = 256;

Context Context::_context;

G_DEFINE_TYPE(Renderer, renderer, PANGO_TYPE_RENDERER);

#define TYPE_RENDERER (renderer_get_type())
#define RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_RENDERER, Renderer))

static GObjectClass* _pangoClass = 0;

void renderer_finalize(GObject* object) {
	Renderer* priv = RENDERER(object);

	G_OBJECT_CLASS(_pangoClass)->finalize(object);
}

void renderer_init(Renderer* priv) {
}

void renderer_class_init(RendererClass* klass) {
	GObjectClass*       object_class   = G_OBJECT_CLASS(klass);
	PangoRendererClass* renderer_class = PANGO_RENDERER_CLASS(klass);

	_pangoClass = static_cast<GObjectClass*>(g_type_class_peek_parent(klass));

	object_class->finalize = renderer_finalize;

	renderer_class->draw_glyphs = &Context::drawGlyphs;
}

Context::~Context() {
	if(_renderer) g_object_unref(_renderer);

	// if(_pfMap) g_object_unref(_pfMap);
	
	if(_pContext) g_object_unref(_pContext);
}

Context& Context::instance() {
	return _context;
}

bool Context::init(
	unsigned int           dpi,
	cairo_antialias_t      aa,
	cairo_hint_style_t     hs,
	cairo_subpixel_order_t spo
) {
	if(_pfMap && _pContext) return false;

	if(!_pfMap) _pfMap = pango_cairo_font_map_new();
	
	if(_pfMap && !_pContext) _pContext = pango_cairo_font_map_create_context(
		PANGO_CAIRO_FONT_MAP(_pfMap)
	);

	if(!_pfMap || !_pContext) return false;

	// TODO: Fix this...
	cairo_font_options_t* options = cairo_font_options_create();

	cairo_font_options_set_antialias(options, aa);
	cairo_font_options_set_hint_style(options, hs);
	cairo_font_options_set_subpixel_order(options, spo);

	pango_cairo_context_set_font_options(_pContext, options);
	pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(_pfMap), dpi);

	cairo_font_options_destroy(options);

	_renderer = static_cast<Renderer*>(g_object_new(TYPE_RENDERER, 0));

	// Add our custom attribute types here.
	// pango_attr_type_register("cache");

	return true;
}

unsigned int Context::getFontList(FontList& fl, bool faces) {
	if(!_pContext) return false;

	PangoFontFamily** pff = 0;
	int               num = 0;

	pango_context_list_families(_pContext, &pff, &num);

	if(!num || !pff) return 0;

	for(int i = 0; i < num; i++) {
		std::ostringstream ss;

		ss << pango_font_family_get_name(pff[i]);
		
		if(faces) {
			ss << " (";
		
			PangoFontFace** faces    = 0;
			int             numFaces = 0;

			pango_font_family_list_faces(pff[i], &faces, &numFaces);

			for(int j = 0; j < numFaces; j++) {
				ss << pango_font_face_get_face_name(faces[j]);

				if(j + 1 != numFaces) ss << ", ";
			}

			g_free(faces);

			ss << ")";
		}

		fl.push_back(ss.str());
	}

	g_free(pff);

	return static_cast<unsigned int>(num);
}

void Context::drawGlyphs(
	PangoRenderer*    renderer,
	PangoFont*        font,
	PangoGlyphString* glyphs,
	int               x,
	int               y
) {
	ContextDrawable* drawable  = instance()._drawable;

	if(!drawable) return;
	
	// Setup any kind of new "state" here.
	ColorPair& color = instance()._color;

	// Begin querying Pango context variables.
	PangoColor* fg = pango_renderer_get_color(renderer, PANGO_RENDER_PART_FOREGROUND);
	PangoColor* bg = pango_renderer_get_color(renderer, PANGO_RENDER_PART_BACKGROUND);

	if(fg) color.first.set(
		fg->red / 65535.0f,
		fg->green / 65535.0f,
		fg->blue / 65535.0f
	);

	else color.first.set(1.0f, 1.0f, 1.0f);

	if(bg) color.second.set(
		bg->red / 65535.0f,
		bg->green / 65535.0f,
		bg->blue / 65535.0f
	);

	else color.second.set(0.0f, 0.0f, 0.0f);
	
	drawable->drawGlyphs(font, glyphs, x, y);
}

void Context::drawLayout(ContextDrawable* drawable, PangoLayout* layout, int x, int y) {
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(instance()._mutex);

	_drawable = drawable;
	
	pango_renderer_draw_layout(
		PANGO_RENDERER(_renderer),
		layout,
		x * PANGO_SCALE,
		-(y * PANGO_SCALE)
	);

	_drawable = 0;
}

void Context::writeCachesToPNGFiles(const std::string& path) const {
	for(GlyphRendererMap::const_iterator i = _grMap.begin(); i != _grMap.end(); i++) {
		GlyphRenderer* r = i->second.get();

		std::string prefix = path + "_" + i->first;
		
		for(
			GlyphRenderer::FontGlyphCacheMap::const_iterator j = r->getGlyphCaches().begin();
			j != r->getGlyphCaches().end();
			j++
		) {
			GlyphCache* gc = j->second.get();
			
			std::ostringstream ss;
			
			ss 
				<< prefix
				<< "_"
				<< j->first
				<< "_"
			;
			
			gc->writeImagesAsFiles(ss.str());
		}
	}
}

unsigned long Context::getMemoryUsageInBytes() const {
	unsigned long bytes = 0;

	for(GlyphRendererMap::const_iterator i = _grMap.begin(); i != _grMap.end(); i++) {
		GlyphRenderer* r = i->second.get();

		for(
			GlyphRenderer::FontGlyphCacheMap::const_iterator j = r->getGlyphCaches().begin();
			j != r->getGlyphCaches().end();
			j++
		) {
			GlyphCache* gc = j->second.get();
			
			bytes += gc->getMemoryUsageInBytes();
		}
	}

	return bytes;
}

void Context::setDefaultGlyphRenderer(GlyphRenderer* renderer) {
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
	
	_grMap[""] = renderer;
}

void Context::addGlyphRenderer(const std::string& name, GlyphRenderer* renderer) {
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
	
	_grMap[name] = renderer;
	
	if(_onAddCallback) (*_onAddCallback)(renderer);

	renderer->_name = name;
}

void Context::removeGlyphRenderer(const std::string& name) {
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

	GlyphRenderer* gr = _grMap[name].get();

	if(gr && _onRemoveCallback) (*_onRemoveCallback)(gr);
	
	_grMap.erase(name);
}

void Context::reset() {
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
	
	_grMap.clear();

	_drawable = 0;
	
	setDefaultGlyphRenderer(new GlyphRendererDefault());
}

Context::Context():
_pfMap         (0),
_pContext      (0),
_renderer      (0),
_drawable      (0),
_color         (ColorPair(osg::Vec3(1.0f, 1.0f, 1.0f), osg::Vec3(0.0f, 0.0f, 0.0f))),
_textureWidth  (DEFAULT_CACHE_WIDTH),
_textureHeight (DEFAULT_CACHE_HEIGHT) {
	setDefaultGlyphRenderer(new GlyphRendererDefault());
}

GlyphRenderer* Context::_getGlyphRenderer(const std::string& renderer) const {
	const GlyphRendererMap::const_iterator i = _grMap.find(renderer);
	
	if(i == _grMap.end()) return 0;

	return i->second.get();
}

}
