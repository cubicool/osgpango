// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPango/GlyphRenderer>
#include <osgPango/GlyphLayer>

static bool checkOutline(const osgPango::GlyphRendererOutline& gro) {
	const osgPango::GlyphLayerOutline* outline =
		dynamic_cast<const osgPango::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(outline) return outline->getOutline() != osgPango::DEFAULT_OUTLINE;

	return false;
} 

static bool readOutline(osgDB::InputStream& is, osgPango::GlyphRendererOutline& gro) {
	osgPango::GlyphLayerOutline* outline =
		dynamic_cast<osgPango::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(!outline) return false;

	unsigned int outlineValue;

	is >> outlineValue;

	outline->setOutline(outlineValue);

	return true;
}

static bool writeOutline(osgDB::OutputStream& os, const osgPango::GlyphRendererOutline& gro) {
	const osgPango::GlyphLayerOutline* outline =
		dynamic_cast<const osgPango::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(!outline) return false;

	os << outline->getOutline() << std::endl;

	return true;
}

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRendererOutline,
	new osgPango::GlyphRendererOutline(),
	osgPango::GlyphRendererOutline,
	"osg::Object osgPango::GlyphRenderer osgPango::GlyphRendererOutline"
) {
	ADD_USER_SERIALIZER(Outline);
}

