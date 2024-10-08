/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2018 Red Hat, Inc.
 */

#ifndef __NM_SECRET_UTILS_H__
#define __NM_SECRET_UTILS_H__

#include "nm-macros-internal.h"

/*****************************************************************************/

void nm_explicit_bzero(void *s, gsize n);

/*****************************************************************************/

char *nm_secret_strchomp(char *secret);

/*****************************************************************************/

void nm_free_secret(char *secret);

static inline gboolean
nm_strdup_reset_secret(char **dst, const char *src)
{
    char *old;

    nm_assert(dst);

    if (nm_streq0(*dst, src))
        return FALSE;
    old  = *dst;
    *dst = src ? g_strdup(src) : NULL;
    if (old)
        nm_free_secret(old);
    return TRUE;
}

NM_AUTO_DEFINE_FCN0(char *, _nm_auto_free_secret, nm_free_secret);
/**
 * nm_auto_free_secret:
 *
 * Call g_free() on a variable location when it goes out of scope.
 * Also, previously, calls memset(loc, 0, strlen(loc)) to clear out
 * the secret.
 */
#define nm_auto_free_secret nm_auto(_nm_auto_free_secret)

/*****************************************************************************/

GBytes *nm_secret_copy_to_gbytes(gconstpointer mem, gsize mem_len);

/*****************************************************************************/

/* NMSecretPtr is a pair of malloc'ed data pointer and the length of the
 * data. The purpose is to use it in combination with nm_auto_clear_secret_ptr
 * which ensures that the data pointer (with all len bytes) is cleared upon
 * cleanup. */
typedef struct {
    gsize len;

    /* the data pointer. This pointer must be allocated with malloc (at least
     * when used with nm_secret_ptr_clear()). */
    union {
        char   *str;
        void   *ptr;
        guint8 *bin;
    };
} NMSecretPtr;

static inline void
nm_secret_ptr_bzero(NMSecretPtr *secret)
{
    if (secret) {
        if (secret->len > 0) {
            if (secret->ptr)
                nm_explicit_bzero(secret->ptr, secret->len);
        }
    }
}

#define nm_auto_bzero_secret_ptr nm_auto(nm_secret_ptr_bzero)

static inline void
nm_secret_ptr_clear(NMSecretPtr *secret)
{
    if (secret) {
        if (secret->len > 0) {
            if (secret->ptr)
                nm_explicit_bzero(secret->ptr, secret->len);
            secret->len = 0;
        }
        nm_clear_g_free(&secret->ptr);
    }
}

#define nm_auto_clear_secret_ptr nm_auto(nm_secret_ptr_clear)

#define NM_SECRET_PTR_INIT() \
    ((const NMSecretPtr) {   \
        .len = 0,            \
        .ptr = NULL,         \
    })

#define NM_SECRET_PTR_STATIC(_len)  \
    ((const NMSecretPtr) {          \
        .len = _len,                \
        .ptr = ((guint8[_len]) {}), \
    })

#define NM_SECRET_PTR_ARRAY(_arr)                      \
    ((const NMSecretPtr) {                             \
        .len = G_N_ELEMENTS(_arr) * sizeof((_arr)[0]), \
        .ptr = &((_arr)[0]),                           \
    })

static inline void
nm_secret_ptr_clear_static(const NMSecretPtr *secret)
{
    if (secret) {
        if (secret->len > 0) {
            nm_assert(secret->ptr);
            nm_explicit_bzero(secret->ptr, secret->len);
        }
    }
}

#define nm_auto_clear_static_secret_ptr nm_auto(nm_secret_ptr_clear_static)

static inline void
nm_secret_ptr_move(NMSecretPtr *dst, NMSecretPtr *src)
{
    if (dst && dst != src) {
        *dst     = *src;
        src->len = 0;
        src->ptr = NULL;
    }
}

/*****************************************************************************/

typedef struct {
    const gsize len;
    union {
        char   str[0];
        guint8 bin[0];
    };
} NMSecretBuf;

static inline void
_nm_auto_free_secret_buf(NMSecretBuf **ptr)
{
    NMSecretBuf *b = *ptr;

    if (b) {
        nm_assert(b->len > 0);
        nm_explicit_bzero(b->bin, b->len);
        g_free(b);
    }
}
#define nm_auto_free_secret_buf nm_auto(_nm_auto_free_secret_buf)

NMSecretBuf *nm_secret_buf_new(gsize len);

GBytes *nm_secret_buf_to_gbytes_take(NMSecretBuf *secret, gssize actual_len);

/*****************************************************************************/

gboolean nm_utils_memeqzero_secret(gconstpointer data, gsize length);

/*****************************************************************************/

/**
 * nm_secret_mem_realloc:
 * @m_old: the current buffer of length @cur_len.
 * @do_bzero_mem: if %TRUE, bzero the old buffer
 * @cur_len: the current buffer length of @m_old. It is necessary for bzero.
 * @new_len: the desired new length
 *
 * If @do_bzero_mem is false, this is like g_realloc().
 * Otherwise, this will allocate a new buffer of the desired size, copy over the
 * old data, and bzero the old buffer before freeing it. As such, it also behaves
 * similar to g_realloc(), with the overhead of nm_explicit_bzero() and using
 * malloc/free instead of realloc().
 *
 * Returns: the new allocated buffer. Think of it behaving like g_realloc().
 */
static inline gpointer
nm_secret_mem_realloc(gpointer m_old, gboolean do_bzero_mem, gsize cur_len, gsize new_len)
{
    gpointer m_new;

    nm_assert(m_old || cur_len == 0);

    if (do_bzero_mem && G_LIKELY(cur_len > 0)) {
        m_new = g_malloc(new_len);
        if (G_LIKELY(new_len > 0))
            memcpy(m_new, m_old, NM_MIN(cur_len, new_len));
        nm_explicit_bzero(m_old, cur_len);
        g_free(m_old);
    } else
        m_new = g_realloc(m_old, new_len);

    return m_new;
}

/**
 * nm_secret_mem_try_realloc:
 * @m_old: the current buffer of length @cur_len.
 * @do_bzero_mem: if %TRUE, bzero the old buffer
 * @cur_len: the current buffer length of @m_old. It is necessary for bzero.
 * @new_len: the desired new length
 *
 * If @do_bzero_mem is false, this is like g_try_realloc().
 * Otherwise, this will try to allocate a new buffer of the desired size, copy over the
 * old data, and bzero the old buffer before freeing it. As such, it also behaves
 * similar to g_try_realloc(), with the overhead of nm_explicit_bzero() and using
 * malloc/free instead of realloc().
 *
 * Returns: the new allocated buffer or NULL. Think of it behaving like g_try_realloc().
 */
static inline gpointer
nm_secret_mem_try_realloc(gpointer m_old, gboolean do_bzero_mem, gsize cur_len, gsize new_len)
{
    gpointer m_new;

    nm_assert(m_old || cur_len == 0);

    if (do_bzero_mem && G_LIKELY(cur_len > 0)) {
        if (G_UNLIKELY(new_len == 0))
            m_new = NULL;
        else {
            m_new = g_try_malloc(new_len);
            if (!m_new)
                return NULL;
            memcpy(m_new, m_old, NM_MIN(cur_len, new_len));
        }
        nm_explicit_bzero(m_old, cur_len);
        g_free(m_old);
        return m_new;
    }

    return g_try_realloc(m_old, new_len);
}

/**
 * nm_secret_mem_try_realloc_take:
 * @m_old: the current buffer of length @cur_len.
 * @do_bzero_mem: if %TRUE, bzero the old buffer
 * @cur_len: the current buffer length of @m_old. It is necessary for bzero.
 * @new_len: the desired new length
 *
 * This works like nm_secret_mem_try_realloc(), which is not unlike g_try_realloc().
 * The difference is, if we fail to allocate a new buffer, then @m_old will be
 * freed (and possibly cleared). This differs from plain realloc(), where the
 * old buffer is unchanged if the operation fails.
 *
 * Returns: the new allocated buffer or NULL. Think of it behaving like g_try_realloc()
 *   but it will always free @m_old.
 */
static inline gpointer
nm_secret_mem_try_realloc_take(gpointer m_old, gboolean do_bzero_mem, gsize cur_len, gsize new_len)
{
    gpointer m_new;

    nm_assert(m_old || cur_len == 0);

    if (do_bzero_mem && G_LIKELY(cur_len > 0)) {
        if (G_UNLIKELY(new_len == 0))
            m_new = NULL;
        else {
            m_new = g_try_malloc(new_len);
            if (G_LIKELY(m_new))
                memcpy(m_new, m_old, NM_MIN(cur_len, new_len));
        }
        nm_explicit_bzero(m_old, cur_len);
        g_free(m_old);
        return m_new;
    }

    m_new = g_try_realloc(m_old, new_len);
    if (G_UNLIKELY(!m_new && new_len > 0))
        g_free(m_old);
    return m_new;
}

/*****************************************************************************/

gboolean nm_utils_read_crypto_file(const char *filename, NMSecretPtr *out_contents, GError **error);

GBytes *nm_utils_read_crypto_file_to_bytes(const char *filename, GError **error);

#endif /* __NM_SECRET_UTILS_H__ */
