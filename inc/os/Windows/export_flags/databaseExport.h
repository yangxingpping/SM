
#ifndef DATABASE_EXPORT_H
#define DATABASE_EXPORT_H

#ifdef DATABASE_STATIC_DEFINE
#  define DATABASE_EXPORT
#  define DATABASE_NO_EXPORT
#else
#  ifndef DATABASE_EXPORT
#    ifdef database_EXPORTS
        /* We are building this library */
#      define DATABASE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define DATABASE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef DATABASE_NO_EXPORT
#    define DATABASE_NO_EXPORT 
#  endif
#endif

#ifndef DATABASE_DEPRECATED
#  define DATABASE_DEPRECATED __declspec(deprecated)
#endif

#ifndef DATABASE_DEPRECATED_EXPORT
#  define DATABASE_DEPRECATED_EXPORT DATABASE_EXPORT DATABASE_DEPRECATED
#endif

#ifndef DATABASE_DEPRECATED_NO_EXPORT
#  define DATABASE_DEPRECATED_NO_EXPORT DATABASE_NO_EXPORT DATABASE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef DATABASE_NO_DEPRECATED
#    define DATABASE_NO_DEPRECATED
#  endif
#endif

#endif /* DATABASE_EXPORT_H */
