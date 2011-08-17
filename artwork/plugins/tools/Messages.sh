#! /bin/sh
$EXTRACTRC ../../ui/ArtworkPatternOptionsWidget.ui filterEffectTool/FilterEffectEditWidget.ui >> rc.cpp
$XGETTEXT rc.cpp *.cpp *.h CalligraphyTool/*.cpp CalligraphyTool/*.h filterEffectTool/*cpp ../../ui/ArtworkPatternOptionsWidget.cpp \
-o $podir/ArtworkTools.pot
