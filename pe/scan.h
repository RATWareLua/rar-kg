#ifndef KEYGEN_SCAN_H
#define KEYGEN_SCAN_H

#include "pe.h"

enum { SCAN_REF_LIMIT = 65536 };
int scan_refs(const pe_image *image, long refs[SCAN_REF_LIMIT]);

#endif
