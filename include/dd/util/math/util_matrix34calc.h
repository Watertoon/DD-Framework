 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#pragma once

namespace dd::util::math {

    /* WARNING: Avoid continous rotations with these unless you know what you're doing */
    NO_CONSTANT_PROPAGATION void RotateLocalX(Matrix34f *out_rot_matrix, float theta);

    NO_CONSTANT_PROPAGATION void RotateLocalY(Matrix34f *out_rot_matrix, float theta);

    NO_CONSTANT_PROPAGATION void RotateLocalZ(Matrix34f *out_rot_matrix, float theta);
}
