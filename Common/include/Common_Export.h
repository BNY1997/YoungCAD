
#ifndef Common_EXPORT_H
#define Common_EXPORT_H

#ifdef Common_STATIC_DEFINE
#  define Common_EXPORT
#  define COMMON_NO_EXPORT
#else
#  ifndef Common_EXPORT
#    ifdef Common_EXPORTS
        /* We are building this library */
#      define Common_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define Common_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef COMMON_NO_EXPORT
#    define COMMON_NO_EXPORT 
#  endif
#endif

#ifndef COMMON_DEPRECATED
#  define COMMON_DEPRECATED __declspec(deprecated)
#endif

#ifndef COMMON_DEPRECATED_EXPORT
#  define COMMON_DEPRECATED_EXPORT Common_EXPORT COMMON_DEPRECATED
#endif

#ifndef COMMON_DEPRECATED_NO_EXPORT
#  define COMMON_DEPRECATED_NO_EXPORT COMMON_NO_EXPORT COMMON_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef COMMON_NO_DEPRECATED
#    define COMMON_NO_DEPRECATED
#  endif
#endif

#endif /* Common_EXPORT_H */
