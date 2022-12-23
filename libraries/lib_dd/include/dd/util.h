 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <dd/util/util_result.hpp>
#include <dd/util/util_result_ukern.h>
#include <dd/util/util_result_trace.h>

#include <dd/util/util_new.h>
#include <dd/util/util_singleton.h>

#include <dd/util/util_alignment.hpp>
#include <dd/util/util_sizeconstants.hpp>
#include <dd/util/util_countbits.hpp>
#include <dd/util/util_member.hpp>
#include <dd/util/util_intrusivelist.hpp>
#include <dd/util/util_intrusivetreenode.hpp>
#include <dd/util/util_critsec.hpp>
#include <dd/util/util_condvar.hpp>
#include <dd/util/util_typestorage.hpp>
#include <dd/util/util_delegate.hpp>
#include <dd/util/util_delegate1.hpp>
#include <dd/util/util_delegate2.hpp>
#include <dd/util/util_messagequeue.hpp>
#include <dd/util/util_delegatethread.hpp>
#include <dd/util/util_runtimetypeinfo.hpp>
#include <dd/util/util_fixedobjectallocator.hpp>
#include <dd/util/util_heapobjectallocator.hpp>
#include <dd/util/util_pointerarray.hpp>
#include <dd/util/util_heaparray.hpp>
#include <dd/util/util_timestamp.h>
#include <dd/util/util_timespan.hpp>
#include <dd/util/util_constevalfail.hpp>
#include <dd/util/util_tstring.hpp>
#include <dd/util/util_charactercode.hpp>

#include <dd/util/util_spinloopintrinsics.x64.hpp>
#include <dd/util/math/util_constants.hpp>
#include <dd/util/math/util_int128.sse4.hpp>
#include <dd/util/math/util_float128.sse4.hpp>
#include <dd/util/math/util_vector2.hpp>
#include <dd/util/math/util_vector3.hpp>
#include <dd/util/math/util_vector3calc.h>
#include <dd/util/math/util_vector4.hpp>
#include <dd/util/math/util_matrix33.hpp>
#include <dd/util/math/util_matrix34.hpp>
#include <dd/util/math/util_matrix34calc.h>
#include <dd/util/math/util_matrix44.hpp>
#include <dd/util/math/util_clamp.hpp>

#include <dd/util/util_logicalframebuffer.hpp>
#include <dd/util/util_viewport.hpp>
#include <dd/util/util_camera.hpp>
#include <dd/util/util_projection.hpp>

#include <dd/util/util_deltatime.h>
