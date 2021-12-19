#ifndef GMACROS_H_17TQZ2BG
#define GMACROS_H_17TQZ2BG

#define TRACE(probe)

#ifndef __GI_SCANNER__ /* The static assert macro really confuses the introspection parser */
#define G_STATIC_ASSERT(expr) _Static_assert (expr, "Expression evaluates to false")
#endif /* !__GI_SCANNER__ */

#define G_STRINGIFY(macro_or_string)	G_STRINGIFY_ARG (macro_or_string)
#define	G_STRINGIFY_ARG(contents)	#contents
/*
 * Note: Clang (but not clang-cl) defines __GNUC__ and __GNUC_MINOR__.
 * Both Clang 11.1 on current Arch Linux and Apple's Clang 12.0 define
 * __GNUC__ = 4 and __GNUC_MINOR__ = 2. So G_GNUC_CHECK_VERSION(4, 2) on
 * current Clang will be 1.
 */
#ifdef __GNUC__
#define G_GNUC_CHECK_VERSION(major, minor)                                     \
  ((__GNUC__ > (major)) ||                                                     \
   ((__GNUC__ == (major)) && (__GNUC_MINOR__ >= (minor))))
#else
#define G_GNUC_CHECK_VERSION(major, minor) 0
#endif

/*
 * The G_LIKELY and G_UNLIKELY macros let the programmer give hints to
 * the compiler about the expected result of an expression. Some compilers
 * can use this information for optimizations.
 *
 * The _G_BOOLEAN_EXPR macro is intended to trigger a gcc warning when
 * putting assignments in g_return_if_fail ().
 */
#if G_GNUC_CHECK_VERSION(2, 0) && defined(__OPTIMIZE__)
#define _G_BOOLEAN_EXPR(expr)                                                  \
  G_GNUC_EXTENSION({                                                           \
    int _g_boolean_var_;                                                       \
    if (expr)                                                                  \
      _g_boolean_var_ = 1;                                                     \
    else                                                                       \
      _g_boolean_var_ = 0;                                                     \
    _g_boolean_var_;                                                           \
  })
#define G_LIKELY(expr) (__builtin_expect(_G_BOOLEAN_EXPR(expr), 1))
#define G_UNLIKELY(expr) (__builtin_expect(_G_BOOLEAN_EXPR(expr), 0))
#else
#define G_LIKELY(expr) (expr)
#define G_UNLIKELY(expr) (expr)
#endif

/* Here we provide G_GNUC_EXTENSION as an alias for __extension__,
 * where this is valid. This allows for warningless compilation of
 * "long long" types even in the presence of '-ansi -pedantic'.
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#define G_GNUC_EXTENSION __extension__
#else
#define G_GNUC_EXTENSION
#endif

#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* Provide a string identifying the current function, non-concatenatable */
#if defined(__GNUC__) && defined(__cplusplus)
#define G_STRFUNC ((const char *)(__PRETTY_FUNCTION__))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define G_STRFUNC ((const char *)(__func__))
#elif defined(__GNUC__) || (defined(_MSC_VER) && (_MSC_VER > 1300))
#define G_STRFUNC ((const char *)(__FUNCTION__))
#else
#define G_STRFUNC ((const char *)("???"))
#endif

#define G_GNUC_PRINTF(format_idx, arg_idx)                                     \
  __attribute__((__format__(__printf__, format_idx, arg_idx)))

/* Provide a string identifying the current code position */
#if defined(__GNUC__) && (__GNUC__ < 3) && !defined(__cplusplus)
#define G_STRLOC __FILE__ ":" G_STRINGIFY(__LINE__) ":" __PRETTY_FUNCTION__ "()"
#else
#define G_STRLOC __FILE__ ":" G_STRINGIFY(__LINE__)
#endif

/*
 * Attribute support detection. Works on clang and GCC >= 5
 * https://clang.llvm.org/docs/LanguageExtensions.html#has-attribute
 * https://gcc.gnu.org/onlinedocs/cpp/_005f_005fhas_005fattribute.html
 */

#ifdef __has_attribute
#define g_macro__has_attribute __has_attribute
#else

/*
 * Fallback for GCC < 5 and other compilers not supporting __has_attribute.
 */
#define g_macro__has_attribute(x) g_macro__has_attribute_##x

#define g_macro__has_attribute___pure__ G_GNUC_CHECK_VERSION(2, 96)
#define g_macro__has_attribute___malloc__ G_GNUC_CHECK_VERSION(2, 96)
#define g_macro__has_attribute___noinline__ G_GNUC_CHECK_VERSION(2, 96)
#define g_macro__has_attribute___sentinel__ G_GNUC_CHECK_VERSION(4, 0)
#define g_macro__has_attribute___alloc_size__ G_GNUC_CHECK_VERSION(4, 3)
#define g_macro__has_attribute___format__ G_GNUC_CHECK_VERSION(2, 4)
#define g_macro__has_attribute___format_arg__ G_GNUC_CHECK_VERSION(2, 4)
#define g_macro__has_attribute___noreturn__                                    \
  (G_GNUC_CHECK_VERSION(2, 8) || (0x5110 <= __SUNPRO_C))
#define g_macro__has_attribute___const__ G_GNUC_CHECK_VERSION(2, 4)
#define g_macro__has_attribute___unused__ G_GNUC_CHECK_VERSION(2, 4)
#define g_macro__has_attribute___no_instrument_function__                      \
  G_GNUC_CHECK_VERSION(2, 4)
#define g_macro__has_attribute_fallthrough G_GNUC_CHECK_VERSION(6, 0)
#define g_macro__has_attribute___deprecated__ G_GNUC_CHECK_VERSION(3, 1)
#define g_macro__has_attribute_may_alias G_GNUC_CHECK_VERSION(3, 3)
#define g_macro__has_attribute_warn_unused_result G_GNUC_CHECK_VERSION(3, 4)

#endif

#if g_macro__has_attribute(__pure__)
#define G_GNUC_PURE __attribute__((__pure__))
#else
#define G_GNUC_PURE
#endif

#if g_macro__has_attribute(__malloc__)
#define G_GNUC_MALLOC __attribute__((__malloc__))
#else
#define G_GNUC_MALLOC
#endif

/**
 * G_GNUC_ALLOC_SIZE2:
 * @x: the index of the argument specifying one factor of the allocation size
 * @y: the index of the argument specifying the second factor of the allocation
 * size
 *
 * Expands to the GNU C `alloc_size` function attribute if the compiler is a
 * new enough gcc. This attribute tells the compiler that the function returns
 * a pointer to memory of a size that is specified by the product of two
 * function parameters.
 *
 * Place the attribute after the function declaration, just before the
 * semicolon.
 *
 * |[<!-- language="C" -->
 * gpointer g_malloc_n (gsize n_blocks,
 *                      gsize n_block_bytes) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE2(1,
 * 2);
 * ]|
 *
 * See the [GNU C
 * documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alloc_005fsize-function-attribute)
 * for more details.
 *
 * Since: 2.18
 */
#if g_macro__has_attribute(__alloc_size__)
#define G_GNUC_ALLOC_SIZE(x) __attribute__((__alloc_size__(x)))
#define G_GNUC_ALLOC_SIZE2(x, y) __attribute__((__alloc_size__(x, y)))
#else
#define G_GNUC_ALLOC_SIZE(x)
#define G_GNUC_ALLOC_SIZE2(x, y)
#endif

/**
 * G_GNUC_WARN_UNUSED_RESULT:
 *
 * Expands to the GNU C `warn_unused_result` function attribute if the compiler
 * is gcc. This function attribute makes the compiler emit a warning if the
 * result of a function call is ignored.
 *
 * Place the attribute after the declaration, just before the semicolon.
 *
 * |[<!-- language="C" -->
 * GList *g_list_append (GList *list,
 *                       gpointer data) G_GNUC_WARN_UNUSED_RESULT;
 * ]|
 *
 * See the [GNU C
 * documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-warn_005funused_005fresult-function-attribute)
 * for more details.
 *
 * Since: 2.10
 */
#if g_macro__has_attribute(warn_unused_result)
#define G_GNUC_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define G_GNUC_WARN_UNUSED_RESULT
#endif /* __GNUC__ */

#define G_STMT_START do
#define G_STMT_END while (0)

#endif /* end of include guard: GMACROS_H_17TQZ2BG */
