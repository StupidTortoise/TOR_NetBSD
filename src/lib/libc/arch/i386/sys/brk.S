/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
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
 *
 *	from: @(#)brk.s	5.2 (Berkeley) 12/17/90
 *	$Id: brk.S,v 1.8 1994/01/27 15:08:03 mycroft Exp $
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.text
	.asciz "$Id: brk.S,v 1.8 1994/01/27 15:08:03 mycroft Exp $"
#endif /* SYSLIBC_SCCS and not lint */

#include "SYS.h"

	.globl	_end
	.globl	minbrk
	.globl	curbrk

	.data
minbrk:	.long	_end
	.text

ENTRY(brk)
#ifdef PIC
	movl	4(%esp),%ecx
	PIC_PROLOGUE
	movl	PIC_GOT(minbrk),%edx
	PIC_EPILOGUE
	cmpl	%ecx,(%edx)
	jl	1f
	movl	(%edx),%ecx
	movl	%ecx,4(%esp)
1:
	movl	$(SYS_break),%eax
	LCALL(7,0)
	jc	err
	PIC_PROLOGUE
	movl	PIC_GOT(curbrk),%edx	# set up GOT addressing
	PIC_EPILOGUE
	xorl	%eax,%eax
	movl	%ecx,(%edx)
	ret
err:
	jmp	PIC_PLT(cerror)

#else

	movl	4(%esp),%ecx
	cmpl	%ecx,minbrk
	jl	1f
	movl	minbrk,%ecx
	movl	%ecx,4(%esp)
1:
	movl	$(SYS_break),%eax
	LCALL(7,0)
	jc	err
	xorl	%eax,%eax
	movl	%ecx,curbrk
	ret
err:
	jmp	cerror
#endif
