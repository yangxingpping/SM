
#ifndef CONFIGS_EXPORT_H
#define CONFIGS_EXPORT_H

#ifdef CONFIGS_STATIC_DEFINE
#  define CONFIGS_EXPORT
#  define CONFIGS_NO_EXPORT
#else
#  ifndef CONFIGS_EXPORT
#    ifdef configs_EXPORTS
        /* We are building this library */
#      define CONFIGS_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define CONFIGS_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef CONFIGS_NO_EXPORT
#    define CONFIGS_NO_EXPORT 
#  endif
#endif

#ifndef CONFIGS_DEPRECATED
#  define CONFIGS_DEPRECATED __declspec(deprecated)
#endif

#ifndef CONFIGS_DEPRECATED_EXPORT
#  define CONFIGS_DEPRECATED_EXPORT CONFIGS_EXPORT CONFIGS_DEPRECATED
#endif

#ifndef CONFIGS_DEPRECATED_NO_EXPORT
#  define CONFIGS_DEPRECATED_NO_EXPORT CONFIGS_NO_EXPORT CONFIGS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CONFIGS_NO_DEPRECATED
#    define CONFIGS_NO_DEPRECATED
#  endif
#endif

#endif /* CONFIGS_EXPORT_H */
