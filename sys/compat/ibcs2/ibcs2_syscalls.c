/*
 * System call names.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from: NetBSD syscalls.master,v 1.1 1994/05/22 10:04:45
 */

char *ibcs2_syscallnames[] = {
	"nosys",			/* 0 = nosys */
	"exit",			/* 1 = exit */
	"fork",			/* 2 = fork */
	"read",			/* 3 = read */
	"write",			/* 4 = write */
	"ibcs2_open",			/* 5 = ibcs2_open */
	"close",			/* 6 = close */
	"ibcs2_waitsys",			/* 7 = ibcs2_waitsys */
	"ocreat",			/* 8 = ocreat */
	"link",			/* 9 = link */
	"unlink",			/* 10 = unlink */
	"ibcs2_execv",			/* 11 = ibcs2_execv */
	"chdir",			/* 12 = chdir */
	"ibcs2_time",			/* 13 = ibcs2_time */
	"ibcs2_mknod",			/* 14 = ibcs2_mknod */
	"chmod",			/* 15 = chmod */
	"chown",			/* 16 = chown */
	"break",			/* 17 = break */
	"ibcs2_stat",			/* 18 = ibcs2_stat */
	"olseek",			/* 19 = olseek */
	"getpid",			/* 20 = getpid */
	"ibcs2_mount",			/* 21 = ibcs2_mount */
	"ibcs2_umount",			/* 22 = ibcs2_umount */
	"ibcs2_setuid",			/* 23 = ibcs2_setuid */
	"getuid",			/* 24 = getuid */
	"ibcs2_stime",			/* 25 = ibcs2_stime */
	"ibcs2_ptrace",			/* 26 = ibcs2_ptrace */
	"ibcs2_alarm",			/* 27 = ibcs2_alarm */
	"ibcs2_fstat",			/* 28 = ibcs2_fstat */
	"ibcs2_pause",			/* 29 = ibcs2_pause */
	"ibcs2_utime",			/* 30 = ibcs2_utime */
	"#31",			/* 31 = was stty */
	"#32",			/* 32 = was gtty */
	"access",			/* 33 = access */
	"ibcs2_nice",			/* 34 = ibcs2_nice */
	"ibcs2_statfs",			/* 35 = ibcs2_statfs */
	"sync",			/* 36 = sync */
	"kill",			/* 37 = kill */
	"ibcs2_fstatfs",			/* 38 = ibcs2_fstatfs */
	"ibcs2_pgrpsys",			/* 39 = ibcs2_pgrpsys */
	"ibcs2_xenix",			/* 40 = ibcs2_xenix */
	"dup",			/* 41 = dup */
	"pipe",			/* 42 = pipe */
	"ibcs2_times",			/* 43 = ibcs2_times */
	"profil",			/* 44 = profil */
	"ibcs2_plock",			/* 45 = ibcs2_plock */
	"ibcs2_setgid",			/* 46 = ibcs2_setgid */
	"getgid",			/* 47 = getgid */
	"ibcs2_sigsys",			/* 48 = ibcs2_sigsys */
#ifdef SYSVMSG
	"msgsys",			/* 49 = msgsys */
#else
	"#49",			/* 49 = nosys */
#endif
	"#50",			/* 50 = ibcs2_sys3b */
	"acct",			/* 51 = acct */
#ifdef SYSVSHM
	"shmsys",			/* 52 = shmsys */
#else
	"#52",			/* 52 = nosys */
#endif
#ifdef SYSVSEM
	"semsys",			/* 53 = semsys */
#else
	"#53",			/* 53 = nosys */
#endif
	"ibcs2_ioctl",			/* 54 = ibcs2_ioctl */
	"ibcs2_uadmin",			/* 55 = ibcs2_uadmin */
	"#56",			/* 56 = nosys */
	"ibcs2_utssys",			/* 57 = ibcs2_utssys */
	"#58",			/* 58 = nosys */
	"execve",			/* 59 = execve */
	"umask",			/* 60 = umask */
	"chroot",			/* 61 = chroot */
	"ibcs2_fcntl",			/* 62 = ibcs2_fcntl */
	"ibcs2_ulimit",			/* 63 = ibcs2_ulimit */
	"#64",			/* 64 = reserved for unix/pc */
	"#65",			/* 65 = reserved for unix/pc */
	"#66",			/* 66 = reserved for unix/pc */
	"#67",			/* 67 = reserved for unix/pc */
	"#68",			/* 68 = reserved for unix/pc */
	"#69",			/* 69 = reserved for unix/pc */
	"obs_rfs_advfs",			/* 70 = obsolete rfs_advfs */
	"obs_rfs_unadvfs",			/* 71 = obsolete rfs_unadvfs */
	"obs_rfs_rmount",			/* 72 = obsolete rfs_rmount */
	"obs_rfs_rumount",			/* 73 = obsolete rfs_rumount */
	"obs_rfs_rfstart",			/* 74 = obsolete rfs_rfstart */
	"obs_rfs_sigret",			/* 75 = obsolete rfs_sigret */
	"obs_rfs_rdebug",			/* 76 = obsolete rfs_rdebug */
	"obs_rfs_rfstop",			/* 77 = obsolete rfs_rfstop */
	"obs_rfs_rfsys",			/* 78 = obsolete rfs_rfsys */
	"rmdir",			/* 79 = rmdir */
	"mkdir",			/* 80 = mkdir */
	"ibcs2_getdents",			/* 81 = ibcs2_getdents */
	"#82",			/* 82 = nosys */
	"#83",			/* 83 = nosys */
	"ibcs2_sysfs",			/* 84 = ibcs2_sysfs */
	"ibcs2_getmsg",			/* 85 = ibcs2_getmsg */
	"ibcs2_putmsg",			/* 86 = ibcs2_putmsg */
	"ibcs2_poll",			/* 87 = ibcs2_poll */
	"#88",			/* 88 = nosys */
	"#89",			/* 89 = nosys */
	"symlink",			/* 90 = symlink */
	"ibcs2_lstat",			/* 91 = ibcs2_lstat */
	"readlink",			/* 92 = readlink */
	"#93",			/* 93 = nosys */
	"#94",			/* 94 = nosys */
	"#95",			/* 95 = nosys */
	"#96",			/* 96 = nosys */
	"#97",			/* 97 = nosys */
	"#98",			/* 98 = nosys */
	"#99",			/* 99 = nosys */
	"#100",			/* 100 = nosys */
	"#101",			/* 101 = nosys */
	"#102",			/* 102 = nosys */
	"sigreturn",			/* 103 = sigreturn */
	"#104",			/* 104 = nosys */
	"#105",			/* 105 = nosys */
	"#106",			/* 106 = nosys */
	"#107",			/* 107 = nosys */
	"#108",			/* 108 = nosys */
	"#109",			/* 109 = nosys */
	"#110",			/* 110 = nosys */
	"#111",			/* 111 = nosys */
	"#112",			/* 112 = nosys */
	"#113",			/* 113 = nosys */
	"#114",			/* 114 = nosys */
	"#115",			/* 115 = nosys */
	"#116",			/* 116 = nosys */
	"#117",			/* 117 = nosys */
	"#118",			/* 118 = nosys */
	"#119",			/* 119 = nosys */
	"#120",			/* 120 = nosys */
	"#121",			/* 121 = nosys */
	"#122",			/* 122 = nosys */
	"#123",			/* 123 = nosys */
	"#124",			/* 124 = nosys */
	"#125",			/* 125 = nosys */
	"#126",			/* 126 = nosys */
	"#127",			/* 127 = nosys */
	"#128",			/* 128 = nosys */
	"#129",			/* 129 = XENIX xlocking 0x0128 */
	"#130",			/* 130 = XENIX creatsem 0x0228 */
	"#131",			/* 131 = XENIX opensem 0x0328 */
	"#132",			/* 132 = XENIX sigsem 0x0428 */
	"#133",			/* 133 = XENIX waitsem 0x0528 */
	"#134",			/* 134 = XENIX nbwaitsem 0x0628 */
	"#135",			/* 135 = XENIX rdchk 0x0728 */
	"#136",			/* 136 = XENIX nosys 0x0828 */
	"#137",			/* 137 = XENIX nosys 0x0928 */
	"#138",			/* 138 = XENIX chsize 0x0a28 */
	"ibcs2_ftime",			/* 139 = ibcs2_ftime */
	"#140",			/* 140 = XENIX nap 0x0c28 */
	"#141",			/* 141 = XENIX sdget 0x0d28 */
	"#142",			/* 142 = XENIX sdfree 0x0e28 */
	"#143",			/* 143 = XENIX sdenter 0x0f28 */
	"#144",			/* 144 = XENIX sdleave 0x1028 */
	"#145",			/* 145 = XENIX sdgetv 0x1128 */
	"#146",			/* 146 = XENIX sdwaitv 0x1228 */
	"#147",			/* 147 = XENIX nosys 0x1328 */
	"#148",			/* 148 = XENIX nosys 0x1428 */
	"#149",			/* 149 = XENIX nosys 0x1528 */
	"#150",			/* 150 = XENIX nosys 0x1628 */
	"#151",			/* 151 = XENIX nosys 0x1728 */
	"#152",			/* 152 = XENIX nosys 0x1828 */
	"#153",			/* 153 = XENIX nosys 0x1928 */
	"#154",			/* 154 = XENIX nosys 0x1a28 */
	"#155",			/* 155 = XENIX nosys 0x1b28 */
	"#156",			/* 156 = XENIX nosys 0x1c28 */
	"#157",			/* 157 = XENIX nosys 0x1d28 */
	"#158",			/* 158 = XENIX nosys 0x1e28 */
	"#159",			/* 159 = XENIX nosys 0x1f28 */
	"#160",			/* 160 = XENIX proctl 0x2028 */
	"#161",			/* 161 = XENIX execseg 0x2128 */
	"#162",			/* 162 = XENIX unexecseg 0x2228 */
	"#163",			/* 163 = XENIX nosys 0x2328 */
	"ibcs2_select",			/* 164 = ibcs2_select */
	"#165",			/* 165 = XENIX eaccess 0x2528 */
	"#166",			/* 166 = XENIX paccess 0x2628 */
	"ibcs2_sigaction",			/* 167 = ibcs2_sigaction */
	"ibcs2_sigprocmask",			/* 168 = ibcs2_sigprocmask */
	"ibcs2_sigpending",			/* 169 = ibcs2_sigpending */
	"ibcs2_sigsuspend",			/* 170 = ibcs2_sigsuspend */
	"ibcs2_getgroups",			/* 171 = ibcs2_getgroups */
	"ibcs2_setgroups",			/* 172 = ibcs2_setgroups */
	"ibcs2_sysconf",			/* 173 = ibcs2_sysconf */
	"ibcs2_pathconf",			/* 174 = ibcs2_pathconf */
	"ibcs2_fpathconf",			/* 175 = ibcs2_fpathconf */
	"rename",			/* 176 = rename */
};
