/* defaults.h
 *
 * Copyright (C) 2010-2016 Bernhard Lang, University of Geneva
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _defaults_h_
#define _defaults_h_

#ifndef NUM_PIXELS
# define NUM_PIXELS 600
#endif
#ifndef FIRST_PIXEL
//# define FIRST_PIXEL 19
# define FIRST_PIXEL 0
#endif
#ifndef USED_PIXELS
# define USED_PIXELS 600
//# define USED_PIXELS 526
#endif
#ifndef LINES
# define LINES 60
#endif
#ifndef VFREQ
# define VFREQ 7
#endif
#ifndef NUM_DMA_BUFFERS
# define  NUM_DMA_BUFFERS 32
#endif
#ifndef MAX_BOARDS
# define  MAX_BOARDS 4
#endif

#ifdef CONFIG_64BIT
typedef uint64_t pmem_uint;
#else
typedef uint32_t pmem_uint;
#endif

#endif /* _defaults_h_ */
