//========================================================================
//
// Error.h
//
// Copyright 1996-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef ERROR_H
#define ERROR_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include "xpdf_config.h"

extern void CDECL error(int pos, const char *msg, ...);

#endif
