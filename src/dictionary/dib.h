/* This version of dibio.* is a direct modification of the files in the original 'zstd' repo.
 *
 * "dibio.*" - BSD-style License
 * Copyright (c) Yann Collet, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#pragma once

#define ZDICT_STATIC_LINKING_ONLY
#include <zdict.h>

#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)

struct ZSTD_Dictionary {
    void* data;
    const size_t size;
};

ZSTD_Dictionary DECO_DIB_TrainFromFiles(unsigned maxDictSize,
    const char** fileNamesTable, unsigned nbFiles, size_t chunkSize,
    ZDICT_legacy_params_t* params, ZDICT_cover_params_t* coverParams,
    ZDICT_fastCover_params_t* fastCoverParams, int optimize);