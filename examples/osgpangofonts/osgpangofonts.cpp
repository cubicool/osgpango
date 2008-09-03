#include <iostream>
#include <osgPango/Font>

int main(int argc, char** argv) {
	osgPango::Font::init(132);

	osgPango::Font::FontList fl;

	unsigned int numFonts = osgPango::Font::getFontList(fl);

	std::cout << "Found " << numFonts << " font families." << std::endl;

	for(osgPango::Font::FontList::iterator i = fl.begin(); i != fl.end(); i++) std::cout
		<< *i << std::endl
	;

	osgPango::Font::cleanup();

	return 0;
}
