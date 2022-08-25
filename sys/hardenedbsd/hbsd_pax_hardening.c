/*-
 * Copyright (c) 2014-2022, by Shawn Webb <shawn.webb@hardenedbsd.org>
 * Copyright (c) 2014-2017, by Oliver Pinter <oliver.pinter@hardenedbsd.org>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/cdefs.h>

#include "opt_compat.h"
#include "opt_pax.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/fcntl.h>
#include <sys/imgact.h>
#include <sys/imgact_elf.h>
#include <sys/jail.h>
#include <sys/ktr.h>
#include <sys/libkern.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/pax.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/sx.h>
#include <sys/sysctl.h>
#include <sys/vnode.h>

#include "hbsd_pax_internal.h"

FEATURE(hbsd_hardening, "Various hardening features.");

#if __FreeBSD_version < 1100000
#define	kern_unsetenv	unsetenv
#endif

#ifdef PAX_HARDENING
static int pax_procfs_harden_global = PAX_FEATURE_SIMPLE_ENABLED;
static int pax_randomize_pids_global = PAX_FEATURE_SIMPLE_ENABLED;
static int pax_init_hardening_global = PAX_FEATURE_SIMPLE_ENABLED;
static int pax_insecure_kmod_global = PAX_FEATURE_SIMPLE_DISABLED;
static int pax_tpe_global = PAX_FEATURE_OPTOUT;
static int harden_rtld_global = PAX_FEATURE_SIMPLE_ENABLED;
#else
static int pax_procfs_harden_global = PAX_FEATURE_SIMPLE_DISABLED;
static int pax_randomize_pids_global = PAX_FEATURE_SIMPLE_DISABLED;
static int pax_init_hardening_global = PAX_FEATURE_SIMPLE_DISABLED;
static int pax_insecure_kmod_global = PAX_FEATURE_SIMPLE_ENABLED;
static int pax_tpe_global = PAX_FEATURE_OPTOUT;
static int harden_rtld_global = PAX_FEATURE_SIMPLE_DISABLED;
#endif

static int pax_tpe_gid = 0;
static int pax_tpe_negate = 0;
static int pax_tpe_all = 1;
static int pax_tpe_root_owned = 1;
static int pax_tpe_user_owned = 0;

TUNABLE_INT("hardening.procfs_harden", &pax_procfs_harden_global);
TUNABLE_INT("hardening.randomize_pids", &pax_randomize_pids_global);
TUNABLE_INT("hardening.insecure_kmod", &pax_insecure_kmod_global);
TUNABLE_INT("hardening.tpe.status", &pax_tpe_global);
TUNABLE_INT("hardening.tpe.gid", &pax_tpe_gid);
TUNABLE_INT("hardening.tpe.negate", &pax_tpe_negate);
TUNABLE_INT("hardening.tpe.all", &pax_tpe_all);
TUNABLE_INT("hardening.tpe.root_owned", &pax_tpe_root_owned);
TUNABLE_INT("hardening.harden_rtld", &harden_rtld_global);

#ifdef PAX_SYSCTLS
SYSCTL_DECL(_hardening_pax);

SYSCTL_HBSD_2STATE(pax_procfs_harden_global, pr_hbsd.hardening.procfs_harden,
    _hardening, procfs_harden,
    CTLTYPE_INT|CTLFLAG_RWTUN|CTLFLAG_SECURE,
    "Harden procfs, disabling write of /proc/pid/mem");
SYSCTL_HBSD_2STATE_GLOBAL(pax_insecure_kmod_global, _hardening, insecure_kmod,
    CTLTYPE_INT|CTLFLAG_RWTUN|CTLFLAG_SECURE,
    "Enable loading of inecure kernel modules");
SYSCTL_HBSD_2STATE(harden_rtld_global, pr_hbsd.hardening.harden_rtld,
    _hardening, harden_rtld,
    CTLTYPE_INT|CTLFLAG_PRISON|CTLFLAG_RWTUN|CTLFLAG_SECURE,
    "Harden RTLD");

SYSCTL_DECL(_hardening_pax);
SYSCTL_NODE(_hardening_pax, OID_AUTO, tpe, CTLFLAG_RD, 0,
    "Settings for Trusted Path Execution (TPE).");

SYSCTL_HBSD_4STATE(pax_tpe_global, pr_hbsd.hardening.tpe,
    _hardening_pax_tpe, status,
    CTLTYPE_INT|CTLFLAG_RWTUN|CTLFLAG_PRISON|CTLFLAG_SECURE);
SYSCTL_INT(_hardening_pax_tpe, OID_AUTO, gid,
    CTLFLAG_RWTUN|CTLFLAG_SECURE, &pax_tpe_gid, 0,
    "Untrusted TPE GID");
SYSCTL_INT(_hardening_pax_tpe, OID_AUTO, negate,
    CTLFLAG_RWTUN|CTLFLAG_SECURE, &pax_tpe_negate, 0,
    "Negate TPE GID logic");
SYSCTL_INT(_hardening_pax_tpe, OID_AUTO, all,
    CTLFLAG_RWTUN|CTLFLAG_SECURE, &pax_tpe_all, 0,
    "Apply TPE to all users");
SYSCTL_INT(_hardening_pax_tpe, OID_AUTO, root_owned,
    CTLFLAG_RWTUN|CTLFLAG_SECURE, &pax_tpe_root_owned, 0,
    "Ensure directory is root-owned");
SYSCTL_INT(_hardening_pax_tpe, OID_AUTO, user_owned,
    CTLFLAG_RWTUN|CTLFLAG_SECURE, &pax_tpe_user_owned, 0,
    "Ensure directory is user-owned");
#endif

#ifdef PAX_JAIL_SUPPORT
SYSCTL_DECL(_security_jail_param_hardening);
SYSCTL_JAIL_PARAM(_hardening, harden_rtld,
    CTLTYPE_INT | CTLFLAG_RD, "I", "RTLD Hardening");
#endif

#if 0
#ifdef PAX_JAIL_SUPPORT
SYSCTL_JAIL_PARAM_SUBNODE(hardening_pax, tpe, "TPE");
SYSCTL_JAIL_PARAM(_hardening_pax_tpe, status,
    CTLTYPE_INT | CTLFLAG_RD, "I", "TPE status");
#endif
#endif

#if 0
#ifdef PAX_JAIL_SUPPORT
SYSCTL_JAIL_PARAM(hardening, procfs_harden,
    CTLTYPE_INT | CTLFLAG_RD, "I",
    "disabling write of /proc/pid/mem");
#endif
#endif

static bool _pax_tpe_active(struct thread *);

bool
pax_insecure_kmod(void)
{

	return (pax_insecure_kmod_global == PAX_FEATURE_SIMPLE_ENABLED);
}

static void
pax_hardening_sysinit(void)
{
	pax_state_t old_state;

	old_state = pax_procfs_harden_global;
	if (!pax_feature_simple_validate_state(&pax_procfs_harden_global)) {
		printf("[HBSD HARDENING] WARNING, invalid settings in loader.conf!"
		    " (hardening.procfs_harden = %d)\n", old_state);
	}
	if (bootverbose) {
		printf("[HBSD HARDENING] procfs hardening: %s\n",
		    pax_status_simple_str[pax_procfs_harden_global]);
	}

	old_state = pax_randomize_pids_global;
	if (!pax_feature_simple_validate_state(&pax_randomize_pids_global)) {
		printf("[HBSD HARDENING] WARNING, invalid settings in loader.conf!"
		    " (hardening.randomize_pids = %d)\n", old_state);
	}
	if (bootverbose) {
		printf("[HBSD HARDENING] randomize pids: %s\n",
		    pax_status_simple_str[pax_randomize_pids_global]);
	}

	(void)pax_feature_simple_validate_state(&pax_init_hardening_global);
	if (bootverbose) {
		printf("[HBSD HARDENING] unset insecure init variables: %s\n",
		    pax_status_simple_str[pax_init_hardening_global]);
	}
}
SYSINIT(pax_hardening, SI_SUB_PAX, SI_ORDER_SECOND, pax_hardening_sysinit, NULL);

int
pax_hardening_init_prison(struct prison *pr, struct vfsoptlist *opts)
{
	struct prison *pr_p;
	int error;

	CTR2(KTR_PAX, "%s: Setting prison %s PaX variables\n",
	    __func__, pr->pr_name);

	error = 0;
	if (pr == &prison0) {
		/* prison0 has no parent, use globals */
		pr->pr_hbsd.hardening.procfs_harden =
		    pax_procfs_harden_global;
		pr->pr_hbsd.hardening.tpe = pax_tpe_global;
		pr->pr_hbsd.hardening.harden_rtld = harden_rtld_global;
		pr->pr_allow &= ~(PR_ALLOW_UNPRIV_DEBUG);
	} else {
		KASSERT(pr->pr_parent != NULL,
		   ("%s: pr->pr_parent == NULL", __func__));
		pr_p = pr->pr_parent;

		pr->pr_hbsd.hardening.procfs_harden =
		    pr_p->pr_hbsd.hardening.procfs_harden;
		pr->pr_hbsd.hardening.tpe = pr_p->pr_hbsd.hardening.tpe;
		pr->pr_hbsd.hardening.harden_rtld =
		    pr_p->pr_hbsd.hardening.harden_rtld;
		error = pax_handle_prison_param(opts, "hardening.harden_rtld",
		    &(pr->pr_hbsd.hardening.harden_rtld));
	}

	return (error);
}

pax_flag_t
pax_hardening_setup_flags(struct image_params *imgp, struct thread *td,
    pax_flag_t mode)
{
	struct prison *pr;
	pax_flag_t flags;
	uint32_t status;

	flags = 0;
	status = 0;

	pr = pax_get_prison_td(td);
	status = pr->pr_hbsd.hardening.tpe;
	if (status == PAX_FEATURE_DISABLED) {
		flags &= ~PAX_NOTE_TPE;
		flags |= PAX_NOTE_NOTPE;
		return (flags);
	}
	if (status == PAX_FEATURE_FORCE_ENABLED) {
		flags |= PAX_NOTE_TPE;
		flags &= ~PAX_NOTE_NOTPE;
		return (flags);
	}

	if (status == PAX_FEATURE_OPTIN) {
		if ((mode & PAX_NOTE_TPE) == PAX_NOTE_TPE) {
			flags |= PAX_NOTE_TPE;
			flags &= ~PAX_NOTE_NOTPE;
		} else {
			flags &= ~PAX_NOTE_TPE;
			flags |= PAX_NOTE_NOTPE;
		}
		return (flags);
	}

	if (status == PAX_FEATURE_OPTOUT) {
		if ((mode & PAX_NOTE_NOTPE) == PAX_NOTE_NOTPE) {
			flags &= ~PAX_NOTE_TPE;
			flags |= PAX_NOTE_NOTPE;
		} else {
			flags |= PAX_NOTE_TPE;
			flags &= ~PAX_NOTE_NOTPE;
		}
		return (flags);
	}

	flags |= PAX_NOTE_TPE;
	flags &= ~PAX_NOTE_NOTPE;
	return (flags);
}

int
pax_procfs_harden(struct thread *td)
{
	struct prison *pr;

	pr = pax_get_prison_td(td);

	return (pr->pr_hbsd.hardening.procfs_harden ? EPERM : 0);
}

int
pax_enforce_tpe(struct thread *td, struct vnode *vn, const char *path)
{
	char *parent_path, *tmp;
	struct nameidata nd;
	struct prison *pr;
	struct vattr vap;
	int error;

	if (td == NULL || vn == NULL || path == NULL) {
		return (EDOOFUS);
	}

	pr = pax_get_prison_td(td);
	if (pr->pr_hbsd.hardening.tpe == PAX_FEATURE_DISABLED) {
		return (0);
	}

	if (!_pax_tpe_active(td)) {
		return (0);
	}

	tmp = strrchr(path, '/');
	if (tmp == NULL) {
		return (EDOOFUS);
	}
	if (strlen(tmp) < 2) {
		return (0);
	}

	parent_path = malloc((tmp - path) + 1, M_TEMP,
	    M_WAITOK | M_ZERO);
	strncpy(parent_path, path, tmp - path);

	memset(&nd, 0, sizeof(nd));
	NDINIT(&nd, LOOKUP, FOLLOW, UIO_SYSSPACE, parent_path, td);
	nd.ni_debugflags |= NAMEI_DBG_INITED;
	error = namei(&nd);
	if (error) {
		free(parent_path, M_TEMP);
		NDFREE_PNBUF(&nd);
		return (error);
	}

	error = VOP_GETATTR(nd.ni_vp, &vap, td->td_ucred);
	if (error) {
		goto end;
	}

	if (pax_tpe_root_owned) {
		if (vap.va_uid != 0) {
			error = EPERM;
			goto end;
		}
	}

	if (pax_tpe_user_owned && td->td_ucred->cr_uid != 0) {
		if (vap.va_uid != 0 && vap.va_uid != td->td_ucred->cr_uid) {
			error = EPERM;
			goto end;
		}
	}

	if ((vap.va_mode & (S_IWGRP | S_IWOTH)) != 0) {
		error = EPERM;
		goto end;
	}

end:
	NDFREE_PNBUF(&nd);
	free(parent_path, M_TEMP);
	return (error);
}

static bool
_pax_tpe_active(struct thread *td)
{
	pax_flag_t flags;

	pax_get_flags(td->td_proc, &flags);
	if ((flags & PAX_NOTE_NOTPE) == PAX_NOTE_NOTPE) {
		return (false);
	}

	if (pax_tpe_all) {
		return (true);
	}

	if (td->td_ucred->cr_gid == pax_tpe_gid) {
		return (pax_tpe_negate == 0);
	}

	return (pax_tpe_negate != 0);
}

extern int randompid;

static void
pax_randomize_pids(void *dummy __unused)
{
	if (pax_randomize_pids_global == PAX_FEATURE_SIMPLE_DISABLED)
		return;

	sx_xlock(&allproc_lock);
	/* Steal the logic from FreeBSD's sysctl_kern_randompid */
	randompid = 100 + (arc4random() % 1024);
	sx_xunlock(&allproc_lock);
}
SYSINIT(pax_randomize_pids, SI_SUB_KTHREAD_INIT, SI_ORDER_MIDDLE+1,
    pax_randomize_pids, NULL);

static void
pax_init_hardening(void *dummy __unused)
{
	/*
	 * Never should be made available from the loader / outside
	 * the pax_init_hardening_global variable.
	 */
	if (pax_init_hardening_global == PAX_FEATURE_SIMPLE_DISABLED)
		return;

	kern_unsetenv("init_chroot");
	kern_unsetenv("init_exec");
	kern_unsetenv("init_path");
	kern_unsetenv("init_script");
	kern_unsetenv("init_shell");
}
SYSINIT(pax_init_hardening, SI_SUB_PAX, SI_ORDER_ANY,
    pax_init_hardening, NULL);

