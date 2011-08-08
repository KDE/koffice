#! /bin/sh
$EXTRACTRC ../../ui/widgets/ArtworkPatternOptionsWidget.ui filterEffectTool/FilterEffectEditWidget.ui >> rc.cpp
$XGETTEXT rc.cpp *.cpp *.h CalligraphyTool/*.cpp CalligraphyTool/*.h filterEffectTool/*cpp ../../ui/widgets/ArtworkPatternOptionsWidget.cpp \
-o $podir/ArtworkTools.pot
