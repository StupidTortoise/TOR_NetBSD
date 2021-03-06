/*
 *	$Id: grf_rh.c,v 1.2 1994/06/15 19:06:00 chopps Exp $
 */

#include "grfrh.h"
#if NGRFRH > 0

/*
 * Graphics routines for the Retina BLT Z3 board,
 * using the NCR 77C32BLT VGA controller.
*/

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <machine/cpu.h>
#include <amiga/amiga/device.h>
#include <amiga/dev/grfioctl.h>
#include <amiga/dev/grfvar.h>
#include <amiga/dev/grf_rhreg.h>
#include <amiga/dev/zthreebusvar.h>

int rh_mondefok __P((struct MonDef *));

u_short CompFQ __P((u_int fq));
int rh_load_mon __P((struct grf_softc *gp, struct MonDef *md));
int rh_getvmode __P((struct grf_softc *gp, struct grfvideo_mode *vm));
int rh_setvmode __P((struct grf_softc *gp, unsigned int mode, int txtonly));


extern unsigned char kernel_font_8x8_width, kernel_font_8x8_height;
extern unsigned char kernel_font_8x8_lo, kernel_font_8x8_hi;
extern unsigned char kernel_font_8x8[];
#ifdef KFONT_8X11
extern unsigned char kernel_font_8x11_width, kernel_font_8x11_height;
extern unsigned char kernel_font_8x11_lo, kernel_font_8x11_hi;
extern unsigned char kernel_font_8x11[];
#endif


/*
 * NOTE: this driver for the MacroSystem Retina board was only possible,
 *       because MacroSystem provided information about the pecularities
 *       of the board. THANKS! Competition in Europe among gfx board
 *       manufacturers is rather tough, so Lutz Vieweg, who wrote the
 *       initial driver, has made an agreement with MS not to document
 *       the driver source (see also his Copyright disclaimer below).
 *       -> ALL comments after
 *       -> "/* -------------- START OF CODE -------------- * /"
 *       -> have been added by myself (mw) from studying the publically
 *       -> available "NCR 77C32BLT" Data Manual
 *
 *       Lutz' original driver source (without any of my comments) is
 *       available on request.
 */

/* This code offers low-level routines to access the Retina BLT Z3
 * graphics-board manufactured by MS MacroSystem GmbH from within NetBSD
 * for the Amiga.  * No warranties for any kind of function at all - this
 * code may crash your hardware and scratch your harddisk.  Use at your
 * own risk.  Freely distributable.
 *
 * Written by Lutz Vieweg 02/94
 *
 * Thanks to MacroSystem for providing me with the neccessary information
 * to create theese routines. The sparse documentation of this code
 * results from the agreements between MS and me.
 */



#define MDF_DBL 1
#define MDF_LACE 2
#define MDF_CLKDIV2 4


/* -------------- START OF CODE -------------- */

/* Convert big-endian long into little-endian long. */

#define M2I(val)                                                     \
	asm volatile (" rorw #8,%0   ;                               \
	                swap %0      ;                               \
	                rorw #8,%0   ; " : "=d" (val) : "0" (val));

#define M2INS(val)                                                   \
	asm volatile (" rorw #8,%0   ;                               \
	                swap %0      ;                               \
	                rorw #8,%0   ;                               \
 			swap %0	     ; " : "=d" (val) : "0" (val));

#define ACM_OFFSET	(0x00b00000)
#define LM_OFFSET	(0x00c00000)

static unsigned char optab[] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0
};
static char optabs[] = {
	   0,   -1,   -1,   -1,   -1,    0,   -1,   -1,
	  -1,   -1,    0,   -1,   -1,   -1,   -1,    0
};

void
RZ3DisableHWC(gp)
	struct grf_softc *gp;
{
	volatile void *ba = gp->g_regkva;

	WSeq(ba, SEQ_ID_CURSOR_Y_INDEX, 0x00);
}

void
RZ3SetupHWC(gp, col1, col2, hsx, hsy, data)
	struct grf_softc *gp;
	unsigned char col1;
	unsigned col2;
	unsigned char hsx;
	unsigned char hsy;
	const unsigned long *data;
{
	volatile unsigned char *ba = gp->g_regkva;
	unsigned long *c = (unsigned long *)(ba + LM_OFFSET + HWC_MEM_OFF);
	const unsigned long *s = data;
	struct MonDef *MonitorDef = (struct MonDef *) gp->g_data;
	short x = (HWC_MEM_SIZE / (4*4)) - 1;
	/* copy only, if there is a data pointer. */
	if (data) do {
		*c++ = *s++;
		*c++ = *s++;
		*c++ = *s++;
		*c++ = *s++;
	} while (x-- > 0);

	WSeq(ba, SEQ_ID_CURSOR_COLOR1, col1);
	WSeq(ba, SEQ_ID_CURSOR_COLOR0, col2);
	if (MonitorDef->DEP < 16)
		WSeq(ba, SEQ_ID_CURSOR_CONTROL, 0x85);
	else
		WSeq(ba, SEQ_ID_CURSOR_CONTROL, 0xa5);
	WSeq(ba, SEQ_ID_CURSOR_X_LOC_HI, 0x00);
	WSeq(ba, SEQ_ID_CURSOR_X_LOC_LO, 0x00);
	WSeq(ba, SEQ_ID_CURSOR_Y_LOC_HI, 0x00);
	WSeq(ba, SEQ_ID_CURSOR_Y_LOC_LO, 0x00);
	WSeq(ba, SEQ_ID_CURSOR_X_INDEX, hsx);
	WSeq(ba, SEQ_ID_CURSOR_Y_INDEX, hsy);
	WSeq(ba, SEQ_ID_CURSOR_STORE_HI, 0x00);
	WSeq(ba, SEQ_ID_CURSOR_STORE_LO,  ((HWC_MEM_OFF / 4) & 0x0000f));
	WSeq(ba, SEQ_ID_CURSOR_ST_OFF_HI, (((HWC_MEM_OFF / 4) & 0xff000) >> 12));
	WSeq(ba, SEQ_ID_CURSOR_ST_OFF_LO, (((HWC_MEM_OFF / 4) & 0x00ff0) >>  4));
	WSeq(ba, SEQ_ID_CURSOR_PIXELMASK, 0xff);
}

void
RZ3AlphaErase (gp, xd, yd, w, h)
	struct grf_softc *gp;
	unsigned short xd;
	unsigned short yd;
	unsigned short  w;
	unsigned short  h;
{
	const struct MonDef * md = (struct MonDef *) gp->g_data;
	RZ3AlphaCopy(gp, xd, yd+md->TY, xd, yd, w, h);
}

void
RZ3AlphaCopy (gp, xs, ys, xd, yd, w, h)
	struct grf_softc *gp;
	unsigned short xs;
	unsigned short ys;
	unsigned short xd;
	unsigned short yd;
	unsigned short  w;
	unsigned short  h;
{
	volatile unsigned char *ba = gp->g_regkva;
	const struct MonDef *md = (struct MonDef *) gp->g_data;
	volatile unsigned long *acm = (unsigned long *) (ba + ACM_OFFSET);
	unsigned short mod;

	xs *= 4;
	ys *= 4;
	xd *= 4;
	yd *= 4;
	w  *= 4;

	{
		/* anyone got Windoze GDI opcodes handy?... */
		unsigned long tmp = 0x0000ca00;
		*(acm + ACM_RASTEROP_ROTATION/4) = tmp;
	}

	mod = 0xc0c2;

	{
		unsigned long pat = 8 * PAT_MEM_OFF;
		unsigned long dst = 8 * (xd + yd * md->TX);

		unsigned long src = 8 * (xs + ys * md->TX);

		if (xd > xs) {
			mod &= ~0x8000;
			src += 8 * (w - 1);
			dst += 8 * (w - 1);
			pat += 8 * 2;
		}
		if (yd > ys) {
			mod &= ~0x4000;
			src += 8 * (h - 1) * md->TX * 4;
			dst += 8 * (h - 1) * md->TX * 4;
			pat += 8 * 4;
		}

		M2I(src);
		*(acm + ACM_SOURCE/4) = src;

		M2I(pat);
		*(acm + ACM_PATTERN/4) = pat;

		M2I(dst);
		*(acm + ACM_DESTINATION/4) = dst;
	}
	{

		unsigned long tmp = mod << 16;
		*(acm + ACM_CONTROL/4) = tmp;
	}
	{

		unsigned long tmp  = w | (h << 16);
		M2I(tmp);
		*(acm + ACM_BITMAP_DIMENSION/4) = tmp;
	}

	*(((volatile unsigned char *)acm) + ACM_START_STATUS) = 0x00;
	*(((volatile unsigned char *)acm) + ACM_START_STATUS) = 0x01;

	while ((*(((volatile unsigned char *)acm) +
	    (ACM_START_STATUS + 2)) & 1) == 0);
}

void
RZ3BitBlit (gp, gbb)
	struct grf_softc *gp;
	struct grf_bitblt * gbb;
{
	volatile unsigned char *ba = gp->g_regkva;
	volatile unsigned char *lm = ba + LM_OFFSET;
	volatile unsigned long *acm = (unsigned long *) (ba + ACM_OFFSET);
	const struct MonDef *md = (struct MonDef *) gp->g_data;
	unsigned short mod;

	{
		unsigned long * pt = (unsigned long *) (lm + PAT_MEM_OFF);
		unsigned long tmp  = gbb->mask | ((unsigned long)gbb->mask << 16);
		*pt++ = tmp;
		*pt   = tmp;
	}

	{

		unsigned long tmp = optab[ gbb->op ] << 8;
		*(acm + ACM_RASTEROP_ROTATION/4) = tmp;
	}

	mod = 0xc0c2;

	{
		unsigned long pat = 8 * PAT_MEM_OFF;
		unsigned long dst = 8 * (gbb->dst_x + gbb->dst_y * md->TX);

		if (optabs[gbb->op]) {
			unsigned long src = 8 * (gbb->src_x + gbb->src_y * md->TX);

			if (gbb->dst_x > gbb->src_x) {
				mod &= ~0x8000;
				src += 8 * (gbb->w - 1);
				dst += 8 * (gbb->w - 1);
				pat += 8 * 2;
			}
			if (gbb->dst_y > gbb->src_y) {
				mod &= ~0x4000;
				src += 8 * (gbb->h - 1) * md->TX;
				dst += 8 * (gbb->h - 1) * md->TX;
				pat += 8 * 4;
			}

			M2I(src);
			*(acm + ACM_SOURCE/4) = src;
		}

		M2I(pat);
		*(acm + ACM_PATTERN/4) = pat;

		M2I(dst);
		*(acm + ACM_DESTINATION/4) = dst;
	}
	{

		unsigned long tmp = mod << 16;
		*(acm + ACM_CONTROL/4) = tmp;
	}
	{
		unsigned long tmp  = gbb->w | (gbb->h << 16);
		M2I(tmp);
		*(acm + ACM_BITMAP_DIMENSION/4) = tmp;
	}

	*(((volatile unsigned char *)acm) + ACM_START_STATUS) = 0x00;
	*(((volatile unsigned char *)acm) + ACM_START_STATUS) = 0x01;

	while ((*(((volatile unsigned char *)acm) +
	    (ACM_START_STATUS + 2)) & 1) == 0);
}

void
RZ3BitBlit16 (gp, gbb)
	struct grf_softc *gp;
	struct grf_bitblt * gbb;
{
	volatile unsigned char *ba = gp->g_regkva;
	volatile unsigned char *lm = ba + LM_OFFSET;
	volatile unsigned long * acm = (unsigned long *) (ba + ACM_OFFSET);
	const struct MonDef * md = (struct MonDef *) gp->g_data;
	unsigned short mod;

	{
		unsigned long * pt = (unsigned long *) (lm + PAT_MEM_OFF);
		unsigned long tmp  = gbb->mask | ((unsigned long)gbb->mask << 16);
		*pt++ = tmp;
		*pt++ = tmp;
		*pt++ = tmp;
		*pt   = tmp;
	}

	{

		unsigned long tmp = optab[ gbb->op ] << 8;
		*(acm + ACM_RASTEROP_ROTATION/4) = tmp;
	}

	mod = 0xc0c2;

	{
		unsigned long pat = 8 * PAT_MEM_OFF;
		unsigned long dst = 8 * 2 * (gbb->dst_x + gbb->dst_y * md->TX);

		if (optabs[gbb->op]) {
			unsigned long src = 8 * 2 * (gbb->src_x + gbb->src_y * md->TX);

			if (gbb->dst_x > gbb->src_x) {
				mod &= ~0x8000;
				src += 8 * 2 * (gbb->w);
				dst += 8 * 2 * (gbb->w);
				pat += 8 * 2 * 2;
			}
			if (gbb->dst_y > gbb->src_y) {
				mod &= ~0x4000;
				src += 8 * 2 * (gbb->h - 1) * md->TX;
				dst += 8 * 2 * (gbb->h - 1) * md->TX;
				pat += 8 * 4 * 2;
			}

			M2I(src);
			*(acm + ACM_SOURCE/4) = src;
		}

		M2I(pat);
		*(acm + ACM_PATTERN/4) = pat;

		M2I(dst);
		*(acm + ACM_DESTINATION/4) = dst;
	}
	{

		unsigned long tmp = mod << 16;
		*(acm + ACM_CONTROL/4) = tmp;
	}
	{

		unsigned long tmp  = gbb->w | (gbb->h << 16);
		M2I(tmp);
		*(acm + ACM_BITMAP_DIMENSION/4) = tmp;
	}

	*(((volatile unsigned char *)acm) + ACM_START_STATUS) = 0x00;
	*(((volatile unsigned char *)acm) + ACM_START_STATUS) = 0x01;

	while ((*(((volatile unsigned char *)acm) +
	    (ACM_START_STATUS+ 2)) & 1) == 0);
}

void
RZ3SetCursorPos (gp, pos)
	struct grf_softc *gp;
	unsigned short pos;
{
	volatile unsigned char *ba = gp->g_regkva;

	WCrt(ba, CRT_ID_CURSOR_LOC_LOW, (unsigned char)pos);
	WCrt(ba, CRT_ID_CURSOR_LOC_HIGH, (unsigned char)(pos >> 8));

}

void
RZ3LoadPalette (gp, pal, firstcol, colors)
	struct grf_softc *gp;
	unsigned char * pal;
	unsigned char firstcol;
	unsigned char colors;
{
	volatile unsigned char *ba = gp->g_regkva;

	if (colors == 0)
		return;

	vgaw(ba, VDAC_ADDRESS_W, firstcol);

	{

		short x = colors-1;
		const unsigned char * col = pal;
		do {

			vgaw(ba, VDAC_DATA, (*col++ >> 2));
			vgaw(ba, VDAC_DATA, (*col++ >> 2));
			vgaw(ba, VDAC_DATA, (*col++ >> 2));

		} while (x-- > 0);

	}
}

void
RZ3SetPalette (gp, colornum, red, green, blue)
	struct grf_softc *gp;
	unsigned char colornum;
	unsigned char red, green, blue;
{
	volatile unsigned char *ba = gp->g_regkva;

	vgaw(ba, VDAC_ADDRESS_W, colornum);

	vgaw(ba, VDAC_DATA, (red >> 2));
	vgaw(ba, VDAC_DATA, (green >> 2));
	vgaw(ba, VDAC_DATA, (blue >> 2));

}

/* XXXXXXXXX !! */
static unsigned short xpan;
static unsigned short ypan;

void
RZ3SetPanning (gp, xoff, yoff)
	struct grf_softc *gp;
	unsigned short xoff, yoff;
{
	volatile unsigned char *ba = gp->g_regkva;
	const struct MonDef * md = (struct MonDef *) gp->g_data;
	unsigned long off;

	xpan = xoff;
	ypan = yoff;


	if (md->DEP > 8)
		xoff *= 2;

	vgar(ba, ACT_ADDRESS_RESET);
	WAttr(ba, ACT_ID_HOR_PEL_PANNING, (unsigned char)((xoff << 1) & 0x07));
	/* have the color lookup function normally again */
	vgaw(ba,  ACT_ADDRESS_W, 0x20);

	if (md->DEP == 8)
		off = ((yoff * md->TX)/ 4) + (xoff >> 2);
	else
		off = ((yoff * md->TX * 2)/ 4) + (xoff >> 2);

	WCrt(ba, CRT_ID_START_ADDR_LOW, ((unsigned char)off));

	off >>= 8;

	WCrt(ba, CRT_ID_START_ADDR_HIGH, ((unsigned char)off));

	off >>= 8;

	WCrt(ba, CRT_ID_EXT_START_ADDR,
	    ((RCrt(ba, CRT_ID_EXT_START_ADDR) & 0xf0) | (off & 0x0f)));


}

void
RZ3SetHWCloc (gp, x, y)
	struct grf_softc *gp;
	unsigned short x, y;
{
	volatile unsigned char *ba = gp->g_regkva;
	const struct MonDef *md = (struct MonDef *) gp->g_data;
	volatile unsigned char *acm = ba + ACM_OFFSET;

	if (x < xpan)
		RZ3SetPanning(gp, x, ypan);

	if (x >= (xpan+md->MW))
		RZ3SetPanning(gp, (1 + x - md->MW) , ypan);

	if (y < ypan)
		RZ3SetPanning(gp, xpan, y);

	if (y >= (ypan+md->MH))
		RZ3SetPanning(gp, xpan, (1 + y - md->MH));

	x -= xpan;
	y -= ypan;

	*(acm + (ACM_CURSOR_POSITION+0)) = x & 0xff;
	*(acm + (ACM_CURSOR_POSITION+1)) = x >> 8;
	*(acm + (ACM_CURSOR_POSITION+2)) = y & 0xff;
	*(acm + (ACM_CURSOR_POSITION+3)) = y >> 8;
}

u_short
CompFQ(fq)
	u_int fq;
{
 	/* yuck... this sure could need some explanation.. */

	unsigned long f = fq;
	long n2 = 3;
	long abw = 0x7fffffff;
	long n1 = 3;
	unsigned long m;
	unsigned short erg = 0;

	f *= 8;

	do {

		if (f <= 250000000)
			break;
		f /= 2;

	} while (n2-- > 0);

	if (n2 < 0)
		return(0);


	do {
	  	long tmp;

		f = fq;
		f >>= 3;
		f <<= n2;
		f >>= 7;

		m = (f * n1) / (14318180/1024);

		if (m > 129)
			break;

		tmp =  (((m * 14318180) >> n2) / n1) - fq;
		if (tmp < 0)
			tmp = -tmp;

		if (tmp < abw) {
			abw = tmp;
			erg = (((n2 << 5) | (n1-2)) << 8) | (m-2);
		}

	} while ( (++n1) <= 21);

	return(erg);
}

int
rh_mondefok(mdp)
	struct MonDef *mdp;
{
	switch(mdp->DEP) {
	case 8:
	case 16:
		return(1);
	case 4:
		if (mdp->FX == 4 || (mdp->FX >= 7 && mdp->FX <= 16))
			return(1);
		/*FALLTHROUGH*/
	default:
		return(0);
	}
}


int
rh_load_mon(gp, md)
	struct grf_softc *gp;
	struct MonDef *md;
{
	struct grfinfo *gi = &gp->g_display;
	volatile unsigned char *ba;
	volatile unsigned char *fb;
	short FW, clksel, HDE, VDE;
	unsigned short *c, z;
	const unsigned char *f;

	ba = gp->g_regkva;;
	fb = gp->g_fbkva;

	/* provide all needed information in grf device-independant
	 * locations */
	gp->g_data 		= (caddr_t) md;
	gi->gd_regaddr	 	= (caddr_t) kvtop (ba);
	gi->gd_regsize		= LM_OFFSET;
	gi->gd_fbaddr		= (caddr_t) kvtop (fb);
	gi->gd_fbsize		= MEMSIZE *1024*1024;
#ifdef BANKEDDEVPAGER
	/* we're not using banks NO MORE! */
	gi->gd_bank_size	= 0;
#endif
	gi->gd_colors		= 1 << md->DEP;
	gi->gd_planes		= md->DEP;

	if (md->DEP == 4) {
		gi->gd_fbwidth	= md->MW;
		gi->gd_fbheight	= md->MH;
		gi->gd_fbx	= 0;
		gi->gd_fby	= 0;
		gi->gd_dwidth	= md->TX * md->FX;
		gi->gd_dheight	= md->TY * md->FY;
		gi->gd_dx	= 0;
		gi->gd_dy	= 0;
	} else {
		gi->gd_fbwidth	= md->TX;
		gi->gd_fbheight	= md->TY;
		gi->gd_fbx	= 0;
		gi->gd_fby	= 0;
		gi->gd_dwidth	= md->MW;
		gi->gd_dheight	= md->MH;
		gi->gd_dx	= 0;
		gi->gd_dy	= 0;
	}

	FW =0;
	if (md->DEP == 4) {		/* XXX some text-mode! */
		switch (md->FX) {
		case 4:
			FW = 0;
			break;
		case 7:
			FW = 1;
			break;
		case 8:
			FW = 2;
			break;
		case 9:
			FW = 3;
			break;
		case 10:
			FW = 4;
			break;
		case 11:
			FW = 5;
			break;
		case 12:
			FW = 6;
			break;
		case 13:
			FW = 7;
			break;
		case 14:
			FW = 8;
			break;
		case 15:
			FW = 9;
			break;
		case 16:
			FW = 11;
			break;
		default:
			return(0);
			break;
		}
	}

	if (md->DEP == 4)
		HDE = (md->MW+md->FX-1)/md->FX;
	else if (md->DEP == 8)
		HDE = (md->MW+3)/4;
	else if (md->DEP == 16)
		HDE = (md->MW*2+3)/4;

	VDE = md->MH-1;

	clksel = 0;

	vgaw(ba, GREG_MISC_OUTPUT_W, 0xe3 | ((clksel & 3) * 0x04));
	vgaw(ba, GREG_FEATURE_CONTROL_W, 0x00);

	WSeq(ba, SEQ_ID_RESET, 0x00);
	WSeq(ba, SEQ_ID_RESET, 0x03);
	WSeq(ba, SEQ_ID_CLOCKING_MODE, 0x01 | ((md->FLG & MDF_CLKDIV2)/ MDF_CLKDIV2 * 8));
	WSeq(ba, SEQ_ID_MAP_MASK, 0x0f);
	WSeq(ba, SEQ_ID_CHAR_MAP_SELECT, 0x00);
	WSeq(ba, SEQ_ID_MEMORY_MODE, 0x06);
	WSeq(ba, SEQ_ID_RESET, 0x01);
	WSeq(ba, SEQ_ID_RESET, 0x03);

	WSeq(ba, SEQ_ID_EXTENDED_ENABLE, 0x05);
	WSeq(ba, SEQ_ID_CURSOR_CONTROL, 0x00);
	WSeq(ba, SEQ_ID_PRIM_HOST_OFF_HI, 0x00);
	WSeq(ba, SEQ_ID_PRIM_HOST_OFF_HI, 0x00);
	WSeq(ba, SEQ_ID_LINEAR_0, 0x4a);
	WSeq(ba, SEQ_ID_LINEAR_1, 0x00);

	WSeq(ba, SEQ_ID_SEC_HOST_OFF_HI, 0x00);
	WSeq(ba, SEQ_ID_SEC_HOST_OFF_LO, 0x00);
	WSeq(ba, SEQ_ID_EXTENDED_MEM_ENA, 0x3 | 0x4 | 0x10 | 0x40);
	WSeq(ba, SEQ_ID_EXT_CLOCK_MODE, 0x10 | (FW & 0x0f));
	WSeq(ba, SEQ_ID_EXT_VIDEO_ADDR, 0x03);
	if (md->DEP == 4) {
	  	/* 8bit pixel, no gfx byte path */
		WSeq(ba, SEQ_ID_EXT_PIXEL_CNTL, 0x00);
	} else if (md->DEP == 8) {
	  	/* 8bit pixel, gfx byte path */
		WSeq(ba, SEQ_ID_EXT_PIXEL_CNTL, 0x01);
	} else if (md->DEP == 16) {
	  	/* 16bit pixel, gfx byte path */
		WSeq(ba, SEQ_ID_EXT_PIXEL_CNTL, 0x11);
	}
	WSeq(ba, SEQ_ID_BUS_WIDTH_FEEDB, 0x04);
	WSeq(ba, SEQ_ID_COLOR_EXP_WFG, 0x01);
	WSeq(ba, SEQ_ID_COLOR_EXP_WBG, 0x00);
	WSeq(ba, SEQ_ID_EXT_RW_CONTROL, 0x00);
	WSeq(ba, SEQ_ID_MISC_FEATURE_SEL, (0x51 | (clksel & 8)));
	WSeq(ba, SEQ_ID_COLOR_KEY_CNTL, 0x40);
	WSeq(ba, SEQ_ID_COLOR_KEY_MATCH0, 0x00);
	WSeq(ba, SEQ_ID_COLOR_KEY_MATCH1, 0x00);
	WSeq(ba, SEQ_ID_COLOR_KEY_MATCH2, 0x00);
	WSeq(ba, SEQ_ID_CRC_CONTROL, 0x00);
	WSeq(ba, SEQ_ID_PERF_SELECT, 0x10);
	WSeq(ba, SEQ_ID_ACM_APERTURE_1, 0x00);
	WSeq(ba, SEQ_ID_ACM_APERTURE_2, 0x30);
	WSeq(ba, SEQ_ID_ACM_APERTURE_3, 0x00);
	WSeq(ba, SEQ_ID_MEMORY_MAP_CNTL, 0x07);

	WCrt(ba, CRT_ID_END_VER_RETR, (md->VSE & 0xf) | 0x20);
	WCrt(ba, CRT_ID_HOR_TOTAL, md->HT    & 0xff);
	WCrt(ba, CRT_ID_HOR_DISP_ENA_END, (HDE-1)   & 0xff);
	WCrt(ba, CRT_ID_START_HOR_BLANK, md->HBS   & 0xff);
	WCrt(ba, CRT_ID_END_HOR_BLANK, (md->HBE   & 0x1f) | 0x80);

	WCrt(ba, CRT_ID_START_HOR_RETR, md->HSS   & 0xff);
	WCrt(ba, CRT_ID_END_HOR_RETR,
	    (md->HSE & 0x1f)   |
	    ((md->HBE & 0x20)/ 0x20 * 0x80));
	WCrt(ba, CRT_ID_VER_TOTAL,  (md->VT  & 0xff));
	WCrt(ba, CRT_ID_OVERFLOW,
	    ((md->VSS & 0x200) / 0x200 * 0x80) |
	    ((VDE     & 0x200) / 0x200 * 0x40) |
	    ((md->VT  & 0x200) / 0x200 * 0x20) |
	    0x10                               |
	    ((md->VBS & 0x100) / 0x100 * 8)    |
	    ((md->VSS & 0x100) / 0x100 * 4)    |
	    ((VDE     & 0x100) / 0x100 * 2)    |
	    ((md->VT  & 0x100) / 0x100));
	WCrt(ba, CRT_ID_PRESET_ROW_SCAN, 0x00);

	if (md->DEP == 4) {
		WCrt(ba, CRT_ID_MAX_SCAN_LINE,
		    ((md->FLG & MDF_DBL)/ MDF_DBL * 0x80) |
		    0x40 |
		    ((md->VBS & 0x200)/0x200*0x20) |
		    ((md->FY-1) & 0x1f));
	} else {
		WCrt(ba, CRT_ID_MAX_SCAN_LINE,
		    ((md->FLG & MDF_DBL)/ MDF_DBL * 0x80) |
		    0x40 |
		    ((md->VBS & 0x200)/0x200*0x20) |
		    (0 & 0x1f));
	}

	/* I prefer "_" cursor to "block" cursor.. */
#if 1
	WCrt(ba, CRT_ID_CURSOR_START, (md->FY & 0x1f) - 2);
	WCrt(ba, CRT_ID_CURSOR_END, (md->FY & 0x1f) - 1);
#else
	WCrt(ba, CRT_ID_CURSOR_START, 0x00);
	WCrt(ba, CRT_ID_CURSOR_END, md->FY & 0x1f);
#endif

	WCrt(ba, CRT_ID_START_ADDR_HIGH, 0x00);
	WCrt(ba, CRT_ID_START_ADDR_LOW, 0x00);

	WCrt(ba, CRT_ID_CURSOR_LOC_HIGH, 0x00);
	WCrt(ba, CRT_ID_CURSOR_LOC_LOW, 0x00);

	WCrt(ba, CRT_ID_START_VER_RETR, md->VSS & 0xff);
	WCrt(ba, CRT_ID_END_VER_RETR, (md->VSE & 0xf) | 0x80 | 0x20);
	WCrt(ba, CRT_ID_VER_DISP_ENA_END, VDE  & 0xff);

	if (md->DEP == 4)
		WCrt(ba, CRT_ID_OFFSET, (HDE / 2) & 0xff);
	else if (md->DEP == 8)
		WCrt(ba, CRT_ID_OFFSET, (md->TX / 8) & 0xff);
	else
		WCrt(ba, CRT_ID_OFFSET, (md->TX / 4) & 0xff);

	WCrt(ba, CRT_ID_UNDERLINE_LOC, (md->FY-1) & 0x1f);
	WCrt(ba, CRT_ID_START_VER_BLANK, md->VBS & 0xff);
	WCrt(ba, CRT_ID_END_VER_BLANK, md->VBE & 0xff);
	WCrt(ba, CRT_ID_MODE_CONTROL, 0xe3);
	WCrt(ba, CRT_ID_LINE_COMPARE, 0xff);

	WCrt(ba, CRT_ID_EXT_HOR_TIMING1,
		    0 | 0x20                                    |
		    ((md->FLG & MDF_LACE)  / MDF_LACE   * 0x10) |
		    ((md->HT  & 0x100) / 0x100)                 |
		    (((HDE-1) & 0x100) / 0x100 * 2)             |
		    ((md->HBS & 0x100) / 0x100 * 4)             |
		    ((md->HSS & 0x100) / 0x100 * 8));

	if (md->DEP == 4)
		WCrt(ba, CRT_ID_EXT_START_ADDR,
		    (((HDE / 2) & 0x100)/0x100 * 16));
	else if (md->DEP == 8)
		WCrt(ba, CRT_ID_EXT_START_ADDR,
		    (((md->TX / 8) & 0x100)/0x100 * 16));
	else
		WCrt(ba, CRT_ID_EXT_START_ADDR,
		    (((md->TX / 4) & 0x100)/0x100 * 16));

	WCrt(ba, CRT_ID_EXT_HOR_TIMING2,
		    ((md->HT  & 0x200)/ 0x200)       |
	            (((HDE-1) & 0x200)/ 0x200 * 2  ) |
	            ((md->HBS & 0x200)/ 0x200 * 4  ) |
	            ((md->HSS & 0x200)/ 0x200 * 8  ) |
	            ((md->HBE & 0xc0) / 0x40  * 16 ) |
	            ((md->HSE & 0x60) / 0x20  * 64));

	WCrt(ba, CRT_ID_EXT_VER_TIMING,
		    ((md->VSE & 0x10) / 0x10  * 0x80  ) |
		    ((md->VBE & 0x300)/ 0x100 * 0x20  ) |
		    0x10                                |
		    ((md->VSS & 0x400)/ 0x400 * 8     ) |
		    ((md->VBS & 0x400)/ 0x400 * 4     ) |
		    ((VDE     & 0x400)/ 0x400 * 2     ) |
		    ((md->VT & 0x400)/ 0x400));
	WCrt(ba, CRT_ID_MONITOR_POWER, 0x00);

	{
		unsigned short tmp = CompFQ(md->FQ);
		WPLL(ba, 2   , tmp);
		tmp = CompFQ(MEMCLK);
		WPLL(ba,10   , tmp);
		WPLL(ba,14   , 0x22);
	}

	WGfx(ba, GCT_ID_SET_RESET, 0x00);
	WGfx(ba, GCT_ID_ENABLE_SET_RESET, 0x00);
	WGfx(ba, GCT_ID_COLOR_COMPARE, 0x00);
	WGfx(ba, GCT_ID_DATA_ROTATE, 0x00);
	WGfx(ba, GCT_ID_READ_MAP_SELECT, 0x00);
	WGfx(ba, GCT_ID_GRAPHICS_MODE, 0x00);
	if (md->DEP == 4)
		WGfx(ba, GCT_ID_MISC, 0x04);
	else
		WGfx(ba, GCT_ID_MISC, 0x05);
	WGfx(ba, GCT_ID_COLOR_XCARE, 0x0f);
	WGfx(ba, GCT_ID_BITMASK, 0xff);

	vgar(ba, ACT_ADDRESS_RESET);
	WAttr(ba, ACT_ID_PALETTE0 , 0x00);
	WAttr(ba, ACT_ID_PALETTE1 , 0x01);
	WAttr(ba, ACT_ID_PALETTE2 , 0x02);
	WAttr(ba, ACT_ID_PALETTE3 , 0x03);
	WAttr(ba, ACT_ID_PALETTE4 , 0x04);
	WAttr(ba, ACT_ID_PALETTE5 , 0x05);
	WAttr(ba, ACT_ID_PALETTE6 , 0x06);
	WAttr(ba, ACT_ID_PALETTE7 , 0x07);
	WAttr(ba, ACT_ID_PALETTE8 , 0x08);
	WAttr(ba, ACT_ID_PALETTE9 , 0x09);
	WAttr(ba, ACT_ID_PALETTE10, 0x0a);
	WAttr(ba, ACT_ID_PALETTE11, 0x0b);
	WAttr(ba, ACT_ID_PALETTE12, 0x0c);
	WAttr(ba, ACT_ID_PALETTE13, 0x0d);
	WAttr(ba, ACT_ID_PALETTE14, 0x0e);
	WAttr(ba, ACT_ID_PALETTE15, 0x0f);

	vgar(ba, ACT_ADDRESS_RESET);
	if (md->DEP == 4)
		WAttr(ba, ACT_ID_ATTR_MODE_CNTL, 0x08);
	else
		WAttr(ba, ACT_ID_ATTR_MODE_CNTL, 0x09);

	WAttr(ba, ACT_ID_OVERSCAN_COLOR, 0x00);
	WAttr(ba, ACT_ID_COLOR_PLANE_ENA, 0x0f);
	WAttr(ba, ACT_ID_HOR_PEL_PANNING, 0x00);
	WAttr(ba, ACT_ID_COLOR_SELECT, 0x00);

	vgar(ba, ACT_ADDRESS_RESET);
	vgaw(ba, ACT_ADDRESS_W, 0x20);

	vgaw(ba, VDAC_MASK, 0xff);
	if (md->DEP < 16)
	  	/* well... probably the PLL chip */
		vgaw(ba, 0x83c6, ((0 & 7) << 5));
	else if (md->DEP == 16)
	  	/* well... */
		vgaw(ba, 0x83c6, ((3 & 7) << 5));
	vgaw(ba, VDAC_ADDRESS_W, 0x00);

	if (md->DEP < 16) {
		short x = 256-17;
		unsigned char cl = 16;
		RZ3LoadPalette(gp, md->PAL, 0, 16);
		do {
			vgaw(ba, VDAC_DATA, (cl >> 2));
			vgaw(ba, VDAC_DATA, (cl >> 2));
			vgaw(ba, VDAC_DATA, (cl >> 2));
			cl++;
		} while (x-- > 0);
	}

	if (md->DEP == 4) {
		{
			struct grf_bitblt bb = {
				GRFBBOPset,
				0, 0,
				0, 0,
				md->TX*4, 2*md->TY,
				EMPTY_ALPHA
			};
			RZ3BitBlit(gp, &bb);
		}

		c = (unsigned short *)(ba + LM_OFFSET);
		c += 2 * md->FLo*32;
		c += 1;
		f = md->FData;
		for (z = md->FLo; z <= md->FHi; z++) {
			short y = md->FY-1;
			if (md->FX > 8){
				do {
					*c = *((const unsigned short *)f);
					c += 2;
					f += 2;
				} while (y-- > 0);
			} else {
				do {
					*c = (*f++) << 8;
					c += 2;
				} while (y-- > 0);
			}

			c += 2 * (32-md->FY);
		}
		{
			unsigned long * pt = (unsigned long *) (ba + LM_OFFSET + PAT_MEM_OFF);
			unsigned long tmp  = 0xffff0000;
			*pt++ = tmp;
			*pt = tmp;
		}

		WSeq(ba, SEQ_ID_MAP_MASK, 3);

		c = (unsigned short *)(ba + LM_OFFSET);
		c += (md->TX-6)*2;
		{
		  	/* it's show-time :-) */
			static unsigned short init_msg[6] = {
				0x520a, 0x450b, 0x540c, 0x490d, 0x4e0e, 0x410f
			};
			unsigned short * m = init_msg;
			short x = 5;
			do {
				*c = *m++;
				c += 2;
			} while (x-- > 0);
		}

		return(1);
	} else if (md->DEP == 8) {
		struct grf_bitblt bb = {
			GRFBBOPset,
			0, 0,
			0, 0,
			md->TX, md->TY,
			0x0000
		};
		WSeq(ba, SEQ_ID_MAP_MASK, 0x0f);

		RZ3BitBlit(gp, &bb);

		xpan = 0;
		ypan = 0;

		return(1);
	} else if (md->DEP == 16) {
		struct grf_bitblt bb = {
			GRFBBOPset,
			0, 0,
			0, 0,
			md->TX, md->TY,
			0x0000
		};
		WSeq(ba, SEQ_ID_MAP_MASK, 0x0f);

		RZ3BitBlit16(gp, &bb);

		xpan = 0;
		ypan = 0;

		return(1);
	} else
		return(0);
}

/* standard-palette definition */

unsigned char RZ3StdPalette[16*3] = {
/*        R   G   B  */
	  0,  0,  0,
	192,192,192,
	128,  0,  0,
	  0,128,  0,
	  0,  0,128,
	128,128,  0,
	  0,128,128,
	128,  0,128,
	 64, 64, 64, /* the higher 8 colors have more intensity for  */
	255,255,255, /* compatibility with standard attributes       */
	255,  0,  0,
	  0,255,  0,
	  0,  0,255,
	255,255,  0,
	  0,255,255,
	255,  0,255
};

/*
 * The following structures are examples for monitor-definitions. To make one
 * of your own, first use "DefineMonitor" and create the 8-bit or 16-bit
 * monitor-mode of your dreams. Then save it, and make a structure from the
 * values provided in the file DefineMonitor stored - the labels in the comment
 * above the structure definition show where to put what value.
 *
 * If you want to use your definition for the text-mode, you'll need to adapt
 * your 8-bit monitor-definition to the font you want to use. Be FX the width of
 * the font, then the following modifications have to be applied to your values:
 *
 * HBS = (HBS * 4) / FX
 * HSS = (HSS * 4) / FX
 * HSE = (HSE * 4) / FX
 * HBE = (HBE * 4) / FX
 * HT  = (HT  * 4) / FX
 *
 * Make sure your maximum width (MW) and height (MH) are even multiples of
 * the fonts' width and height.
 *
 * You may use definitons created by the old DefineMonitor, but you'll get
 * better results with the new DefineMonitor supplied along with the Retin Z3.
*/

/*
 *  FQ     FLG    MW   MH   HBS HSS HSE HBE  HT  VBS  VSS  VSE  VBE   VT
 * Depth,          PAL, TX,  TY,    XY,FontX, FontY,    FontData,  FLo,  Fhi
 */
static struct MonDef monitor_defs[] = {
  /* Text-mode definitions */

  /* horizontal 31.5 kHz */
#ifdef KFONT_8X11
  { 50000000,  28,  640, 506,   81, 86, 93, 98, 95, 513, 513, 521, 535, 535,
      4, RZ3StdPalette, 80,  46,  3680,    8,    11, kernel_font_8x11, 32,  255},
#else
  { 50000000,  28,  640, 512,   81, 86, 93, 98, 95, 513, 513, 521, 535, 535,
      4, RZ3StdPalette, 80,  64,  5120,    8,     8, kernel_font_8x8,   32,  255},
#endif

  /* horizontal 38kHz */
#ifdef KFONT_8X11
  { 75000000,  28,  768, 594,   97, 99,107,120,117, 601, 615, 625, 638, 638,
      4, RZ3StdPalette, 96,  54,  5184,    8,     11, kernel_font_8x11,   32,  255},
#else
  { 75000000,  28,  768, 600,   97, 99,107,120,117, 601, 615, 625, 638, 638,
      4, RZ3StdPalette, 96,  75,  7200,    8,     8, kernel_font_8x8,   32,  255},
#endif

  /* horizontal 64kHz */
#ifdef KFONT_8X11
  { 50000000, 24,  768, 594,   97,104,112,122,119, 601, 606, 616, 628, 628,
      4, RZ3StdPalette, 96,  54,  5184,    8,     8, kernel_font_8x8,   32,  255},
#else
  { 50000000, 24,  768, 600,   97,104,112,122,119, 601, 606, 616, 628, 628,
      4, RZ3StdPalette, 96,  75,  7200,    8,     8, kernel_font_8x8,   32,  255},
#endif

  /* 8-bit gfx-mode definitions */

  /*
   * IMPORTANT: the "logical" screen size can be up to 2048x2048 pixels,
   * independent from the "physical" screen size. If your code does NOT
   * support panning, please adjust the "logical" screen sizes below to
   * match the physical ones
   */

  /* 640 x 480, 8 Bit, 31862 Hz, 63 Hz */
  { 26000000,  0,  640, 480,  161,175,188,200,199, 481, 483, 491, 502, 502,
      8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},
  /* This is the logical ^    ^    screen size */

  /* 640 x 480, 8 Bit, 38366 Hz, 76 Hz */
 { 31000000,  0,  640, 480,  161,169,182,198,197, 481, 482, 490, 502, 502,
     8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},

  /* 800 x 600, 8 Bit, 31620 Hz, 50 Hz (1950) */
  { 32000000,  0,  800, 600,  201,202,218,249,248, 601, 602, 612, 628, 628,
      8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},
  /* 800 x 600, 8 Bit, 38537 Hz, 61 Hz */
  { 39000000,  0,  800, 600,  201,211,227,249,248, 601, 603, 613, 628, 628,
      8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},

  /* 1024 x 768, 8 Bit, 63862 Hz, 79 Hz */
  { 82000000,  0, 1024, 768,  257,257,277,317,316, 769, 771, 784, 804, 804,
      8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},

  /* 1120 x 896, 8 Bit, 64000 Hz, 69 Hz */
  { 97000000,  0, 1120, 896,  281,283,306,369,368, 897, 898, 913, 938, 938,
      8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},

  /* 1152 x 910, 8 Bit, 76177 Hz, 79 Hz */
  {110000000,  0, 1152, 910,  289,310,333,357,356, 911, 923, 938, 953, 953,
      8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},

  /* 1184 x 848, 8 Bit, 73529 Hz, 82 Hz */
  {110000000,  0, 1184, 848,  297,319,342,370,369, 849, 852, 866, 888, 888,
      8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},

  /* 1280 x 1024, 8 Bit, 64516 Hz, 60 Hz */
  {104000000, 0, 1280,1024,  321,323,348,399,398,1025,1026,1043,1073,1073,
     8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},

/* WARNING: THE FOLLOWING MONITOR MODE EXCEEDS THE 110-MHz LIMIT THE PROCESSOR
            HAS BEEN SPECIFIED FOR. USE AT YOUR OWN RISK (AND THINK ABOUT
            MOUNTING SOME COOLING DEVICE AT THE PROCESSOR AND RAMDAC)!     */
  /* 1280 x 1024, 8 Bit, 75436 Hz, 70 Hz */
  {121000000, 0, 1280,1024,  321,322,347,397,396,1025,1026,1043,1073,1073,
     8, RZ3StdPalette,1280,1024,  5120,    8,     8, kernel_font_8x8,   32,  255},


  /* 16-bit gfx-mode definitions */

  /* 640 x 480, 16 Bit, 31795 Hz, 63 Hz */
  { 51000000, 0,  640, 480,  321,344,369,397,396, 481, 482, 490, 502, 502,
      16,           0,1280, 1024,  7200,    8,     8, kernel_font_8x8,   32,  255},

  /* 800 x 600, 16 Bit, 38500 Hz, 61 Hz */
  { 77000000, 0,  800, 600,  401,418,449,496,495, 601, 602, 612, 628, 628,
      16,           0,1280, 1024,  7200,    8,     8, kernel_font_8x8,   32,  255},

  /* 1024 x 768, 16 Bit, 42768 Hz, 53 Hz */
  {110000000,  0, 1024, 768,  513,514,554,639,638, 769, 770, 783, 804, 804,
      16,           0,1280, 1024,  7200,    8,     8, kernel_font_8x8,   32,  255},

  /* 864 x 648, 16 Bit, 50369 Hz, 74 Hz */
  {109000000,  0,  864, 648,  433,434,468,537,536, 649, 650, 661, 678, 678,
      16,           0,1280, 1024,  7200,    8,     8, kernel_font_8x8,   32,  255},

/*
 * WARNING: THE FOLLOWING MONITOR MODE EXCEEDS THE 110-MHz LIMIT THE PROCESSOR
 *          HAS BEEN SPECIFIED FOR. USE AT YOUR OWN RISK (AND THINK ABOUT
 *          MOUNTING SOME COOLING DEVICE AT THE PROCESSOR AND RAMDAC)!
 */
  /* 1024 x 768, 16 Bit, 48437 Hz, 60 Hz */
  {124000000,  0, 1024, 768,  513,537,577,636,635, 769, 770, 783, 804, 804,
      16,           0,1280, 1024,  7200,    8,     8, kernel_font_8x8,   32,  255},
};

static const char *monitor_descr[] = {
#ifdef KFONT_8X11
  "80x46 (640x506) 31.5kHz",
  "96x54 (768x594) 38kHz",
  "96x54 (768x594) 64kHz",
#else
  "80x64 (640x512) 31.5kHz",
  "96x75 (768x600) 38kHz",
  "96x75 (768x600) 64kHz",
#endif

  "GFX-8 (640x480) 31.5kHz",
  "GFX-8 (640x480) 38kHz",
  "GFX-8 (800x600) 31.6kHz",
  "GFX-8 (800x600) 38.5kHz",
  "GFX-8 (1024x768) 64kHz",
  "GFX-8 (1120x896) 64kHz",
  "GFX-8 (1152x910) 76kHz",
  "GFX-8 (1182x848) 73kHz",
  "GFX-8 (1280x1024) 64.5kHz",
  "GFX-8 (1280x1024) 75.5kHz ***EXCEEDS CHIP LIMIT!!!***",

  "GFX-16 (640x480) 31.8kHz",
  "GFX-16 (800x600) 38.5kHz",
  "GFX-16 (1024x768) 42.8kHz",
  "GFX-16 (864x648) 50kHz",
  "GFX-16 (1024x768) 48.5kHz ***EXCEEDS CHIP LIMIT!!!***",
};

int rh_mon_max = sizeof (monitor_defs)/sizeof (monitor_defs[0]);

/* patchable */
int rh_default_mon = 0;
int rh_default_gfx = 4;

static struct MonDef *current_mon;

int  rh_mode     __P((struct grf_softc *, int, void *, int, int));
void grfrhattach __P((struct device *, struct device *, void *));
int  grfrhprint  __P((void *, char *));
int  grfrhmatch  __P((struct device *, struct cfdata *, void *));

struct cfdriver grfrhcd = {
	NULL, "grfrh", grfrhmatch, grfrhattach,
	DV_DULL, sizeof(struct grf_softc), NULL, 0
};

static struct cfdata *cfdata;

int
grfrhmatch(pdp, cfp, auxp)
	struct device *pdp;
	struct cfdata *cfp;
	void *auxp;
{
#ifdef RETINACONSOLE
	static int rhconunit = -1;
#endif
	struct zthreebus_args *zap;

	zap = auxp;

	if (amiga_realconfig == 0)
#ifdef RETINACONSOLE
		if (rhconunit != -1)
#endif
			return(0);
	if (zap->manid != 18260 || zap->prodid != 16)
		return(0);
#ifdef RETINACONSOLE
	if (amiga_realconfig == 0 || rhconunit != cfp->cf_unit) {
#endif
		if ((unsigned)rh_default_mon >= rh_mon_max ||
		    monitor_defs[rh_default_mon].DEP == 8)
			rh_default_mon = 0;
		current_mon = monitor_defs + rh_default_mon;
		if (rh_mondefok(current_mon) == 0)
			return(0);
#ifdef RETINACONSOLE
		if (amiga_realconfig == 0) {
			rhconunit = cfp->cf_unit;
			cfdata = cfp;
		}
	}
#endif
	return(1);
}

void
grfrhattach(pdp, dp, auxp)
	struct device *pdp, *dp;
	void *auxp;
{
	static struct grf_softc congrf;
	static int coninited;
	struct zthreebus_args *zap;
	struct grf_softc *gp;

	zap = auxp;

	if (dp == NULL)
		gp = &congrf;
	else
		gp = (struct grf_softc *)dp;
	if (dp != NULL && congrf.g_regkva != 0) {
		/*
		 * inited earlier, just copy (not device struct)
		 */
		bcopy(&congrf.g_display, &gp->g_display,
		    (char *)&gp[1] - (char *)&gp->g_display);
	} else {
		gp->g_regkva = (volatile caddr_t)zap->va;
		gp->g_fbkva = (volatile caddr_t)zap->va + LM_OFFSET;
		gp->g_unit = GRF_RETINAIII_UNIT;
		gp->g_mode = rh_mode;
		gp->g_conpri = grfrh_cnprobe();
		gp->g_flags = GF_ALIVE;
		grfrh_iteinit(gp);
		(void)rh_load_mon(gp, current_mon);
	}
	if (dp != NULL)
		printf("\n");
	/*
	 * attach grf
	 */
	amiga_config_found(cfdata, &gp->g_device, gp, grfrhprint);
}

int
grfrhprint(auxp, pnp)
	void *auxp;
	char *pnp;
{
	if (pnp)
		printf("ite at %s", pnp);
	return(UNCONF);
}

int
rh_getvmode(gp, vm)
	struct grf_softc *gp;
	struct grfvideo_mode *vm;
{
	struct MonDef *md;

	if (vm->mode_num && vm->mode_num > rh_mon_max)
		return(EINVAL);

	if (! vm->mode_num)
		vm->mode_num = (current_mon - monitor_defs) + 1;

	md = monitor_defs + (vm->mode_num - 1);
	strncpy (vm->mode_descr, monitor_descr + (vm->mode_num - 1),
	   sizeof (vm->mode_descr));
	vm->pixel_clock  = md->FQ;
	vm->disp_width   = md->MW;
	vm->disp_height  = md->MH;
	vm->depth        = md->DEP;
	vm->hblank_start = md->HBS;
	vm->hblank_stop  = md->HBE;
	vm->hsync_start  = md->HSS;
	vm->hsync_stop   = md->HSE;
	vm->htotal       = md->HT;
	vm->vblank_start = md->VBS;
	vm->vblank_stop  = md->VBE;
	vm->vsync_start  = md->VSS;
	vm->vsync_stop   = md->VSE;
	vm->vtotal       = md->VT;

	return(0);
}


int
rh_setvmode(gp, mode, txtonly)
	struct grf_softc *gp;
	unsigned mode;
	int txtonly;
{
	struct MonDef *md;
	int error;

	if (!mode || mode > rh_mon_max)
		return(EINVAL);

	if (txtonly && monitor_defs[mode-1].DEP != 4)
		return(EINVAL);

	current_mon = monitor_defs + (mode - 1);

	error = rh_load_mon (gp, current_mon) ? 0 : EINVAL;

	return(error);
}


/*
 * Change the mode of the display.
 * Return a UNIX error number or 0 for success.
 */
rh_mode(gp, cmd, arg, a2, a3)
	register struct grf_softc *gp;
	int cmd;
	void *arg;
	int a2, a3;
{
  /* implement these later... */

	switch (cmd) {
	case GM_GRFON:
		rh_setvmode (gp, rh_default_gfx + 1, 0);
		return(0);

	case GM_GRFOFF:
		rh_setvmode (gp, rh_default_mon + 1, 0);
		return(0);

	case GM_GRFCONFIG:
		return(0);

	case GM_GRFGETVMODE:
		return(rh_getvmode (gp, (struct grfvideo_mode *) arg));

	case GM_GRFSETVMODE:
		return(rh_setvmode (gp, *(unsigned *) arg, 1));

	case GM_GRFGETNUMVM:
		*(int *)arg = rh_mon_max;
		return(0);

#ifdef BANKEDDEVPAGER
	case GM_GRFGETBANK:
	case GM_GRFGETCURBANK:
	case GM_GRFSETBANK:
		return(EINVAL);
#endif
	case GM_GRFIOCTL:
		return(rh_ioctl (gp, (int) arg, (caddr_t) a2));

	default:
		break;
	}

	return(EINVAL);
}

int
rh_ioctl (gp, cmd, data)
	register struct grf_softc *gp;
	int cmd;
	void *data;
{
	switch (cmd) {
	case GRFIOCGSPRITEPOS:
		return(rh_getspritepos (gp, (struct grf_position *) data));

	case GRFIOCSSPRITEPOS:
		return(rh_setspritepos (gp, (struct grf_position *) data));

	case GRFIOCSSPRITEINF:
		return(rh_setspriteinfo (gp, (struct grf_spriteinfo *) data));

	case GRFIOCGSPRITEINF:
		return(rh_getspriteinfo (gp, (struct grf_spriteinfo *) data));

	case GRFIOCGSPRITEMAX:
		return(rh_getspritemax (gp, (struct grf_position *) data));

	case GRFIOCGETCMAP:
		return(rh_getcmap (gp, (struct grf_colormap *) data));

	case GRFIOCPUTCMAP:
		return(rh_putcmap (gp, (struct grf_colormap *) data));

	case GRFIOCBITBLT:
		return(rh_bitblt (gp, (struct grf_bitblt *) data));
	}

	return(EINVAL);
}


int
rh_getcmap (gfp, cmap)
	struct grf_softc *gfp;
	struct grf_colormap *cmap;
{
	volatile unsigned char *ba;
	u_char red[256], green[256], blue[256], *rp, *gp, *bp;
	short x;
	int error;

	if (cmap->count == 0 || cmap->index >= 256)
		return 0;

	if (cmap->index + cmap->count > 256)
		cmap->count = 256 - cmap->index;

	ba = gfp->g_regkva;
	/* first read colors out of the chip, then copyout to userspace */
	vgaw (ba, VDAC_ADDRESS_W, cmap->index);
	x = cmap->count - 1;
	rp = red + cmap->index;
	gp = green + cmap->index;
	bp = blue + cmap->index;
	do {
		*rp++ = vgar (ba, VDAC_DATA) << 2;
		*gp++ = vgar (ba, VDAC_DATA) << 2;
		*bp++ = vgar (ba, VDAC_DATA) << 2;
	} while (x-- > 0);

	if (!(error = copyout (red + cmap->index, cmap->red, cmap->count))
	    && !(error = copyout (green + cmap->index, cmap->green, cmap->count))
	    && !(error = copyout (blue + cmap->index, cmap->blue, cmap->count)))
		return(0);

	return(error);
}

int
rh_putcmap (gfp, cmap)
	struct grf_softc *gfp;
	struct grf_colormap *cmap;
{
	volatile unsigned char *ba;
	u_char red[256], green[256], blue[256], *rp, *gp, *bp;
	short x;
	int error;

	if (cmap->count == 0 || cmap->index >= 256)
		return(0);

	if (cmap->index + cmap->count > 256)
		cmap->count = 256 - cmap->index;

	/* first copy the colors into kernelspace */
	if (!(error = copyin (cmap->red, red + cmap->index, cmap->count))
	    && !(error = copyin (cmap->green, green + cmap->index, cmap->count))
	    && !(error = copyin (cmap->blue, blue + cmap->index, cmap->count))) {
		/* argl.. LoadPalette wants a different format, so do it like with
		* Retina2.. */
		ba = gfp->g_regkva;
		vgaw (ba, VDAC_ADDRESS_W, cmap->index);
		x = cmap->count - 1;
		rp = red + cmap->index;
		gp = green + cmap->index;
		bp = blue + cmap->index;
		do {
			vgaw (ba, VDAC_DATA, *rp++ >> 2);
			vgaw (ba, VDAC_DATA, *gp++ >> 2);
			vgaw (ba, VDAC_DATA, *bp++ >> 2);
		} while (x-- > 0);
		return(0);
	}
	else
		return(error);
}

int
rh_getspritepos (gp, pos)
	struct grf_softc *gp;
	struct grf_position *pos;
{
	volatile unsigned char *acm = gp->g_regkva + ACM_OFFSET;

	pos->x = acm[ACM_CURSOR_POSITION + 0] +
	    (acm[ACM_CURSOR_POSITION + 1] << 8);
	pos->y = acm[ACM_CURSOR_POSITION + 2] +
	    (acm[ACM_CURSOR_POSITION + 3] << 8);

	pos->x += xpan;
	pos->y += ypan;

	return(0);
}

int
rh_setspritepos (gp, pos)
	struct grf_softc *gp;
	struct grf_position *pos;
{
	RZ3SetHWCloc (gp, pos->x, pos->y);
	return(0);
}

int
rh_getspriteinfo (gp, info)
	struct grf_softc *gp;
	struct grf_spriteinfo *info;
{
	volatile unsigned char *ba, *fb;

	ba = gp->g_regkva;
	fb = gp->g_fbkva;
	if (info->set & GRFSPRSET_ENABLE)
		info->enable = RSeq (ba, SEQ_ID_CURSOR_CONTROL) & 0x01;
	if (info->set & GRFSPRSET_POS)
		rh_getspritepos (gp, &info->pos);
	if (info->set & GRFSPRSET_HOT) {
		info->hot.x = RSeq (ba, SEQ_ID_CURSOR_X_INDEX) & 0x3f;
		info->hot.y = RSeq (ba, SEQ_ID_CURSOR_Y_INDEX) & 0x7f;
	}
	if (info->set & GRFSPRSET_CMAP) {
		struct grf_colormap cmap;
		int index;
		cmap.index = 0;
		cmap.count = 256;
		rh_getcmap (gp, &cmap);
		index = RSeq (ba, SEQ_ID_CURSOR_COLOR0);
		info->cmap.red[0] = cmap.red[index];
		info->cmap.green[0] = cmap.green[index];
		info->cmap.blue[0] = cmap.blue[index];
		index = RSeq (ba, SEQ_ID_CURSOR_COLOR1);
		info->cmap.red[1] = cmap.red[index];
		info->cmap.green[1] = cmap.green[index];
		info->cmap.blue[1] = cmap.blue[index];
	}
	if (info->set & GRFSPRSET_SHAPE) {
		u_char image[128], mask[128];
		volatile u_long *hwp;
		u_char *imp, *mp;
		short row;

		/* sprite bitmap is WEIRD in this chip.. see grf_rhvar.h
		 * for an explanation. To convert to "our" format, the
		 * following holds:
		 *   col2   = !image & mask
		 *   col1   = image & mask
		 *   transp = !mask
		 * and thus:
		 *   image  = col1
		 *   mask   = col1 | col2
		 * hope I got these bool-eqs right below..
		 */

		info->size.x = 64;
		info->size.y = 64;
		for (row = 0, hwp = (u_long *)(ba + LM_OFFSET + HWC_MEM_OFF),
		    mp = mask, imp = image;
		    row < 64;
		    row++) {
			u_long bp10, bp20, bp11, bp21;
			bp10 = *hwp++;
			bp20 = *hwp++;
			bp11 = *hwp++;
			bp21 = *hwp++;
			M2I (bp10);
			M2I (bp20);
			M2I (bp11);
			M2I (bp21);
			*imp++ = (~bp10) & bp11;
			*imp++ = (~bp20) & bp21;
			*mp++  = (~bp10) | (bp10 & ~bp11);
			*mp++  = (~bp20) & (bp20 & ~bp21);
		}
		copyout (image, info->image, sizeof (image));
		copyout (mask, info->mask, sizeof (mask));
	}
	return(0);
}

int
rh_setspriteinfo (gp, info)
	struct grf_softc *gp;
	struct grf_spriteinfo *info;
{
	volatile unsigned char *ba, *fb;
	u_char control;

	ba = gp->g_regkva;
	fb = gp->g_fbkva;

	if (info->set & GRFSPRSET_SHAPE) {
		/*
		 * For an explanation of these weird actions here, see above
		 * when reading the shape.  We set the shape directly into
		 * the video memory, there's no reason to keep 1k on the
		 * kernel stack just as template
		 */
		u_char *image, *mask;
		volatile u_long *hwp;
		u_char *imp, *mp;
		short row;

		if (info->size.y > 64)
			info->size.y = 64;
		if (info->size.x > 64)
			info->size.x = 64;

		if (info->size.x < 32)
			info->size.x = 32;

		image = malloc(HWC_MEM_SIZE, M_TEMP, M_WAITOK);
		mask  = image + HWC_MEM_SIZE/2;

		copyin(info->image, image, info->size.y * info->size.x / 8);
		copyin(info->mask, mask, info->size.y * info->size.x / 8);

		hwp = (u_long *)(ba + LM_OFFSET + HWC_MEM_OFF);

		/*
		 * setting it is slightly more difficult, because we can't
		 * force the application to not pass a *smaller* than
		 * supported bitmap
		 */

		for (row = 0, mp = mask, imp = image;
		    row < info->size.y;
		    row++) {
			u_long im1, im2, m1, m2;

			im1 = *(unsigned long *)imp;
			imp += 4;
			m1  = *(unsigned long *)mp;
			mp  += 4;
			if (info->size.x > 32) {
	      			im2 = *(unsigned long *)imp;
				imp += 4;
				m2  = *(unsigned long *)mp;
				mp  += 4;
			}
			else
				im2 = m2 = 0;

			M2I(im1);
			M2I(im2);
			M2I(m1);
			M2I(m2);

			*hwp++ = ~m1;
			*hwp++ = ~m2;
			*hwp++ = m1 & im1;
			*hwp++ = m2 & im2;
		}
		for (; row < 64; row++) {
			*hwp++ = 0xffffffff;
			*hwp++ = 0xffffffff;
			*hwp++ = 0x00000000;
			*hwp++ = 0x00000000;
		}

		free(image, M_TEMP);
		RZ3SetupHWC(gp, 1, 0, 0, 0, 0);
	}
	if (info->set & GRFSPRSET_CMAP) {
		/* hey cheat a bit here.. XXX */
		WSeq(ba, SEQ_ID_CURSOR_COLOR0, 0);
		WSeq(ba, SEQ_ID_CURSOR_COLOR1, 1);
	}
	if (info->set & GRFSPRSET_ENABLE) {
		if (info->enable)
			control = 0x85;
		else
			control = 0;
		WSeq(ba, SEQ_ID_CURSOR_CONTROL, control);
	}
	if (info->set & GRFSPRSET_POS)
		rh_setspritepos(gp, &info->pos);
	if (info->set & GRFSPRSET_HOT) {
		WSeq(ba, SEQ_ID_CURSOR_X_INDEX, info->hot.x & 0x3f);
		WSeq(ba, SEQ_ID_CURSOR_Y_INDEX, info->hot.y & 0x7f);
	}

	return(0);
}

int
rh_getspritemax (gp, pos)
	struct grf_softc *gp;
	struct grf_position *pos;
{
	pos->x = 64;
	pos->y = 64;

	return(0);
}


int
rh_bitblt (gp, bb)
	struct grf_softc *gp;
	struct grf_bitblt *bb;
{
	struct MonDef *md = (struct MonDef *)gp->g_data;
	if (md->DEP < 16)
		RZ3BitBlit(gp, bb);
	else
		RZ3BitBlit16(gp, bb);
}
#endif	/* NGRF */
