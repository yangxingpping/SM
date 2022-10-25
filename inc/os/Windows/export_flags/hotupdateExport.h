
#ifndef HOTUPDATE_EXPORT_H
#define HOTUPDATE_EXPORT_H

#ifdef HOTUPDATE_STATIC_DEFINE
#  define HOTUPDATE_EXPORT
#  define HOTUPDATE_NO_EXPORT
#else
#  ifndef HOTUPDATE_EXPORT
#    ifdef hotupdate_EXPORTS
        /* We are building this library */
#      define HOTUPDATE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define HOTUPDATE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef HOTUPDATE_NO_EXPORT
#    define HOTUPDATE_NO_EXPORT 
#  endif
#endif

#ifndef HOTUPDATE_DEPRECATED
#  define HOTUPDATE_DEPRECATED __declspec(deprecated)
#endif

#ifndef HOTUPDATE_DEPRECATED_EXPORT
#  define HOTUPDATE_DEPRECATED_EXPORT HOTUPDATE_EXPORT HOTUPDATE_DEPRECATED
#endif

#ifndef HOTUPDATE_DEPRECATED_NO_EXPORT
#  define HOTUPDATE_DEPRECATED_NO_EXPORT HOTUPDATE_NO_EXPORT HOTUPDATE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef HOTUPDATE_NO_DEPRECATED
#    define HOTUPDATE_NO_DEPRECATED
#  endif
#endif

#endif /* HOTUPDATE_EXPORT_H */
