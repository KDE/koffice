#!/bin/bash

# This script helps finding out problems in the OASIS loading/saving code,
# by converting .kwd -> .oot -> .kwd and comparing the initial and final .kwd files.
# We use the kwd format as a "dump" of the KWord data, to check if everything is correct
# in memory, but the point is of course to ensure that the .oot has all the information.

# To use this script, you need to change this path to an existing kwd file
# Don't use a relative path, dcopstart won't handle it
input=/mnt/devel/kde/kofficetests/documents/native/kword/frames/transparent_pixmaps.kwd

test -f "$input" || { echo "No such file $input"; exit 1; }

# Load KWD
appid=`dcopstart kword $input`
sleep 3

# Save to KWD again (in case of changes in syntax etc.)
origfile=/tmp/oasisregtest-initial.kwd
dcop $appid Document-0 saveAs $origfile || exit 1
sleep 1
test -f $origfile || exit 1

# Save to OOT
tmpoasisfile=/tmp/oasisregtest.oot
dcop $appid Document-0 setOutputMimeType "application/vnd.oasis.opendocument.text" || exit 1
dcop $appid Document-0 saveAs $tmpoasisfile || exit 1
sleep 1
test -f $tmpoasisfile || exit 1

dcopquit $appid

# Load resulting OOT, convert to KWD
tmpnativefile=/tmp/oasisregtest-final.kwd
appid=`dcopstart kword $tmpoasisfile`
# openURL doesn't clear enough stuff from the previous doc. I get crashes when saving paraglayouts...
# [pointing to deleted styles]
#dcop $appid Document-0 openURL $tmpoasisfile || exit 1
sleep 2
dcop $appid Document-0 setOutputMimeType "application/x-kword" || exit 1
dcop $appid Document-0 saveAs $tmpnativefile || exit 1
test -f $tmpnativefile || exit 1

# Compare initial and final KWD files
rm -rf /tmp/oasisregtest-orig
mkdir /tmp/oasisregtest-orig
rm -rf /tmp/oasisregtest-final
mkdir /tmp/oasisregtest-final
rm -rf /tmp/oasisregtest-oasis
mkdir /tmp/oasisregtest-oasis
cd /tmp/oasisregtest-orig
unzip $origfile || exit 1
cd /tmp/oasisregtest-final
unzip $tmpnativefile || exit 1
cd /tmp/oasisregtest-oasis
unzip $tmpoasisfile || exit 1
cd ..

diff -urp oasisregtest-orig oasisregtest-final | less

echo "See /tmp/oasisregtest-oasis for the OASIS xml files."

