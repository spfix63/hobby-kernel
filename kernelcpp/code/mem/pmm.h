
#ifndef PMM_H
#define PMM_H

#include "common.h"

#define MEM_UNIT_SIZE 0x00001000
#define MEM_SIZE 0x00800000

void init_pmm (uint32_t start);

uint32_t pmm_alloc_page ();

void pmm_free_page (uint32_t p);

void map_pmm_to_virtual_space();


#endif
