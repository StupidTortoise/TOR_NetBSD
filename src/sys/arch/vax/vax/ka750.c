/*-
 * Copyright (c) 1982, 1986, 1988 The Regents of the University of California.
 * Copyright (c) 1994 Ludd, University of Lule}, Sweden.
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
 *	from: @(#)ka750.c	7.4 (Berkeley) 5/9/91
 *      also from: @(#)autoconf.c  7.20 (Berkeley) 5/9/91
 *	$Id: ka750.c,v 1.1 1994/08/02 20:21:58 ragge Exp $
 */

/* All bugs are subject to removal without further notice */

#if VAX750

#include "sys/param.h"
#include "sys/types.h"
#include "vax/include/ka750.h"
#include "vax/include/nexus.h"
#include "vax/include/pte.h"
#include "vax/include/mtpr.h"
#include "vax/uba/ubavar.h"
#include "vax/uba/ubareg.h"

#include "mba.h"
#include "uba.h"

#include "vm/vm.h"
#include "vm/vm_kern.h"
#include "vax/include/pmap.h"

struct nexus *nexus;

conf_750(){
	extern int cpu_type, nmcr, nmba, numuba;
	int i,j,k,l,m,cardinfo,nexnum;
	void *tempaddr;
	union nexcsr nexcsr;
	extern caddr_t mcraddr[];

	printf("VAX 11/750, hardware rev %d, ucode rev %d\n",
		V750HARDW(cpu_type), V750UCODE(cpu_type));
/*
 * First we map up so that we have virtual I/O-adresses for each nexus.
 */
	phys_map=kmem_suballoc(kernel_map, &nexus, &tempaddr,
		(16*sizeof(struct nexus)), FALSE); 
		/* XXX 16==NNEXUS */
	pmap_map(nexus, 0xf20000, 0xf40000, VM_PROT_READ|VM_PROT_WRITE);
		/* XXX 0xf20000 == NEXBASE */
/*
 * Check out what type of nexus there is. On 750 nexus don't identify
 * themselves, so we check them for specific places. :-/
 */

	for(nexnum=0;nexnum<4;nexnum++){ /* First mcr at 0-3 */
		if(badaddr((caddr_t)&nexus[nexnum],4))continue;
		mcraddr[nmcr]=(caddr_t)&nexus[nexnum];
		printf("mcr%d at tr%d: mcr type ", nmcr++, nexnum);
	/* We will use this info for error reporting - later! */
                cardinfo=*((int *)(nexus+nexnum)+2);
                switch((cardinfo>>24)&3){
                case 0: printf("L0011 ");
                        break;
                case 1: printf("L0016 ");
                        m=cardinfo&0xaaaa;
                        for(k=l=0;k<16;k++){if(m&1)l++;m>>=1;}
                        printf("with %d M8750",l);
                        break;
                case 3: printf("L0022 ");
                        m=cardinfo&0x5555;
                        for(k=l=0;k<16;k++){if(m&1)l++;m>>=1;}
                        printf("with %d M7199",l);
                        m=cardinfo&0xaaaa;
                        if(m){
                        for(k=l=0;k<16;k++){if(m&1)l++;m>>=1;}
                        printf(" and %d M8750",l);
                        }
                        break;
                }
                printf("\n");
	}

	for(nexnum=4;nexnum<8;nexnum++){ /* Check mba at 4-7 */
		if(badaddr((caddr_t)&nexus[nexnum],4)) continue;
		printf("mba%d at tr%d", nmba, nexnum);
		if(nmba>=NMBA){
			printf(" not configured.\n");
		} else {
			printf("\n");
#if NMBA > 0
			mbafind(nexnum,&nexus[nexnum]);
#endif
		}
		nmba++;
	}

	for(nexnum=8;nexnum<10;nexnum++){
		if(badaddr((caddr_t)&nexus[nexnum],4)) continue;
                printf("uba%d at tr%d", numuba, nexnum);
                if(numuba>=NUBA){
                        printf(" not configured.\n");
                } else {
                        printf("\n");
#if NUBA > 0
                        uba750(nexnum,&nexus[nexnum]);
#endif
                }
                numuba++;
        }

	for(nexnum=10;nexnum<16;nexnum++){
		if(badaddr((caddr_t)&nexus[nexnum],4)) continue;
		nexcsr=nexus[nexnum].nexcsr;
		switch(nexcsr.nex_type){

                case NEX_UBA0:
                case NEX_UBA1:
                case NEX_UBA2:
                case NEX_UBA3:
                        printf("uba%d at tr%d ", numuba, nexnum);
			if(nmba>=NUBA){
				printf(" not configured.\n");
			}
			break;

		default:
			printf("nexus type %x", nexcsr.nex_type);
unsupp:
                        printf(" unsupported (at tr %d)\n", nexnum);
                        continue;
		}
	}
/*
 * Enable memory error reporting.
 */
	ka750_memenable();
}

clock_750(){
	int i;
/*
 * It's time to start clocks in system...
 */
	i=~10000; /* Complement of 10000 milliseconds */
	mtpr(i,PR_NICR); /* Load in count register */
	mtpr(0x51,PR_ICCS); /* Start clock and enable interrupt */
	if(mfpr(PR_TODR)){
		/* todr running */
		return 0;
	} else {
		/* Start TODR register. */
		mtpr(1,PR_TODR);
		return 1;
	}

}

#if NMBA < 1
/*
 * Dummy routine; should never be called. :)
 * Should also be somewhere else, but it doesn't matter right now :)
 */
mbainterrupt(){return;}
#endif

#if NUBA > 0

uba750(int nexnum,caddr_t nexaddr){
	extern struct uba_hd uba_hd[];
	extern int numuba;
	struct uba_regs *ubar=(struct uba_regs *)nexaddr;
	void *tempaddr;
	unsigned int i;

	struct uba_hd *uhp = &uba_hd[numuba];

	uhp->uh_mr = ubar->uba_map;
	uhp->uh_type = DW750;
	uhp->uh_uba = (void*)nexaddr;
	uhp->uh_physuba = (void*)0xf20000+nexnum*0x2000;
	uhp->uh_memsize = UBAPAGES;
	uhp->uh_mem = Tumem(numuba);
        uhp->uh_iopage = Tumem(numuba) + (uhp->uh_memsize * NBPG);

/*
 * First we map up so that we have virtual I/O-adresses for each nexus.
 */
/*        phys_map=kmem_suballoc(kernel_map, &Numem, &tempaddr,
		(UBAPAGES+UBAIOPAGES)*NBPG, FALSE);
        pmap_map(Numem, UMEM750(numuba), UMEM750(numuba)+
		(UBAPAGES+UBAIOPAGES)*NBPG, VM_PROT_READ|VM_PROT_WRITE);
*/
	ioaccess(UMEM750(numuba), UMEMmap[numuba], (UBAPAGES+UBAIOPAGES)*NBPG);

        unifind(uhp, UMEM750(numuba) + (uhp->uh_memsize * NBPG));
}
#else
ubainterrupt(){return;}
#endif

/*
 * 750-specific code.
 */


#include "sys/param.h"

/* #include "mem.h" */
#include "../include/mtpr.h"

extern caddr_t mcraddr[];

struct	mcr750 {
	int	mc_err;			/* error bits */
	int	mc_inh;			/* inhibit crd */
};

#define	M750_ICRD	0x10000000	/* inhibit crd interrupts, in [1] */
#define	M750_UNCORR	0xc0000000	/* uncorrectable error, in [0] */
#define	M750_CORERR	0x20000000	/* correctable error, in [0] */

#define	M750_INH(mcr)	((mcr)->mc_inh = 0)
#define	M750_ENA(mcr)	((mcr)->mc_err = (M750_UNCORR|M750_CORERR), \
			 (mcr)->mc_inh = M750_ICRD)
#define	M750_ERR(mcr)	((mcr)->mc_err & (M750_UNCORR|M750_CORERR))

#define	M750_SYN(err)	((err) & 0x7f)
#define	M750_ADDR(err)	(((err) >> 9) & 0x7fff)

/* enable crd interrupts */
ka750_memenable()
{

	M750_ENA((struct mcr750 *)mcraddr[0]);
}

/* log crd errors */
ka750_memerr()
{
	register struct mcr750 *mcr = (struct mcr750 *)mcraddr[0];
	register int err;

	if (M750_ERR(mcr)) {
		err = mcr->mc_err;	/* careful with i/o space refs */
		printf("mcr0: %s", err & M750_UNCORR ?
		    "hard error" : "soft ecc");
		printf(" addr %x syn %x\n", M750_ADDR(err), M750_SYN(err));
		M750_INH(mcr);
	}
}

char mc750[]={"0","1","2","3","4","5","6","7","8","9","10","11","12","13",
	"14","15"};

struct mc750frame {
	int	mc5_bcnt;		/* byte count == 0x28 */
	int	mc5_summary;		/* summary parameter (as above) */
	int	mc5_va;			/* virtual address register */
	int	mc5_errpc;		/* error pc */
	int	mc5_mdr;
	int	mc5_svmode;		/* saved mode register */
	int	mc5_rdtimo;		/* read lock timeout */
	int	mc5_tbgpar;		/* tb group parity error register */
	int	mc5_cacherr;		/* cache error register */
	int	mc5_buserr;		/* bus error register */
	int	mc5_mcesr;		/* machine check status register */
	int	mc5_pc;			/* trapped pc */
	int	mc5_psl;		/* trapped psl */
};

#define MC750_TBERR	2		/* type code of cp tbuf par */
#define	MC750_TBPAR	4		/* tbuf par bit in mcesr */

ka750_mchk(cmcf)
	caddr_t cmcf;
{
	register struct mc750frame *mcf = (struct mc750frame *)cmcf;
	register int type = mcf->mc5_summary;
	int mcsr = mfpr(PR_MCSR);

	printf("machine check %x: %s%s\n", type, mc750[type&0xf],
	    (type&0xf0) ? " abort" : " fault"); 
	mtpr(PR_TBIA, 0);
	mtpr(PR_MCESR, 0xf);
	printf(
"\tva %x errpc %x mdr %x smr %x rdtimo %x tbgpar %x cacherr %x\n",
	    mcf->mc5_va, mcf->mc5_errpc, mcf->mc5_mdr, mcf->mc5_svmode,
	    mcf->mc5_rdtimo, mcf->mc5_tbgpar, mcf->mc5_cacherr);
	printf("\tbuserr %x mcesr %x pc %x psl %x mcsr %x\n",
	    mcf->mc5_buserr, mcf->mc5_mcesr, mcf->mc5_pc, mcf->mc5_psl,
	    mcsr);
	if (type == MC750_TBERR && (mcf->mc5_mcesr&0xe) == MC750_TBPAR) {
		printf("tbuf par: flushing and returning\n");
		return (0);
	}
	return (-1);
}
#endif
