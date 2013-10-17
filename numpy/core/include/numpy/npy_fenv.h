#ifndef _NPY_FENV_H_
#define _NPY_FENV_H_

/* This code checks the IEEE status flags in a platform-dependent way */
/* Adapted from Numarray  */

/* platform-dependent code translates floating point
   status to an integer sum of these values
*/
#define UFUNC_FPE_DIVIDEBYZERO  1
#define UFUNC_FPE_OVERFLOW      2
#define UFUNC_FPE_UNDERFLOW     4
#define UFUNC_FPE_INVALID       8

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

/*  OSF/Alpha (Tru64)  ---------------------------------------------*/
#if defined(__osf__) && defined(__alpha)

#include <machine/fpu.h>

#define UFUNC_CHECK_STATUS(ret) { \
        unsigned long fpstatus; \
         \
        fpstatus = ieee_get_fp_control(); \
        /* clear status bits as well as disable exception mode if on */ \
        ieee_set_fp_control( 0 ); \
        ret = ((IEEE_STATUS_DZE & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((IEEE_STATUS_OVF & fpstatus) ? UFUNC_FPE_OVERFLOW : 0) \
                | ((IEEE_STATUS_UNF & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((IEEE_STATUS_INV & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        }

/* MS Windows -----------------------------------------------------*/
#elif defined(_MSC_VER)

#include <float.h>

/* Clear the floating point exception default of Borland C++ */
#if defined(__BORLANDC__)
#define UFUNC_NOFPE _control87(MCW_EM, MCW_EM);
#endif

#if defined(_WIN64)
#define UFUNC_CHECK_STATUS(ret) { \
        int fpstatus = (int) _clearfp(); \
         \
        ret = ((SW_ZERODIVIDE & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((SW_OVERFLOW & fpstatus) ? UFUNC_FPE_OVERFLOW : 0) \
                | ((SW_UNDERFLOW & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((SW_INVALID & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        }
#else
/* windows enables sse on 32 bit, so check both flags */
#define UFUNC_CHECK_STATUS(ret) { \
        int fpstatus, fpstatus2; \
        _statusfp2(&fpstatus, &fpstatus2); \
        _clearfp(); \
        fpstatus |= fpstatus2; \
         \
        ret = ((SW_ZERODIVIDE & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((SW_OVERFLOW & fpstatus) ? UFUNC_FPE_OVERFLOW : 0) \
                | ((SW_UNDERFLOW & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((SW_INVALID & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        }
#endif

/* Solaris --------------------------------------------------------*/
/* --------ignoring SunOS ieee_flags approach, someone else can
**         deal with that! */
#elif defined(sun) || defined(__BSD__) || defined(__OpenBSD__) || \
      (defined(__FreeBSD__) && (__FreeBSD_version < 502114)) || \
      defined(__NetBSD__)
#include <ieeefp.h>

#define UFUNC_CHECK_STATUS(ret) { \
        int fpstatus; \
         \
        fpstatus = (int) fpgetsticky(); \
        ret = ((FP_X_DZ  & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((FP_X_OFL & fpstatus) ? UFUNC_FPE_OVERFLOW : 0) \
                | ((FP_X_UFL & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((FP_X_INV & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        (void) fpsetsticky(0); \
        }

#elif defined(__GLIBC__) || defined(__APPLE__) || \
      defined(__CYGWIN__) || defined(__MINGW32__) || \
      (defined(__FreeBSD__) && (__FreeBSD_version >= 502114))

#if defined(__GLIBC__) || defined(__APPLE__) || \
    defined(__MINGW32__) || defined(__FreeBSD__)
#include <fenv.h>
#elif defined(__CYGWIN__)
#include "numpy/fenv/fenv.h"
#endif

#define UFUNC_CHECK_STATUS(ret) { \
        int fpstatus = (int) fetestexcept(FE_DIVBYZERO | FE_OVERFLOW | \
                                          FE_UNDERFLOW | FE_INVALID); \
        ret = ((FE_DIVBYZERO  & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((FE_OVERFLOW   & fpstatus) ? UFUNC_FPE_OVERFLOW : 0) \
                | ((FE_UNDERFLOW  & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((FE_INVALID    & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        (void) feclearexcept(FE_DIVBYZERO | FE_OVERFLOW | \
                             FE_UNDERFLOW | FE_INVALID); \
}

#elif defined(_AIX)

#include <float.h>
#include <fpxcp.h>

#define UFUNC_CHECK_STATUS(ret) { \
        fpflag_t fpstatus; \
 \
        fpstatus = fp_read_flag(); \
        ret = ((FP_DIV_BY_ZERO & fpstatus) ? UFUNC_FPE_DIVIDEBYZERO : 0) \
                | ((FP_OVERFLOW & fpstatus) ? UFUNC_FPE_OVERFLOW : 0)   \
                | ((FP_UNDERFLOW & fpstatus) ? UFUNC_FPE_UNDERFLOW : 0) \
                | ((FP_INVALID & fpstatus) ? UFUNC_FPE_INVALID : 0); \
        fp_swap_flag(0); \
}

#else

#define NO_FLOATING_POINT_SUPPORT
#define UFUNC_CHECK_STATUS(ret) { \
    ret = 0; \
  }

#endif

/* Make sure it gets defined if it isn't already */
#ifndef UFUNC_NOFPE
#define UFUNC_NOFPE
#endif

#endif
