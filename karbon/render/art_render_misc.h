/* This file is part of the KDE project.
 * art_render_misc.c: Here I store some routines I feel should be in libart :)
 *
 * Copyright (C) 2002, The Karbon Developers
 *
 * This code is adapted from :
 *
 * art_render_gradient.h: Gradient image source for modular rendering.
 *
 * Libart_LGPL - library of basic graphic primitives
 * Copyright (C) 2000 Raph Levien
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Authors: Raph Levien <raph@acm.org>
 *          Alexander Larsson <alla@lysator.liu.se>
 */

#ifndef __ART_RENDER_MISC_H__
#define __ART_RENDER_MISC_H__

#ifdef LIBART_COMPILATION
#include "art_filterlevel.h"
#include "art_render.h"
#else
#include <libart_lgpl/art_filterlevel.h>
#include <libart_lgpl/art_render.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _ArtGradientLinear ArtGradientLinear;
typedef struct _ArtGradientRadial ArtGradientRadial;
typedef struct _ArtGradientConical ArtGradientConical;
typedef struct _ArtGradientStop ArtGradientStop;

typedef enum {
  ART_GRADIENT_PAD,
  ART_GRADIENT_REFLECT,
  ART_GRADIENT_REPEAT
} ArtGradientSpread;

struct _ArtGradientLinear {
  double a;
  double b;
  double c;
  ArtGradientSpread spread;
  int n_stops;
  ArtGradientStop *stops;
};

struct _ArtGradientRadial {
  double affine[6]; /* transforms user coordinates to unit circle */
  double fx, fy;    /* focal point in unit circle coords */
  int n_stops;
  ArtGradientSpread spread;
  ArtGradientStop *stops;
};

struct _ArtGradientConical {
	double cx, cy;    /* focal point in unit circle coords */
	double r;         /* focal point in unit circle coords */
	ArtGradientSpread spread;
	art_u8 *buf;
	int n_stops;
	ArtGradientStop *stops;
};


struct _ArtGradientStop {
  double offset;
  ArtPixMaxDepth color[ART_MAX_CHAN + 1];
};

void
art_karbon_render_gradient_linear (ArtRender *render,
			    const ArtGradientLinear *gradient,
			    ArtFilterLevel level);

void
art_karbon_render_gradient_radial (ArtRender *render,
			    const ArtGradientRadial *gradient,
			    ArtFilterLevel level);

void
art_karbon_render_gradient_conical (ArtRender *render,
          	      const ArtGradientConical *gradient,
           	      ArtFilterLevel level);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ART_RENDER_MISC_H__ */
