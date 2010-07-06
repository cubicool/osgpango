// -*-c++-*- Copyright (C) 2010 osgPango Development Team

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
	unsigned int                 dpi,
	osgCairo::CairoAntialias     aa,
	osgCairo::CairoHintStyle     hs,
	osgCairo::CairoSubpixelOrder spo
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
	pango_attr_type_register("cache");

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

GlyphCache* Context::getGlyphCache(PangoFont* font, const std::string& renderer) {
	static unsigned int cacheId = 0;

	GlyphCacheFontMapKey key(font, renderer);

	GlyphCache* gc = _gcfMap[key].get();

	if(!gc) {
		gc = new GlyphCache(_grMap[renderer], _textureWidth, _textureHeight);

		std::ostringstream ss;
		
		ss << "GlyphCache_" << cacheId;

		if(!renderer.empty()) ss << "_" << renderer;

		gc->setName(ss.str());

		cacheId++;

		_gcfMap[key] = gc;
	}

	return gc;
}

const GlyphRenderer* Context::getGlyphRenderer(const std::string& renderer) const {
	const GlyphRendererMap::const_iterator i = _grMap.find(renderer);

	if(i == _grMap.end()) return 0;

	return i->second;
}

void Context::drawGlyphs(
	PangoRenderer*    renderer,
	PangoFont*        font,
	PangoGlyphString* glyphs,
	int               x,
	int               y
) {
	Text* text  = instance()._text;

	if(!text) return;

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

	text->drawGlyphs(font, glyphs, x, y);
}

void Context::drawLayout(Text* text, PangoLayout* layout, int x, int y) {
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(instance()._mutex);

	_text = text;
	
	pango_renderer_draw_layout(
		PANGO_RENDERER(_renderer),
		layout,
		x * PANGO_SCALE,
		-(y * PANGO_SCALE)
	);

	_text = 0;
}

void Context::writeCachesToPNGFiles(const std::string& path) {
	// TODO: Temporary hack!
	static int count = 0;

	for(GlyphCacheFontMap::iterator i = _gcfMap.begin(); i != _gcfMap.end(); i++) {
		PangoFontDescription* d  = pango_font_describe(i->first.first);
		GlyphCache*           gc = i->second.get();

		std::ostringstream os;

		/*
		std::string family(pango_font_description_get_family(d));

		// Get the PangoStyle and convert it to a string.
		std::string style("Normal");
		
		PangoStyle pStyle = pango_font_description_get_style(d);

		if(pStyle == PANGO_STYLE_OBLIQUE) style = "Oblique";

		else if(pStyle == PANGO_STYLE_ITALIC) style = "Italic";

		unsigned int size = pango_font_description_get_size(d) / PANGO_SCALE;

		std::replace(family.begin(), family.end(), ' ', '_');

		os << path << "_" << family << "_" << style << "_" << size;
		*/

		os << path << "_" << count << "_";

		count++;

		gc->writeImagesAsFiles(os.str());

		pango_font_description_free(d);
	}
}

bool Context::addGlyphRenderer(const std::string& name, GlyphRenderer* renderer) {
	std::string key(name);

	_grMap[key] = renderer;

	return true;
}

Context::Context():
_pfMap         (0),
_pContext      (0),
_renderer      (0),
_text          (0),
_color         (ColorPair(osg::Vec3(1.0f, 1.0f, 1.0f), osg::Vec3(0.0f, 0.0f, 0.0f))),
_textureWidth  (DEFAULT_CACHE_WIDTH),
_textureHeight (DEFAULT_CACHE_HEIGHT) {
	_grMap[""] = new GlyphRenderer();
}

}
