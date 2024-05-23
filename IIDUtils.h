//
// Created by Nettal on 2024/5/23.
//

#ifndef PARSEC_IIDUTILS_H
#define PARSEC_IIDUTILS_H

#include <rpc.h>
#include <basetyps.h>

int iid_utils_hexToInt(const char *c, int l);

IID iid_utils_guidFrom(const char *c);

#endif //PARSEC_IIDUTILS_H
