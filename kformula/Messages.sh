#! /bin/sh
$PREPARETIPS >> rc.cpp
$XGETTEXT *.h *.cpp -o $podir/kformula.pot
