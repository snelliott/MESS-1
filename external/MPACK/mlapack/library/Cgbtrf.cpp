/*
 * Copyright (c) 2008-2010
 *      Nakata, Maho
 *      All rights reserved.
 *
 *  $Id: Cgbtrf.cpp,v 1.8 2010/08/07 04:48:32 nakatamaho Exp $ 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/*
Copyright (c) 1992-2007 The University of Tennessee.  All rights reserved.

$COPYRIGHT$

Additional copyrights may follow

$HEADER$

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer. 
  
- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer listed
  in this license in the documentation and/or other materials
  provided with the distribution.
  
- Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.
  
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT  
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include <mblas.h>
#include <mlapack.h>

void Cgbtrf(INTEGER m, INTEGER n, INTEGER kl, INTEGER ku, COMPLEX * ab, INTEGER ldab, INTEGER * ipiv, INTEGER * info)
{
    INTEGER i, j, i2, i3, j2, j3, k2, jb, nb, ii, jj, jm, ip, jp, km, ju, kv, nw;
    COMPLEX temp;
    COMPLEX work13[4160], work31[4160];
    REAL Zero = 0.0, One = 1.0;

//KV is the number of superdiagonals in the factor U, allowing for
//fill-in
    kv = ku + kl;
//Test the input parameters.
    *info = 0;
    if (m < 0) {
	*info = -1;
    } else if (n < 0) {
	*info = -2;
    } else if (kl < 0) {
	*info = -3;
    } else if (ku < 0) {
	*info = -4;
    } else if (ldab < kl + kv + 1) {
	*info = -6;
    }
    if (*info != 0) {
	Mxerbla("Cgbtrf", -(*info));
	return;
    }
//Quick return if possible
    if (m == 0 || n == 0) {
	return;
    }
//Determine the block size for this environment
    nb = iMlaenv(1, "Cgbtrf", " ", m, n, kl, ku);
//The block size must not exceed the limit set by the size of the
//local arrays WORK13 and WORK31
    nb = min(nb, (INTEGER) 64);
    if (nb <= 1 || nb > kl) {
//Use unblocked code
	Cgbtf2(m, n, kl, ku, ab, ldab, ipiv, info);
    } else {
//Use blocked code
//Zero the superdiagonal elements of the work array WORK13
	for (j = 0; j < nb; j++) {
	    for (i = 0; i < j - 1; i++) {
		work13[i + j * 65 - 66] = Zero;
	    }
	}
//Zero the subdiagonal elements of the work array WORK31
	for (j = 0; j < nb; j++) {
	    for (i = j + 1; i <= nb; i++) {
		work31[i + j * 65 - 66] = Zero;
	    }
	}
//Gaussian elimination with partial pivoting
//Set fill-in elements in columns KU+2 to KV to zero
	for (j = ku + 2; j <= min(kv, n); j++) {
	    for (i = kv - j + 2; i <= kl; i++) {
		ab[i + j * ldab] = Zero;
	    }
	}
//JU is the index of the last column affected by the current
//stage of the factorization
	ju = 1;
	for (j = 0; j <= min(m, n); j = j + nb) {
	    jb = min(nb, min(m, n) - j + 1);
//The active part of the matrix is partitioned
//   A11   A12   A13
//   A21   A22   A23
//   A31   A32   A33
//Here A11, A21 and A31 denote the current block of JB columns
//which is about to be factorized. The number of rows in the
//partitioning are JB, I2, I3 respectively, and the numbers
//of columns are JB, J2, J3. The superdiagonal elements of A13
//and the subdiagonal elements of A31 lie outside the band.
	    i2 = min(kl - jb, m - j - jb + 1);
	    i3 = min(jb, m - j - kl + 1);
//J2 and J3 are computed after JU has been updated.
//Factorize the current block of JB columns
	    for (jj = j; jj <= j + jb - 1; jj++) {
//Set fill-in elements in column JJ+KV to zero
		if (jj + kv <= n) {
		    for (i = 0; i < kl; i++) {
			ab[i + (jj + kv) * ldab] = Zero;
		    }
		}
//Find pivot and test for singularity. KM is the number of
//subdiagonal elements in the current column.
		km = min(kl, m - jj);
		jp = iCamax(km + 1, &ab[kv + 1 + jj * ldab], 1);
		ipiv[jj] = jp + jj - j;
		if (ab[kv + jp + jj * ldab] != Zero) {
		    ju = max(ju, min(jj + ku + jp - 1, n));
		    if (jp != 1) {
//Apply interchange to columns J to J+JB-1
			if (jp + jj - 1 < j + kl) {
			    Cswap(jb, &ab[kv + 1 + jj - j + j * ldab], ldab - 1, &ab[kv + jp + jj - j + j * ldab], ldab - 1);
			} else {
//The interchange affects columns J to JJ-1 of A31
//which are stored in the work array WORK31
			    Cswap(jj - j, &ab[kv + 1 + jj - j + j * ldab], ldab - 1, &work31[jp + jj - j - kl - 1], 65);
			    Cswap(j + jb - jj, &ab[kv + 1 + jj * ldab], ldab - 1, &ab[kv + jp + jj * ldab], ldab - 1);
			}
		    }
//Compute multipliers
		    Cscal(km, One / ab[kv + 1 + jj * ldab], &ab[kv + 2 + jj * ldab], 1);
//Update trailing submatrix within the band and within
//the current block. JM is the index of the last column
//which needs to be updated.
		    jm = min(ju, j + jb - 1);
		    if (jm > jj) {
			Cgeru(km, jm - jj, (COMPLEX) - One, &ab[kv + 2 + jj * ldab], 1, &ab[kv + (jj + 1) * ldab], ldab - 1, &ab[kv + 1 + (jj + 1) * ldab], ldab - 1);
		    }
		} else {
//If pivot is zero, set INFO to the index of the pivot
//unless a zero pivot has already been found.
		    if (*info == 0) {
			*info = jj;
		    }
		}
//Copy current column of A31 into the work array WORK31
		nw = min(jj - j + 1, i3);
		if (nw > 0) {
		    Ccopy(nw, &ab[kv + kl + 1 - jj + j + jj * ldab], 1, &work31[(jj - j + 1) * 65 - 65], 1);
		}
	    }
	    if (j + jb <= n) {
//Apply the row interchanges to the other blocks.
		j2 = min(ju - j + 1, kv) - jb;
		j3 = max((INTEGER) 0, ju - j - kv + 1);
//Use ZLASWP to apply the row interchanges to A12, A22, and
//A32
		Claswp(j2, &ab[kv + 1 - jb + (j + jb) * ldab], ldab - 1, 1, jb, &ipiv[j], 1);
//Adjust the pivot indices.
		for (i = j; i <= j + jb - 1; i++) {
		    ipiv[i] = ipiv[i] + j - 1;
		}
//Apply the row interchanges to A13, A23, and A33
//columnwise.
		k2 = j - 1 + jb + j2;
		for (i = 0; i < j3; i++) {
		    jj = k2 + i;
		    for (ii = j + i - 1; ii <= j + jb - 1; ii++) {
			ip = ipiv[ii];
			if (ip != ii) {
			    temp = ab[kv + 1 + ii - jj + jj * ldab];
			    ab[kv + 1 + ii - jj + jj * ldab] = ab[kv + 1 + ip - jj + jj * ldab];
			    ab[kv + 1 + ip - jj + jj * ldab] = temp;
			}
		    }
		}
//Update the relevant part of the trailing submatrix
		if (j2 > 0) {
//Update A12
		    Ctrsm("Left", "Lower", "No transpose", "Unit", jb, j2, One, &ab[kv + 1 + j * ldab], ldab - 1, &ab[kv + 1 - jb + (j + jb) * ldab], ldab - 1);
		    if (i2 > 0) {
//Update A22
			Cgemm("No transpose", "No transpose", i2, j2, jb,
			      (COMPLEX) - One, &ab[kv + 1 + jb + j * ldab], ldab - 1, &ab[kv + 1 - jb + (j + jb) * ldab], ldab - 1, One, &ab[kv + 1 + (j + jb) * ldab], ldab - 1);
		    }
		    if (i3 > 0) {
//Update A32
			Cgemm("No transpose", "No transpose", ldab - 1, j2, jb,
			      (COMPLEX) - One, work31, 65, &ab[kv + 1 - jb + (j + jb) * ldab], ldab - 1, One, &ab[kv + kl + 1 - jb + (j + jb) * ldab], ldab - 1);
		    }
		}
		if (j3 > 0) {
//Copy the lower triangle of A13 into the work array
//WORK13
		    for (jj = 0; jj <= j3; jj++) {
			for (ii = jj; ii <= jb; ii++) {
			    work13[ii + jj * 65 - 66] = ab[ii - jj + 1 + (jj + j + kv - 1) * ldab];
			}
		    }
//Update A13 in the work array
		    Ctrsm("Left", "Lower", "No transpose", "Unit", jb, j3, One, &ab[kv + 1 + j * ldab], ldab - 1, work13, 65);
		    if (i2 > 0) {
//Update A23
			Cgemm("No transpose", "No transpose", i2, j3, jb,
			      (COMPLEX) - One, &ab[kv + 1 + jb + j * ldab], ldab - 1, work13, 65, One, &ab[jb + 1 + (j + kv) * ldab], ldab - 1);
		    }
		    if (i3 > 0) {
//Update A33
			Cgemm("No transpose", "No transpose", ldab - 1, j3, jb, (COMPLEX) - One, work31, 65, work13, 65, (COMPLEX) One, &ab[kl + 1 + (j + kv) * ldab], ldab - 1);
		    }
//Copy the lower triangle of A13 back into place
		    for (jj = 0; jj <= j3; jj++) {
			for (ii = jj; ii <= jb; ii++) {
			    ab[ii - jj + 1 + (jj + j + kv - 1) * ldab] = work13[ii + jj * 65 - 66];
			}
		    }
		}
	    } else {
//Adjust the pivot indices.
		for (i = j; i <= j + jb - 1; i++) {
		    ipiv[i] = ipiv[i] + j - 1;
		}
	    }
//Partially undo the interchanges in the current block to
//restore the upper triangular form of A31 and copy the upper
//triangle of A31 back into place
	    for (jj = j + jb - 1; jj >= j; jj--) {
		jp = ipiv[jj] - jj + 1;
		if (jp != 1) {
//Apply interchange to columns J to JJ-1
		    if (jp + jj - 1 < j + kl) {
//The interchange does not affect A31
			Cswap(jj - j, &ab[kv + 1 + jj - j + j * ldab], ldab - 1, &ab[kv + jp + jj - j + j * ldab], ldab - 1);
		    } else {
//The interchange does affect A31
			Cswap(jj - j, &ab[kv + 1 + jj - j + j * ldab], ldab - 1, &work31[jp + jj - j - kl - 1], 65);
		    }
		}
//Copy the current column of A31 back into place
		nw = min(i3, jj - j + 1);
		if (nw > 0) {
		    Ccopy(nw, &work31[(jj - j + 1) * 65 - 65], 1, &ab[kv + kl + 1 - jj + j + jj * ldab], 1);
		}
	    }
	}
    }
    return;
}
