/*
 *	$Id: i8042.h,v 1.4 1994/03/02 08:02:27 mycroft Exp $
 */

#define	KBSTATP		0x64	/* kbd controller status port (I) */
#define	 KBS_DIB	0x01	/* kbd data in buffer */
#define	 KBS_IBF	0x02	/* kbd input buffer low */
#define	 KBS_WARM	0x04	/* kbd input buffer low */
#define	 KBS_OCMD	0x08	/* kbd output buffer has command */
#define	 KBS_NOSEC	0x10	/* kbd security lock not engaged */
#define	 KBS_TERR	0x20	/* kbd transmission error */
#define	 KBS_RERR	0x40	/* kbd receive error */
#define	 KBS_PERR	0x80	/* kbd parity error */

#define	KBCMDP		0x64	/* kbd controller port (O) */
#define	KBDATAP		0x60	/* kbd data port (I) */
#define	KBOUTP		0x60	/* kbd data port (O) */

#define	 KBC_PULSE0	0xfe	/* pulse output bit 0 */
#define	 KBC_PULSE1	0xfd	/* pulse output bit 1 */
#define	 KBC_PULSE2	0xfb	/* pulse output bit 2 */
#define	 KBC_PULSE3	0xf7	/* pulse output bit 3 */

#define	K_RDCMDBYTE	0x20
#define	K_LDCMDBYTE	0x60

#define	KC8_TRANS	0x40	/* convert to old scan codes */
#define	KC8_MDISABLE	0x20	/* disable mouse */
#define	KC8_KDISABLE	0x10	/* disable keyboard */
#define	KC8_IGNSEC	0x08	/* ignore security lock */
#define	KC8_CPU		0x04	/* exit from protected mode reset */
#define	KC8_MENABLE	0x02	/* enable mouse interrupt */
#define	KC8_KENABLE	0x01	/* enable keyboard interrupt */
#define	CMDBYTE		(KC8_TRANS|KC8_CPU|KC8_MENABLE|KC8_KENABLE)
