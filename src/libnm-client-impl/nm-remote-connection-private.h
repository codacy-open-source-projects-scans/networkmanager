/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2009 Red Hat, Inc.
 */

#ifndef __NM_REMOTE_CONNECTION_PRIVATE_H__
#define __NM_REMOTE_CONNECTION_PRIVATE_H__

#if !((NETWORKMANAGER_COMPILATION) & NM_NETWORKMANAGER_COMPILATION_WITH_LIBNM_PRIVATE)
#error Cannot use this header.
#endif

#define NM_REMOTE_CONNECTION_INIT_RESULT "init-result"

typedef enum {
    NM_REMOTE_CONNECTION_INIT_RESULT_UNKNOWN = 0,
    NM_REMOTE_CONNECTION_INIT_RESULT_SUCCESS,
    NM_REMOTE_CONNECTION_INIT_RESULT_ERROR,
    NM_REMOTE_CONNECTION_INIT_RESULT_INVISIBLE,
} NMRemoteConnectionInitResult;

#endif /* __NM_REMOTE_CONNECTION_PRIVATE__ */