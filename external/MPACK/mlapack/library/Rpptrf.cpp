/*
 * Copyright (c) 2008-2010
 *      Nakata, Maho
 *      All rights reserved.
 *
 *  $Id: Rpptrf.cpp,v 1.8 2010/08/07 04:48:33 nakatamaho Exp $ 
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

void Rpptrf(const char *uplo, INTEGER n, REAL * ap, INTEGER * info)
{
    INTEGER j, jc, jj;
    REAL ajj, mtemp;
    REAL One = 1.0, Zero = 0.0;
    INTEGER upper;

    *info = 0;
    upper = Mlsame(uplo, "U");
    if (!upper && !Mlsame(uplo, "L")) {
	*info = -1;
    } else if (n < 0) {
	*info = -2;
    }
    if (*info != 0) {
	Mxerbla("Rpptrf", -(*info));
	return;
    }
//Quick return if possible
    if (n == 0)
	return;
    if (upper) {

//Compute the Cholesky factorization A = U'*U.
	jj = 0;
	for (j = 0; j < n; j++) {
	    jc = jj + 1;
	    jj += j;
//Compute elements 1:J-1 of column J.
	    if (j > 1) {
		Rtpsv("Upper", "Transpose", "Non-unit", j - 1, &ap[0], &ap[jc], 1);
	    }
// Compute U(J,J) and test for non-positive-definiteness.
	    ajj = ap[jj] - Rdot(j - 1, &ap[jc], 1, &ap[jc], 1);
	    if (ajj <= Zero) {
		ap[jj] = ajj;
		*info = j;
		return;
	    }
	    ap[jj] = sqrt(ajj);
	}
    } else {
// Compute the Cholesky factorization A = L*L'.
	jj = 0;
	for (j = 0; j < n; j++) {
//Compute L(J,J) and test for non-positive-definiteness.
	    ajj = ap[jj];
	    if (ajj <= Zero) {
		ap[jj] = ajj;
		*info = j;
		return;
	    }
	    ajj = sqrt(ajj);
	    ap[jj] = ajj;

//Compute elements J+1:N of column J and update the trailing
//submatrix.

	    if (j < n) {
		mtemp = One / ajj;
		Rscal(n - j, mtemp, &ap[jj + 1], 1);
		Rspr("Lower", n - j, -One, &ap[jj + 1], 1, &ap[jj + n - j + 1]);
		jj = jj + n - j + 1;
	    }
	}
    }
    return;
}
