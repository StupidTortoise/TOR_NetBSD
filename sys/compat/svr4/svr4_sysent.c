/*
 * System call switch table.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from: NetBSD syscalls.master,v 1.2 1994/06/29 06:30:37
 */

#include <sys/param.h>
#include <sys/systm.h>

int	nosys();

int	nosys();
int	exit();
int	fork();
int	read();
int	write();
int	svr4_open();
int	close();
int	svr4_wait();
int	svr4_creat();
int	link();
int	unlink();
int	svr4_execv();
int	chdir();
int	time();
int	svr4_mknod();
int	chmod();
int	chown();
int	obreak();
int	svr4_stat();
int	olseek();
int	getpid();
int	setuid();
int	getuid();
int	svr4_fstat();
int	access();
int	sync();
int	kill();
int	dup();
int	pipe();
int	profil();
int	getgid();
#ifdef SYSVMSG
int	msgsys();
#else
#endif
int	svr4_syssun();
int	acct();
#ifdef SYSVSHM
int	shmsys();
#else
#endif
#ifdef SYSVSEM
int	semsys();
#else
#endif
int	svr4_ioctl();
int	fsync();
int	execve();
int	umask();
int	chroot();
int	rmdir();
int	mkdir();
int	svr4_lstat();
int	symlink();
int	readlink();
int	setgroups();
int	getgroups();
int	fchmod();
int	fchown();
int	sigprocmask();
int	sigaltstack();
int	sigsuspend();
int	sigaction();
int	svr4_sigpending();
#ifdef NFSSERVER
#else
#endif
int	pathconf();
int	mincore();
int	svr4_mmap();
int	mprotect();
int	munmap();
int	fpathconf();
int	vfork();
int	fchdir();
int	readv();
int	writev();
int	svr4_setrlimit();
int	svr4_getrlimit();
int	rename();
int	svr4_uname();
int	setegid();
int	svr4_sysconfig();
int	adjtime();
int	seteuid();
int	svr4_fchroot();
int	svr4_vhangup();
int	gettimeofday();
int	getitimer();
int	setitimer();

#ifdef XXX_UNUSED
#define compat(n, name) n, __CONCAT(o,name)

#ifdef SYSVMSG
#else
#endif
#ifdef SYSVSHM
#else
#endif
#ifdef SYSVSEM
#else
#endif
#ifdef NFSSERVER
#else
#endif

#else /* XXX_UNUSED */
#define compat(n, name) 0, nosys
#endif /* XXX_UNUSED */

struct sysent svr4_sysent[] = {
	{ 0, nosys },			/* 0 = syscall */
	{ 1, exit },			/* 1 = exit */
	{ 0, fork },			/* 2 = fork */
	{ 3, read },			/* 3 = read */
	{ 3, write },			/* 4 = write */
	{ 3, svr4_open },			/* 5 = svr4_open */
	{ 1, close },			/* 6 = close */
	{ 4, svr4_wait },			/* 7 = svr4_wait */
	{ 2, svr4_creat },			/* 8 = svr4_creat */
	{ 2, link },			/* 9 = link */
	{ 1, unlink },			/* 10 = unlink */
	{ 2, svr4_execv },			/* 11 = svr4_execv */
	{ 1, chdir },			/* 12 = chdir */
	{ 0, time },			/* 13 = time */
	{ 3, svr4_mknod },			/* 14 = svr4_mknod */
	{ 2, chmod },			/* 15 = chmod */
	{ 3, chown },			/* 16 = chown */
	{ 1, obreak },			/* 17 = break */
	{ 2, svr4_stat },			/* 18 = svr4_stat */
	{ 3, olseek },			/* 19 = lseek */
	{ 0, getpid },			/* 20 = getpid */
	{ 0, nosys },			/* 21 = svr4_old_mount */
	{ 0, nosys },			/* 22 = System V umount */
	{ 1, setuid },			/* 23 = setuid */
	{ 0, getuid },			/* 24 = getuid */
	{ 0, nosys },			/* 25 = svr4_stime */
	{ 0, nosys },			/* 26 = svr4_ptrace */
	{ 0, nosys },			/* 27 = svr4_alarm */
	{ 1, svr4_fstat },			/* 28 = svr4_fstat */
	{ 0, nosys },			/* 29 = svr4_pause */
	{ 0, nosys },			/* 30 = svr4_utime */
	{ 0, nosys },			/* 31 = was stty */
	{ 0, nosys },			/* 32 = was gtty */
	{ 2, access },			/* 33 = access */
	{ 0, nosys },			/* 34 = svr4_nice */
	{ 0, nosys },			/* 35 = svr4_statfs */
	{ 0, sync },			/* 36 = sync */
	{ 2, kill },			/* 37 = kill */
	{ 0, nosys },			/* 38 = svr4_fstatfs */
	{ 0, nosys },			/* 39 = svr4_pgrpsys */
	{ 0, nosys },			/* 40 = svr4_xenix */
	{ 2, dup },			/* 41 = dup */
	{ 0, pipe },			/* 42 = pipe */
	{ 0, nosys },			/* 43 = svr4_times */
	{ 4, profil },			/* 44 = profil */
	{ 0, nosys },			/* 45 = svr4_plock */
	{ 0, nosys },			/* 46 = svr4_setgid */
	{ 0, getgid },			/* 47 = getgid */
	{ 0, nosys },			/* 48 = svr4_signal */
#ifdef SYSVMSG
	{ 6, msgsys },			/* 49 = msgsys */
#else
	{ 0, nosys },			/* 49 = nosys */
#endif
	{ 1, svr4_syssun },			/* 50 = svr4_syssun */
	{ 1, acct },			/* 51 = acct */
#ifdef SYSVSHM
	{ 4, shmsys },			/* 52 = shmsys */
#else
	{ 0, nosys },			/* 52 = nosys */
#endif
#ifdef SYSVSEM
	{ 5, semsys },			/* 53 = semsys */
#else
	{ 0, nosys },			/* 53 = nosys */
#endif
	{ 3, svr4_ioctl },			/* 54 = svr4_ioctl */
	{ 0, nosys },			/* 55 = svr4_uadmin */
	{ 0, nosys },			/* 56 = svr4_exch */
	{ 0, nosys },			/* 57 = svr4_utssys */
	{ 1, fsync },			/* 58 = fsync */
	{ 3, execve },			/* 59 = execve */
	{ 1, umask },			/* 60 = umask */
	{ 1, chroot },			/* 61 = chroot */
	{ 0, nosys },			/* 62 = svr4_fcntl */
	{ 0, nosys },			/* 63 = nosys */
	{ 0, nosys },			/* 64 = reserved for unix/pc */
	{ 0, nosys },			/* 65 = reserved for unix/pc */
	{ 0, nosys },			/* 66 = reserved for unix/pc */
	{ 0, nosys },			/* 67 = reserved for unix/pc */
	{ 0, nosys },			/* 68 = reserved for unix/pc */
	{ 0, nosys },			/* 69 = reserved for unix/pc */
	{ 0, nosys },			/* 70 = obsolete svr4_advfs */
	{ 0, nosys },			/* 71 = obsolete svr4_unadvfs */
	{ 0, nosys },			/* 72 = obsolete svr4_rmount */
	{ 0, nosys },			/* 73 = obsolete svr4_rumount */
	{ 0, nosys },			/* 74 = obsolete svr4_rfstart */
	{ 0, nosys },			/* 75 = obsolete svr4_sigret */
	{ 0, nosys },			/* 76 = obsolete svr4_rdebug */
	{ 0, nosys },			/* 77 = obsolete svr4_rfstop */
	{ 0, nosys },			/* 78 = svr4_rfsys */
	{ 1, rmdir },			/* 79 = rmdir */
	{ 2, mkdir },			/* 80 = mkdir */
	{ 0, nosys },			/* 81 = svr4_getdents */
	{ 0, nosys },			/* 82 = obsolete svr4_libattach */
	{ 0, nosys },			/* 83 = obsolete svr4_libdetach */
	{ 0, nosys },			/* 84 = svr4_sysfs */
	{ 0, nosys },			/* 85 = getmsg */
	{ 0, nosys },			/* 86 = putmsg */
	{ 0, nosys },			/* 87 = poll */
	{ 2, svr4_lstat },			/* 88 = svr4_lstat */
	{ 2, symlink },			/* 89 = symlink */
	{ 3, readlink },			/* 90 = readlink */
	{ 2, setgroups },			/* 91 = setgroups */
	{ 2, getgroups },			/* 92 = getgroups */
	{ 2, fchmod },			/* 93 = fchmod */
	{ 3, fchown },			/* 94 = fchown */
	{ 2, sigprocmask },			/* 95 = sigprocmask */
	{ 2, sigaltstack },			/* 96 = sigaltstack */
	{ 1, sigsuspend },			/* 97 = sigsuspend */
	{ 3, sigaction },			/* 98 = sigaction */
	{ 1, svr4_sigpending },			/* 99 = svr4_sigpending */
	{ 0, nosys },			/* 100 = svr4_context */
	{ 0, nosys },			/* 101 = svr4_evsys */
	{ 0, nosys },			/* 102 = svr4_evtrapret */
	{ 0, nosys },			/* 103 = svr4_statvfs */
	{ 0, nosys },			/* 104 = svr4_fstatvfs */
	{ 0, nosys },			/* 105 = svr4 reserved */
#ifdef NFSSERVER
	{ 0, nosys },			/* 106 = svr4_nfssvc */
#else
	{ 0, nosys },			/* 106 = nosys */
#endif
	{ 0, nosys },			/* 107 = svr4_waitsys */
	{ 0, nosys },			/* 108 = svr4_sigsendsys */
	{ 0, nosys },			/* 109 = svr4_hrtsys */
	{ 0, nosys },			/* 110 = svr4_acancel */
	{ 0, nosys },			/* 111 = svr4_async */
	{ 0, nosys },			/* 112 = svr4_priocntlsys */
	{ 2, pathconf },			/* 113 = pathconf */
	{ 3, mincore },			/* 114 = mincore */
	{ 6, svr4_mmap },			/* 115 = svr4_mmap */
	{ 3, mprotect },			/* 116 = mprotect */
	{ 2, munmap },			/* 117 = munmap */
	{ 2, fpathconf },			/* 118 = fpathconf */
	{ 0, vfork },			/* 119 = vfork */
	{ 1, fchdir },			/* 120 = fchdir */
	{ 3, readv },			/* 121 = readv */
	{ 3, writev },			/* 122 = writev */
	{ 0, nosys },			/* 123 = svr4_xstat */
	{ 0, nosys },			/* 124 = svr4_lxstat */
	{ 0, nosys },			/* 125 = svr4_fxstat */
	{ 0, nosys },			/* 126 = svr4_xmknod */
	{ 0, nosys },			/* 127 = svr4_clocal */
	{ 2, svr4_setrlimit },			/* 128 = svr4_setrlimit */
	{ 2, svr4_getrlimit },			/* 129 = svr4_getrlimit */
	{ 0, nosys },			/* 130 = svr4_lchown */
	{ 0, nosys },			/* 131 = svr4_memcntl */
	{ 0, nosys },			/* 132 = svr4_getpmsg */
	{ 0, nosys },			/* 133 = svr4_putpmsg */
	{ 2, rename },			/* 134 = rename */
	{ 1, svr4_uname },			/* 135 = svr4_uname */
	{ 1, setegid },			/* 136 = setegid */
	{ 1, svr4_sysconfig },			/* 137 = svr4_sysconfig */
	{ 2, adjtime },			/* 138 = adjtime */
	{ 0, nosys },			/* 139 = svr4_systeminfo */
	{ 0, nosys },			/* 140 = reserved */
	{ 1, seteuid },			/* 141 = seteuid */
	{ 0, nosys },			/* 142 = vtrace */
	{ 0, nosys },			/* 143 = svr4_fork1 */
	{ 0, nosys },			/* 144 = svr4_sigwait */
	{ 0, nosys },			/* 145 = svr4_lwp_info */
	{ 0, nosys },			/* 146 = svr4_yield */
	{ 0, nosys },			/* 147 = svr4_lwp_sema_p */
	{ 0, nosys },			/* 148 = svr4_lwp_sema_v */
	{ 0, nosys },			/* 149 = reserved */
	{ 0, nosys },			/* 150 = reserved */
	{ 0, nosys },			/* 151 = reserved */
	{ 0, nosys },			/* 152 = svr4_modctl */
	{ 1, svr4_fchroot },			/* 153 = svr4_fchroot */
	{ 0, nosys },			/* 154 = svr4_utimes */
	{ 0, svr4_vhangup },			/* 155 = svr4_vhangup */
	{ 2, gettimeofday },			/* 156 = gettimeofday */
	{ 2, getitimer },			/* 157 = getitimer */
	{ 3, setitimer },			/* 158 = setitimer */
	{ 0, nosys },			/* 159 = svr4_lwp_create */
	{ 0, nosys },			/* 160 = svr4_lwp_exit */
	{ 0, nosys },			/* 161 = svr4_lwp_suspend */
	{ 0, nosys },			/* 162 = svr4_lwp_continue */
	{ 0, nosys },			/* 163 = svr4_lwp_kill */
	{ 0, nosys },			/* 164 = svr4_lwp_self */
	{ 0, nosys },			/* 165 = svr4_lwp_getprivate */
	{ 0, nosys },			/* 166 = svr4_lwp_setprivate */
	{ 0, nosys },			/* 167 = svr4_lwp_wait */
	{ 0, nosys },			/* 168 = svr4_lwp_mutex_unlock */
	{ 0, nosys },			/* 169 = svr4_lwp_mutex_lock */
	{ 0, nosys },			/* 170 = svr4_lwp_cond_wait */
	{ 0, nosys },			/* 171 = svr4_lwp_cond_signal */
	{ 0, nosys },			/* 172 = svr4_lwp_cond_broadcast */
	{ 0, nosys },			/* 173 = svr4_pread */
	{ 0, nosys },			/* 174 = svr4_pwrite */
	{ 0, nosys },			/* 175 = svr4_llseek */
	{ 0, nosys },			/* 176 = svr4_inst_sync */
	{ 0, nosys },			/* 177 = reserved */
	{ 0, nosys },			/* 178 = reserved */
	{ 0, nosys },			/* 179 = reserved */
	{ 0, nosys },			/* 180 = reserved */
	{ 0, nosys },			/* 181 = reserved */
	{ 0, nosys },			/* 182 = reserved */
	{ 0, nosys },			/* 183 = reserved */
	{ 0, nosys },			/* 184 = reserved */
	{ 0, nosys },			/* 185 = reserved */
	{ 0, nosys },			/* 186 = svr4_auditsys */
};

int	nsvr4_sysent = sizeof(svr4_sysent) / sizeof(svr4_sysent[0]);
