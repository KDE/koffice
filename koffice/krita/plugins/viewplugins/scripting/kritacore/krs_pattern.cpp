/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_pattern.h"

#include <kis_pattern.h>

namespace Kross {

namespace KritaCore {

Pattern::Pattern(KisPattern* pattern, bool sharedPattern) : Kross::Api::Class<Pattern>("KritaPattern"), m_pattern(pattern), m_sharedPattern(sharedPattern)
{
}

Pattern::~Pattern()
{
    if(!m_sharedPattern)
        delete m_pattern;
}

}

}
