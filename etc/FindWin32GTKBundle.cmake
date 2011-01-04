# This file adds support for the GTK+ Win32 Development bundle. It is REALLY
# unlikely that anyone would EVER try and build all this by default.
# You can get the bundle from:
#
# http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.22/gtk+-bundle_2.22.0-20101016_win32.zip
#
# This bundle was tested against on 12/2/2010 by Jeremy Moles.

SET(WIN32_GTK_BUNDLE_DIR "" CACHE PATH "Location of the GTK+ Win32 bundle.")

FIND_PATH(CAIRO_INCLUDE_DIRS cairo.h ${WIN32_GTK_BUNDLE_DIR}/include/cairo)
FIND_PATH(PANGOCAIRO_INCLUDE_DIRS pango/pangocairo.h ${WIN32_GTK_BUNDLE_DIR}/include/pango-1.0)
FIND_PATH(GLIB_INCLUDE_DIRS glib.h ${WIN32_GTK_BUNDLE_DIR}/include/glib-2.0)
FIND_PATH(GLIBCONFIG_INCLUDE_DIRS glibconfig.h ${WIN32_GTK_BUNDLE_DIR}/lib/glib-2.0/include)

FIND_PATH(PANGOCAIRO_LIBRARY_DIRS pangocairo-1.0.lib ${WIN32_GTK_BUNDLE_DIR}/lib)

