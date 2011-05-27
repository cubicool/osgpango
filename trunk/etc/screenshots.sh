#!/usr/bin/env bash

TEXT1="<span font='droid sans 100' bgcolor='#fff'><b>osgPango</b></span>"
TEXT2="<span font='verdana 100' bgcolor='#f00'><b>osg<span fgcolor='#f80'>Pang</span>o</b></span>"
TEXT3="<span font='droid sans mono 11' fgcolor='#fff'>Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</span>"

function DoExample() {
	local FILENAME="screenshots/${1}"

	shift 1

	osgpangoviewer ${@} &
	
	while true; do
		if import -window osgpangoviewer -crop 800x175+0+425 "${FILENAME}" >/dev/null 2>&1; then
			break
		
		else
			sleep 0.2
		fi
	done

	while true; do
		WID="$(ps axf | grep [o]sgpangoviewer | cut -f 1 -d ' ')"

		if [ -n "${WID}" ]; then
			kill "${WID}"

			break
		
		else
			sleep 0.2
		fi
	done

	sleep 2
}

[ ! -d screenshots ] && mkdir screenshots

rm -f screenshots/*.png

DoExample basic.png --alignment justify
DoExample small.png "${TEXT3}"
DoExample basic-shadow.png --renderer shadow 1
DoExample bitmap-rgb24-and-outline.png --bitmap ../etc/bitmap-rgb24.png --renderer outline 2 "${TEXT1}"
DoExample bitmap-argb32-and-outline.png --bitmap ../etc/bitmap-argb32.png --renderer outline 4 "${TEXT1}"
DoExample multispan-shadow.png --renderer shadowBlur 8 "${TEXT2}"

