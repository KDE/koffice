/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2004 Peter Simonsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KIVIOKIVIO_CONFIG_H
#define KIVIOKIVIO_CONFIG_H

#include <koGlobal.h>

#include "kivio_settings.h"

namespace Kivio {
  /**
  * This class handles all settings
  */
  class Config : public Kivio::Settings
  {
    public:
      /**
      * Set the default page layout
      */
      static void setDefaultPageLayout(const KoPageLayout& layout);
      
      /**
      * Get the default page layout
      */
      static KoPageLayout defaultPageLayout();
    
    protected:
      Config();
  };
};

#endif
