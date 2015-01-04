/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Edward Wang at The University of California, Berkeley.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
/*static char sccsid[] = "from: @(#)wwmisc.c	3.14 (Berkeley) 6/6/90";*/
static char rcsid[] = "$Id: wwmisc.c,v 1.2 1993/08/01 18:02:45 mycroft Exp $";
#endif /* not lint */

#include "ww.h"
#include "tt.h"
#include "char.h"

/*
 * Sufficient but not necessary test for total visibility.
 */
wwvisible(w)
register struct ww *w;
{
	register i;
	register nvis = 0;

	for (i = w->ww_i.t; i < w->ww_i.b; i++)
		nvis += w->ww_nvis[i];
	if (w->ww_hascursor
	    && w->ww_cur.r >= w->ww_i.t && w->ww_cur.r < w->ww_i.b
	    && w->ww_cur.c >= w->ww_i.l && w->ww_cur.c < w->ww_i.r
	    && wwsmap[w->ww_cur.r][w->ww_cur.c] == w->ww_index)
		nvis++;
	return nvis == w->ww_i.nr * w->ww_i.nc;
}

wwbell()
{
	ttputc(ctrl('g'));
}
