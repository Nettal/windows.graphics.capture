//
// Created by Nettal on 2024/5/23.
//

#include "IIDUtils.h"

IID iid_utils_guidFrom(const char *c) {
    return (IID) {iid_utils_hexToInt(c, 8), iid_utils_hexToInt(c + 9, 4), iid_utils_hexToInt(c + 14, 4), {
            (unsigned char) iid_utils_hexToInt(c + 19, 2),
            (unsigned char) iid_utils_hexToInt(c + 21, 2),
            (unsigned char) iid_utils_hexToInt(c + 24, 2),
            (unsigned char) iid_utils_hexToInt(c + 26, 2),
            (unsigned char) iid_utils_hexToInt(c + 28, 2),
            (unsigned char) iid_utils_hexToInt(c + 30, 2),
            (unsigned char) iid_utils_hexToInt(c + 32, 2),
            (unsigned char) iid_utils_hexToInt(c + 34, 2),
    }};
}

int iid_utils_hexToInt(const char *c, int l) {
    int result = 0;
    for (int i = 0; i < l; ++i) {
        if (c[i] >= '0' && c[i] <= '9') {
            result *= 16;
            result += c[i] - '0';
        }
        if (c[i] >= 'A' && c[i] <= 'F') {
            result *= 16;
            result += c[i] - 'A' + 10;
        }

        if (c[i] >= 'a' && c[i] <= 'f') {
            result *= 16;
            result += c[i] - 'a' + 10;
        }
    }
    return result;
}
