#include <iostream>
#include <osgPango/Context>

int main(int argc, char** argv) {
	osgPango::Context::instance().init(132);

	osgPango::FontList fl;

	unsigned int numFonts = osgPango::Context::instance().getFontList(fl);

	std::cout << "Found " << numFonts << " font families." << std::endl;

	for(osgPango::FontList::iterator i = fl.begin(); i != fl.end(); i++) std::cout
		<< *i << std::endl
	;

	return 0;
}
