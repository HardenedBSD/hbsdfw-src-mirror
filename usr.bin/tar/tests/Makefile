# $FreeBSD$

SRCTOP=		${.CURDIR:H:H:H}
TESTSDIR=	${TESTSBASE}/usr.bin/tar

LIBARCHIVEDIR=	${SRCTOP}/contrib/libarchive

ATF_TESTS_SH+=	functional_test

BINDIR=		${TESTSDIR}

CFLAGS+=	-DPLATFORM_CONFIG_H=\"${SRCTOP}/lib/libarchive/config_freebsd.h\"
CFLAGS+=	-I${SRCTOP}/lib/libarchive -I${.OBJDIR}
CFLAGS+=	-I${LIBARCHIVEDIR}/tar -I${LIBARCHIVEDIR}/tar/test
CFLAGS+=	-I${LIBARCHIVEDIR}/libarchive
CFLAGS+=	-I${LIBARCHIVEDIR}/test_utils

# Uncomment to link against dmalloc
#LDADD+= -L/usr/local/lib -ldmalloc
#CFLAGS+= -I/usr/local/include -DUSE_DMALLOC

PROGS+=		bsdtar_test

.PATH: ${LIBARCHIVEDIR}/tar/test
TESTS_SRCS=	\
	test_0.c				\
	test_basic.c				\
	test_copy.c				\
	test_empty_mtree.c			\
	test_extract_tar_bz2.c			\
	test_extract_tar_grz.c			\
	test_extract_tar_gz.c			\
	test_extract_tar_lrz.c			\
	test_extract_tar_lz.c			\
	test_extract_tar_lz4.c			\
	test_extract_tar_lzma.c			\
	test_extract_tar_lzo.c			\
	test_extract_tar_xz.c			\
	test_extract_tar_zstd.c			\
	test_format_newc.c			\
	test_help.c				\
	test_leading_slash.c			\
	test_missing_file.c			\
	test_option_C_mtree.c			\
	test_option_C_upper.c			\
	test_option_H_upper.c			\
	test_option_L_upper.c			\
	test_option_O_upper.c			\
	test_option_T_upper.c			\
	test_option_U_upper.c			\
	test_option_X_upper.c			\
	test_option_a.c				\
	test_option_acls.c			\
	test_option_b.c				\
	test_option_b64encode.c			\
	test_option_exclude.c			\
	test_option_exclude_vcs.c		\
	test_option_fflags.c			\
	test_option_gid_gname.c			\
	test_option_grzip.c			\
	test_option_j.c				\
	test_option_k.c				\
	test_option_keep_newer_files.c		\
	test_option_lrzip.c			\
	test_option_lz4.c			\
	test_option_lzma.c			\
	test_option_lzop.c			\
	test_option_n.c				\
	test_option_newer_than.c		\
	test_option_nodump.c			\
	test_option_older_than.c		\
	test_option_passphrase.c		\
	test_option_q.c				\
	test_option_r.c				\
	test_option_s.c				\
	test_option_uid_uname.c			\
	test_option_uuencode.c			\
	test_option_xattrs.c			\
	test_option_xz.c			\
	test_option_z.c				\
	test_option_zstd.c			\
	test_patterns.c				\
	test_print_longpath.c			\
	test_stdio.c				\
	test_strip_components.c			\
	test_symlink_dir.c			\
	test_version.c

SRCS.bsdtar_test=	\
	${TESTS_SRCS}	\
	list.h

.PATH: ${LIBARCHIVEDIR}/test_utils
SRCS.bsdtar_test+=	test_main.c		\
			test_utils.c

DPADD.bsdtar_test=	${LIBARCHIVE}
LDADD.bsdtar_test=	-larchive

list.h: ${TESTS_SRCS} Makefile
	@(cd ${LIBARCHIVEDIR}/tar/test && \
	grep -h DEFINE_TEST ${.ALLSRC:N*Makefile}) > ${.TARGET}.tmp
	@mv ${.TARGET}.tmp ${.TARGET}

CLEANFILES+=	list.h list.h.tmp

FILES+=	test_extract.tar.Z.uu
FILES+=	test_extract.tar.bz2.uu
FILES+=	test_extract.tar.grz.uu
FILES+=	test_extract.tar.gz.uu
FILES+=	test_extract.tar.lrz.uu
FILES+=	test_extract.tar.lz.uu
FILES+=	test_extract.tar.lz4.uu
FILES+=	test_extract.tar.lzma.uu
FILES+=	test_extract.tar.lzo.uu
FILES+=	test_extract.tar.xz.uu
FILES+=	test_extract.tar.zst.uu
FILES+=	test_leading_slash.tar.uu
FILES+=	test_option_keep_newer_files.tar.Z.uu
FILES+=	test_option_passphrase.zip.uu
FILES+=	test_option_s.tar.Z.uu
FILES+=	test_patterns_2.tar.uu
FILES+=	test_patterns_3.tar.uu
FILES+=	test_patterns_4.tar.uu
FILES+=	test_print_longpath.tar.Z.uu

.include <bsd.test.mk>
