/*
 *******************************************************************
 *
 * Copyright 2016 Intel Corporation All rights reserved.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

#ifndef _SHA2_H
#define _SHA2_H

#include <stdint.h>
#include <dps/dbg.h>
#include <dps/err.h>

#define DPS_SHA2_DIGEST_LEN 32

/**
 * Compute the SHA2 hash of some data
 *
 * @param digest  The result
 * @param data    The data to hash
 * @param len     The length of the data to hash
 */
void DPS_Sha2(uint8_t digest[DPS_SHA2_DIGEST_LEN], const uint8_t* data, size_t len);

#endif
