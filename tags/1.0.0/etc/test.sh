MARKUP1="<span font='Sans 40'>\
<span color='red'>C</span>\
<span color='orange'>E</span>\
<span color='yellow'>D</span>\
<span color='green'>R</span>\
<span color='blue'>I</span>\
<span color='purple'>C</span>, this is all <b>ONE TEXT!</b><i>!!!!!</i>
</span>
"

MARKUP="
<span font='Sans 20'>
<b>Hi All</b>,

<span color='red'>I've now finished all the merges/bug/features fixes that is required </span>\
<span color='orange'>for OpenSceneGraph-2.8 branch.  I had to do a few more changes since </span>\
<span color='yellow'>2.7.9 than I would have liked due to resolve usage problems associated </span>\
<span color='green'>with the osg::TransferFunction1D class.  TransferFunction1D has had to </span>\
<span color='blue'>be refactored in API and implementation.  I've done testing here at it </span>\
<span color='purple'>looks to be running fine (now with .osg support that was missing in 2.7.9) </span>\
<span color='black'>but as I don't have Windows and other platforms I can't test the build </span>\
<span color='white'>there, so pretty please could people do an svn update and let me know how you get on.</span>

In prep for the branch I've now \
<span font='Sans 40'><b><span color='#69c'>u</span>\
<span color='#7ad'>p</span>\
<span color='#8be'>d</span>\
<span color='#9cf'>a</span>\
<span color='#adf'>t</span>\
<span color='#bef'>e</span>d</b></span> \
the version and so numbers so \
we now have:

	<span font='monspace 10'><i>OpenThreads-2.4.0, SO version 11
	OpenSceneGraph-2.8.0, SO version 55</i></span>

<span font='Georgia 18'>\
I will now go across the machines I have and test out the build with a \
fresh checkout.  I'll also do testing on an OSX box that I can \
remotely log into - I'll test just the CMake/Makefile build though, \
won't be able to test the XCode projects.</span>

Robert.</span>"

if [ "${1}" = "valgrind" ]; then
	valgrind --leak-check=full osgpangoviewer "${MARKUP}" 2> valgrind.txt

else
	osgpangoviewer --width 797 --renderer outline 1 "${MARKUP}" 
	# osgpangoviewer --width 797 --alignment justify "${MARKUP}" 
fi
