/*	$NetBSD: kern_exec.c,v 1.55 1994/06/29 06:32:24 cgd Exp $	*/

/*-
 * Copyright (C) 1993, 1994 Christopher G. Demetriou
 * Copyright (C) 1992 Wolfgang Solfrank.
 * Copyright (C) 1992 TooLs GmbH.
 * All rights reserved.
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
 *	This product includes software developed by TooLs GmbH.
 * 4. The name of TooLs GmbH may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY TOOLS GMBH ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TOOLS GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/acct.h>
#include <sys/exec.h>
#include <sys/ktrace.h>
#include <sys/resourcevar.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/signalvar.h>
#include <sys/stat.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>

#include <machine/cpu.h>
#include <machine/reg.h>

#ifdef COPY_SIGCODE
extern char sigcode[], esigcode[];

#define	szsigcode	(esigcode - sigcode)
#else
#define	szsigcode	0
#endif

/*
 * check exec:
 * given an "executable" described in the exec package's namei info,
 * see what we can do with it.
 *
 * ON ENTRY:
 *	exec package with appropriate namei info
 *	proc pointer of exec'ing proc
 *	NO SELF-LOCKED VNODES
 *
 * ON EXIT:
 *	error:	nothing held, etc.  exec header still allocated.
 *	ok:	filled exec package, one locked vnode.
 *
 * EXEC SWITCH ENTRY:
 * 	Locked vnode to check, exec package, proc.
 *
 * EXEC SWITCH EXIT:
 *	ok:	return 0, filled exec package, one locked vnode.
 *	error:	destructive:
 *			everything deallocated execept exec header.
 *		non-descructive:
 *			error code, locked vnode, exec header unmodified
 */
int
check_exec(p, epp)
	struct proc *p;
	struct exec_package *epp;
{
	int error, i;
	struct vnode *vp;
	char *cp, *ep, *name;
	struct nameidata *ndp;
	int resid;

	ndp = epp->ep_ndp;
	ndp->ni_cnd.cn_nameiop = LOOKUP;
	ndp->ni_cnd.cn_flags = FOLLOW | LOCKLEAF | SAVENAME;
	/* first get the vnode */
	if (error = namei(ndp))
		return error;
	epp->ep_vp = vp = ndp->ni_vp;

	/* check for regular file */
	if (vp->v_type != VREG) {
		error = EACCES;
		goto bad1;
	}

	/* get attributes */
	if (error = VOP_GETATTR(vp, epp->ep_vap, p->p_ucred, p))
		goto bad1;

	/* Check mount point */
	if (vp->v_mount->mnt_flag & MNT_NOEXEC) {
		error = EACCES;
		goto bad1;
	}
	if ((vp->v_mount->mnt_flag & MNT_NOSUID) || (p->p_flag & P_TRACED))
		epp->ep_vap->va_mode &= ~(VSUID | VSGID);

	/* check access.  for root we have to see if any exec bit on */
	if (error = VOP_ACCESS(vp, VEXEC, p->p_ucred, p))
		goto bad1;
	if ((epp->ep_vap->va_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) == 0) {
		error = EACCES;
		goto bad1;
	}

	/* try to open it */
	if (error = VOP_OPEN(vp, FREAD, p->p_ucred, p))
		goto bad1;

	/* now we have the file, get the exec header */
	if (error = vn_rdwr(UIO_READ, vp, epp->ep_hdr, epp->ep_hdrlen, 0,
	    UIO_SYSSPACE, IO_NODELOCKED, p->p_ucred, &resid, p))
		goto bad2;
	epp->ep_hdrvalid = epp->ep_hdrlen - resid;

	/*
	 * set up the vmcmds for creation of the process
	 * address space
	 */
	error = ENOEXEC;
	for (i = 0; i < nexecs && error != 0; i++) {
		if (execsw[i].es_check != NULL)
			error = (*execsw[i].es_check)(p, epp);
		if (epp->ep_flags & EXEC_DESTR && error != 0)
			return error;
	}
	if (!error) {
		/* check that entry point is sane */
		if (epp->ep_entry > VM_MAXUSER_ADDRESS)
			error = ENOEXEC;

		/* check limits */
		if ((epp->ep_tsize > MAXTSIZ) ||
		    (epp->ep_dsize > p->p_rlimit[RLIMIT_DATA].rlim_cur))
			error = ENOMEM;

		if (!error)
			return (0);
	}

	/*
	 * free any vmspace-creation commands,
	 * and release their references
	 */
	kill_vmcmds(&epp->ep_vmcmds);

bad2:
	/*
	 * unlock and close the vnode, restore the old one, free the
	 * pathname buf, and punt.
	 */
	VOP_UNLOCK(vp);
	vn_close(vp, FREAD, p->p_ucred, p);
	FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);
	return error;

bad1:
	/*
	 * free the namei pathname buffer, and put the vnode
	 * (which we don't yet have open).
	 */
	FREE(ndp->ni_cnd.cn_pnbuf, M_NAMEI);
	vput(vp);
	return error;
}

/*
 * exec system call
 */
/* ARGSUSED */
execve(p, uap, retval)
	register struct proc *p;
	register struct execve_args *uap;
	int *retval;
{
	int error, i;
	struct exec_package pack;
	struct nameidata nid;
	struct vattr attr;
	struct ucred *cred = p->p_ucred;
	char *argp;
	char **cpp, *dp, *sp, *np;
	int argc, envc, len;
	char *stack;
	struct ps_strings arginfo;
	struct vmspace *vm = p->p_vmspace;
	char **tmpfap;

	/*
	 * figure out the maximum size of an exec header, if necessary.
	 * XXX should be able to keep LKM code from modifying exec switch
	 * when we're still using it, but...
	 */
	if (exec_maxhdrsz == 0) {
		for (i = 0; i < nexecs; i++)
			if (execsw[i].es_check != NULL
			    && execsw[i].es_hdrsz > exec_maxhdrsz)
				exec_maxhdrsz = execsw[i].es_hdrsz;
	}

	/* init the namei data to point the file user's program name */
	NDINIT(&nid, LOOKUP, NOFOLLOW, UIO_USERSPACE, uap->path, p);

	/*
	 * initialize the fields of the exec package.
	 */
	pack.ep_name = uap->path;
	MALLOC(pack.ep_hdr, void *, exec_maxhdrsz, M_EXEC, M_WAITOK);
	pack.ep_hdrlen = exec_maxhdrsz;
	pack.ep_hdrvalid = 0;
	pack.ep_ndp = &nid;
	pack.ep_setup = NULL;	/* assume no setup function */
	pack.ep_vmcmds.evs_cnt = 0;
	pack.ep_vmcmds.evs_used = 0;
	pack.ep_vap = &attr;
	pack.ep_emul = EMUL_NETBSD;
	pack.ep_flags = 0;

	/* see if we can run it. */
	if (error = check_exec(p, &pack))
		goto freehdr;

	/* XXX -- THE FOLLOWING SECTION NEEDS MAJOR CLEANUP */

	/* allocate an argument buffer */
	argp = (char *) kmem_alloc_wait(exec_map, NCARGS);
#ifdef DIAGNOSTIC
	if (argp == (vm_offset_t) 0)
		panic("execve: argp == NULL");
#endif
	dp = argp;
	argc = 0;

	/* copy the fake args list, if there's one, freeing it as we go */
	if (pack.ep_flags & EXEC_HASARGL) {
		tmpfap = pack.ep_fa;
		while (*tmpfap != NULL) {
			char *cp;

			cp = *tmpfap;
			while (*cp)
				*dp++ = *cp++;
			*dp++;

			FREE(*tmpfap, M_EXEC);
			tmpfap++; argc++;
		}
		FREE(pack.ep_fa, M_EXEC);
		pack.ep_flags &= ~EXEC_HASARGL;
	}

	/* Now get argv & environment */
	if (!(cpp = uap->argp)) {
		error = EINVAL;
		goto bad;
	}

	if (pack.ep_flags & EXEC_SKIPARG)
		cpp++;

	while (1) {
		len = argp + ARG_MAX - dp;
		if (error = copyin(cpp, &sp, sizeof(sp)))
			goto bad;
		if (!sp)
			break;
		if (error = copyinstr(sp, dp, len, (u_int *) & len)) {
			if (error == ENAMETOOLONG)
				error = E2BIG;
			goto bad;
		}
		dp += len;
		cpp++;
		argc++;
	}

	envc = 0;
	if (cpp = uap->envp) {	/* environment need not be there */
		while (1) {
			len = argp + ARG_MAX - dp;
			if (error = copyin(cpp, &sp, sizeof(sp)))
				goto bad;
			if (!sp)
				break;
			if (error = copyinstr(sp, dp, len, (u_int *) & len)) {
				if (error == ENAMETOOLONG)
					error = E2BIG;
				goto bad;
			}
			dp += len;
			cpp++;
			envc++;
		}
		dp = (char *) ALIGN(dp);
	}

	/* Now check if args & environ fit into new stack */
	len = ((argc + envc + 2) * sizeof(char *) + sizeof(int) +
	    dp + STACKGAPLEN + szsigcode + sizeof(struct ps_strings)) - argp;
	len = ALIGN(len);	/* make the stack "safely" aligned */

	if (len > pack.ep_ssize) { /* in effect, compare to initial limit */
		error = ENOMEM;
		goto bad;
	}

	/* adjust "active stack depth" for process VSZ */
	pack.ep_ssize = len;	/* maybe should go elsewhere, but... */

	/* Unmap old program */
#ifdef sparc
	kill_user_windows(p);		/* before stack addresses go away */
#endif
	/* Kill shared memory and unmap old program */
#ifdef SYSVSHM
	if (vm->vm_shm)
		shmexit(p);
#endif
	vm_deallocate(&vm->vm_map, VM_MIN_ADDRESS,
		VM_MAXUSER_ADDRESS - VM_MIN_ADDRESS);

	/* Now map address space */
	vm->vm_taddr = (char *) pack.ep_taddr;
	vm->vm_tsize = btoc(pack.ep_tsize);
	vm->vm_daddr = (char *) pack.ep_daddr;
	vm->vm_dsize = btoc(pack.ep_dsize);
	vm->vm_ssize = btoc(pack.ep_ssize);
	vm->vm_maxsaddr = (char *) pack.ep_maxsaddr;

	/* create the new process's VM space by running the vmcmds */
#ifdef DIAGNOSTIC
	if (pack.ep_vmcmds.evs_used == 0)
		panic("execve: no vmcmds");
#endif
	for (i = 0; i < pack.ep_vmcmds.evs_used && !error; i++) {
		struct exec_vmcmd *vcp;

		vcp = &pack.ep_vmcmds.evs_cmds[i];
		error = (*vcp->ev_proc)(p, vcp);
	}

	/* free the vmspace-creation commands, and release their references */
	kill_vmcmds(&pack.ep_vmcmds);

	/* if an error happened, deallocate and punt */
	if (error)
		goto exec_abort;

	/* remember information about the process */
	arginfo.ps_nargvstr = argc;
	arginfo.ps_nenvstr = envc;

	/* Now copy argc, args & environ to new stack */
	stack = (char *) (USRSTACK - len);
	cpp = (char **) stack;

	if (copyout(&argc, cpp++, sizeof(argc)))
		goto exec_abort;
	dp = (char *) (cpp + argc + envc + 2);

	/* XXX don't copy them out, remap them! */
	arginfo.ps_argvstr = dp; /* remember location of argv for later */
	for (sp = argp; --argc >= 0; sp += len, dp += len) {
		len = strlen(sp) + 1;
		if (copyout(&dp, cpp++, sizeof(dp))
		    || copyoutstr(sp, dp, len, 0))
			goto exec_abort;
	}
	np = 0;
	if (copyout(&np, cpp++, sizeof(np)))
		goto exec_abort;

	arginfo.ps_envstr = dp;	/* remember location of env for later */
	for (; --envc >= 0; sp += len, dp += len) {
		len = strlen(sp) + 1;
		if (copyout(&dp, cpp++, sizeof(dp))
		    || copyoutstr(sp, dp, len, 0))
			goto exec_abort;
	}
	if (copyout(&np, cpp, sizeof(np)))
		goto exec_abort;

	/* copy out the process's ps_strings structure */
	if (copyout(&arginfo, (char *) PS_STRINGS, sizeof(arginfo)))
		goto exec_abort;

#ifdef COPY_SIGCODE
	/* copy out the process's signal trapoline code */
	if (copyout((char *) sigcode, ((char *) PS_STRINGS) - szsigcode,
		szsigcode)) {
		goto exec_abort;
	}
#endif

	fdcloseexec(p);		/* handle close on exec */
	execsigs(p);		/* reset catched signals */

	/* set command name & other accounting info */
	len = min(nid.ni_cnd.cn_namelen, MAXCOMLEN);
	bcopy(nid.ni_cnd.cn_nameptr, p->p_comm, len);
	p->p_comm[len] = 0;
	p->p_acflag &= ~AFORK;

	/* record proc's vnode, for use by procfs and others */
        if (p->p_textvp)
                vrele(p->p_textvp);
	VREF(pack.ep_vp);
	p->p_textvp = pack.ep_vp;

	p->p_flag |= P_EXEC;
	if (p->p_flag & P_PPWAIT) {
		p->p_flag &= ~P_PPWAIT;
		wakeup((caddr_t) p->p_pptr);
	}

	/*
	 * deal with set[ug]id.
	 * MNT_NOEXEC and P_TRACED have already been used to disable s[ug]id.
	 */
	p->p_flag &= ~P_SUGID;
	if (((attr.va_mode & VSUID) != 0 &&
	    p->p_ucred->cr_uid != attr.va_uid)
	    || (attr.va_mode & VSGID) != 0 &&
	    p->p_ucred->cr_gid != attr.va_gid) {
		p->p_ucred = crcopy(cred);
#ifdef KTRACE
		/*
		 * If process is being ktraced, turn off - unless
		 * root set it.
		 */
		if (p->p_tracep && !(p->p_traceflag & KTRFAC_ROOT)) {
			vrele(p->p_tracep);
			p->p_tracep = NULL;
			p->p_traceflag = 0;
		}
#endif
		if (attr.va_mode & VSUID)
			p->p_ucred->cr_uid = attr.va_uid;
		if (attr.va_mode & VSGID)
			p->p_ucred->cr_gid = attr.va_gid;
		p->p_flag |= P_SUGID;
	}
	p->p_cred->p_svuid = p->p_ucred->cr_uid;
	p->p_cred->p_svgid = p->p_ucred->cr_gid;

	kmem_free_wakeup(exec_map, (vm_offset_t) argp, NCARGS);

	FREE(nid.ni_cnd.cn_pnbuf, M_NAMEI);
	VOP_CLOSE(pack.ep_vp, FREAD, cred, p);
	vput(pack.ep_vp);

	/* setup new registers and do misc. setup. */
	setregs(p, pack.ep_entry, (u_long) stack, retval);
	if (pack.ep_setup != NULL)
		 (*pack.ep_setup)(p, &pack);

	if (p->p_flag & P_TRACED)
		psignal(p, SIGTRAP);

	p->p_emul = pack.ep_emul;
	FREE(pack.ep_hdr, M_EXEC);
	return 0;

bad:
	/* free the vmspace-creation commands, and release their references */
	kill_vmcmds(&pack.ep_vmcmds);
	/* kill any opened file descriptor, if necessary */
	if (pack.ep_flags & EXEC_HASFD) {
		pack.ep_flags &= ~EXEC_HASFD;
		exec_closefd(p, pack.ep_fd);
	}
	/* close and put the exec'd file */
	VOP_CLOSE(pack.ep_vp, FREAD, cred, p);
	vput(pack.ep_vp);
	FREE(nid.ni_cnd.cn_pnbuf, M_NAMEI);
	kmem_free_wakeup(exec_map, (vm_offset_t) argp, NCARGS);

freehdr:
	FREE(pack.ep_hdr, M_EXEC);
	return error;

exec_abort:
	/*
	 * the old process doesn't exist anymore.  exit gracefully.
	 * get rid of the (new) address space we have created, if any, get rid
	 * of our namei data and vnode, and exit noting failure
	 */
	vm_deallocate(&vm->vm_map, VM_MIN_ADDRESS,
		VM_MAXUSER_ADDRESS - VM_MIN_ADDRESS);
	FREE(nid.ni_cnd.cn_pnbuf, M_NAMEI);
	VOP_CLOSE(pack.ep_vp, FREAD, cred, p);
	vput(pack.ep_vp);
	kmem_free_wakeup(exec_map, (vm_offset_t) argp, NCARGS);
	FREE(pack.ep_hdr, M_EXEC);
	exit1(p, W_EXITCODE(0, SIGABRT));
	exit1(p, -1);

	/* NOTREACHED */
	return 0;
}
