#		             -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])

# ====================
# Version informations
# ====================
m4_define([orcus_major_version], [0])
m4_define([orcus_minor_version], [7])
m4_define([orcus_micro_version], [0])
m4_define([orcus_version], [orcus_major_version.orcus_minor_version.orcus_micro_version])

# ================
# API informations
# ================
m4_define([orcus_minor_api_version], [m4_eval(orcus_minor_version + orcus_minor_version%2)])
m4_define([orcus_api_version], [orcus_major_version.orcus_minor_api_version])

# =============
# Automake init
# =============
AC_INIT([liborcus],[orcus_version])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign dist-bzip2 dist-xz])
AM_SILENT_RULES([yes])
AC_LANG([C++])
CXXFLAGS="$CXXFLAGS -O2 -fvisibility=hidden -Wall"

# ===========================
# Find required base packages
# ===========================
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL
PKG_PROG_PKG_CONFIG([0.20])

# =====================
# Find required headers
# =====================
AC_CHECK_HEADERS([stdlib.h sys/time.h unistd.h])

# =============================================================
# Checks for typedefs, structures, and compiler characteristics
# =============================================================
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T

# ============================
# Checks for library functions
# ============================
AC_FUNC_STRTOD
AC_CHECK_FUNCS([gettimeofday])
AC_DEFINE([_REENTRANT], [], [Enable reentrant.])
AC_DEFINE([BOOST_ALL_NO_LIB], [], [Disable boost's evil autolink.])
AX_BOOST_BASE

# ===========================================================
# Determine if we are going to need to link with Boost.System
# ===========================================================
dnl This seems to be necessary since boost 1.50 (1.48 does not need it,
dnl 1.49 is untested). The macro BOOST_THREAD_DONT_USE_SYSTEM mentioned
dnl in documentation has no effect.
AC_MSG_CHECKING([if we need to link with Boost.System])
AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
@%:@include <boost/version.hpp>
]], [[
#if BOOST_VERSION >= 105000
#   error yes, we need to link with Boost.System
#endif
]])],[
        AC_MSG_RESULT([no])
    ],[
        AC_MSG_RESULT([yes])
        AX_BOOST_SYSTEM
    ])
AC_LANG_POP([C++])

# ======================
# Set required ixion api
# ======================
IXION_REQUIRED_API_VERSION=0.8
AC_SUBST(IXION_REQUIRED_API_VERSION)

# =============
# Set orcus api
# =============
ORCUS_API_VERSION=orcus_api_version
AC_SUBST(ORCUS_API_VERSION)

# ==============
# Werror support
# ==============
AC_ARG_ENABLE([werror],
	[AS_HELP_STRING([--disable-werror], [Treat all warnings as errors, useful for development])],
	[enable_werror="$enableval"],
	[enable_werror=yes]
)
AS_IF([test x"$enable_werror" != "xno"], [
	CXXFLAGS="$CXXFLAGS -Werror"
])

# =========================================================
# Determine if we are going to build static lib (for MinGW)
# =========================================================
AM_CONDITIONAL([HAVE_STATIC_LIB],
               [test "$enable_shared" = no -a "$enable_static" = yes])

# =====
# Debug
# =====
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug], [Build with debug features in mind.])],
	[enable_debug="$enableval"],
	[enable_debug=no]
)
AS_IF([test "x$enable_debug" != "xno"], [
	CXXFLAGS="$CXXFLAGS -g"
], [
	AC_DEFINE([NDEBUG], [], [Disable debugging information])
])

# zlib is a hard requirement in liborcus-parser.
PKG_CHECK_MODULES([ZLIB], [zlib])

# ==============
# tools (binary)
# ==============
AC_ARG_WITH(tools,
            AS_HELP_STRING([--without-tools],
                           [Disables building of binary executable tools.]),
	[with_tools="$withval"],
	[with_tools=yes]
)

# ==================
# ods filter support
# ==================
AC_ARG_WITH(ods-filter,
            AS_HELP_STRING([--without-ods-filter],
                           [Disables the OpenDocument Format spreadsheet import filter.]),
	[with_ods_filter="$withval"],
	[with_ods_filter=yes]
)

# ===================
# xlsx filter support
# ===================
AC_ARG_WITH(xlsx-filter,
            AS_HELP_STRING([--without-xlsx-filter],
                           [Disables the Microsoft Excel OOXML import filter.]),
	[with_xlsx_filter="$withval"],
	[with_xlsx_filter=yes]
)

# ===================
# xls xml filter support
# ===================
AC_ARG_WITH(xls-xml-filter,
            AS_HELP_STRING([--without-xls-xml-filter],
                           [Disables the Microsoft Excel XML import filter.]),
	[with_xls_xml_filter="$withval"],
	[with_xls_xml_filter=yes]
)

# =======================
# gnumeric filter support
# =======================
AC_ARG_WITH(gnumeric-filter,
            AS_HELP_STRING([--without-gnumeric-filter],
                           [Disables the gnumeric import filter. The gnumeric import filter depends on zLib.]),
	[with_gnumeric_filter="$withval"],
	[with_gnumeric_filter=yes]
)

AM_CONDITIONAL([WITH_ODS_FILTER], [test "x$with_ods_filter" != "xno"])
AM_CONDITIONAL([WITH_XLSX_FILTER], [test "x$with_xlsx_filter" != "xno"])
AM_CONDITIONAL([WITH_XLS_XML_FILTER], [test "x$with_xls_xml_filter" != "xno"])
AM_CONDITIONAL([WITH_GNUMERIC_FILTER], [test "x$with_gnumeric_filter" != "xno"])
AM_CONDITIONAL([WITH_TOOLS], [test "x$with_tools" != "xno"])

# ============
# mdds support
# ============
PKG_CHECK_MODULES([MDDS],[mdds >= 0.8.0])
CXXFLAGS="$CXXFLAGS -DMDDS_HASH_CONTAINER_BOOST"

# =================
# Spreadsheet model
# =================
AC_ARG_ENABLE(spreadsheet-model,
	AS_HELP_STRING([--disable-spreadsheet-model],
		[Disable the spreadsheet model implementation in orcus.  Note that the spreadsheet-specific command line utilities will not be built when this is disabled.])],
	[enable_spreadsheet_model="$enableval"],
	[enable_spreadsheet_model=yes]
)
AS_IF([test "x$enable_spreadsheet_model" != "xno"], [
	PKG_CHECK_MODULES([LIBIXION],[libixion-$IXION_REQUIRED_API_VERSION])
])
AM_CONDITIONAL([BUILD_SPREADSHEET_MODEL], [test "x$enable_spreadsheet_model" != "xno"])

AC_CONFIG_FILES([Makefile
	liborcus-$ORCUS_API_VERSION.pc:liborcus.pc.in
	liborcus-spreadsheet-model-$ORCUS_API_VERSION.pc:liborcus-spreadsheet-model.pc.in
	VERSION
	include/Makefile
	include/orcus/Makefile
	include/orcus/mso/Makefile
	include/orcus/spreadsheet/Makefile
	src/Makefile
	src/liborcus/Makefile
	src/mso/Makefile
	src/parser/Makefile
	src/spreadsheet/Makefile
])
AC_OUTPUT

# ==============================================
# Display final informations about configuration
# ==============================================
AC_MSG_NOTICE([
==============================================================================
Build configuration:
	debug:               $enable_debug
	werror:              $enable_werror
	spreadsheet-model:   $enable_spreadsheet_model
        gnumeric-filter:     $with_gnumeric_filter
        ods-filter:          $with_ods_filter
        xlsx-filter:         $with_xlsx_filter
        xls-xml-filter:      $with_xls_xml_filter
        tools:               $with_tools
==============================================================================
])

