/*
 * obfs_random.c - Implementation of random padding obfuscating
 *
 * Copyright (C) 2017, Reinforce-II <reinforce20001@yahoo.com>
 *
 * This file is part of the simple-obfs.
 *
 * simple-obfs is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * simple-obfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with simple-obfs; see the file COPYING. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <strings.h>
#include <ctype.h> /* isblank() */

#include "base64.h"
#include "utils.h"
#include "obfs_random.h"

static int obfs_random(buffer_t *, size_t, obfs_t *);
static int deobfs_random_header(buffer_t *, size_t, obfs_t *);
static int check_random_header(buffer_t *buf);
static void disable_random(obfs_t *obfs);
static int is_enable_random(obfs_t *obfs);

static obfs_para_t obfs_random_st = {
    .name            = "random",
    .host            = "",
    .port            = 0,
    .send_empty_response_upon_connection = true,

    .obfs_request    = &obfs_random,
    .obfs_response   = &obfs_random,
    .deobfs_request  = &deobfs_random_header,
    .deobfs_response = &deobfs_random_header,
    .check_obfs      = &check_random_header,
    .disable         = &disable_random,
    .is_enable       = &is_enable_random
};

obfs_para_t *obfs_random = &obfs_random_st;

static int
obfs_random(buffer_t *buf, size_t cap, obfs_t *obfs)
{
    if (obfs == NULL || obfs->obfs_stage != 0) return 0;
    
	uint8_t padding_len;
	rand_bytes(&padding_len, 1);
	padding_len <<= 3;
	uint8_t *padding_body = (uint8_t*)malloc(padding_len);
	rand_bytes(padding_body, padding_len);

    size_t obfs_len = 1 + padding_len;
    size_t buf_len = buf->len;

    brealloc(buf, obfs_len + buf_len, cap);

    memmove(buf->data + obfs_len, buf->data, buf_len);
    memcpy(buf->data, &padding_len, 1);
    memcpy(buf->data + 1, &padding_body, padding_len);

    buf->len = obfs_len + buf_len;

    return buf->len;
}

static int
deobfs_random_header(buffer_t *buf, size_t cap, obfs_t *obfs)
{
    if (obfs == NULL || obfs->deobfs_stage != 0) return 0;

    char *data = buf->data;
    int len    = buf->len;
    
    uint8_t padding_len = (uint8_t*)data;
    len -= (1 + padding_len);
    data += (1 + padding_len);
    memmove(buf->data, data, len);
    buf->len = len;

    return 0;
}

static int
check_random_header(buffer_t *buf)
{
    return OBFS_OK;
}

static void
disable_random(obfs_t *obfs)
{
    obfs->obfs_stage = -1;
    obfs->deobfs_stage = -1;
}

static int
is_enable_random(obfs_t *obfs)
{
    return obfs->obfs_stage != -1 && obfs->deobfs_stage != -1;
}
