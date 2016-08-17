
#include "pmm.h"
#include "vmm.h"
#include "common.h"
#include <cstring>
#include "../Console.h"

uint32_t pmm_location;

//index of first free page
int _first_free_index = 0; 
//index of last free page. That page cannot be used
int _last_available_index = 0; 

//size of all available pages 
uint8_t *_bitmap;

void set_bit(int b) { 
	// apskaičiuojam indeksa ir per kiek vienetu pastumti
	_bitmap[b/8] |= 1 << (b % 8);
}
void clear_bit(int b) { 
	// invertuojam, kad neuznulintumem pries tai einanciu indeksu ir uznulinam
    _bitmap[b/8] &= ~(1 << (b % 8));
}

int get_bit(int b) { 
    return _bitmap[b/8] & (1 << (b % 8));
}

// #define GET_BITMAP_VALUE(bitmap, index) bitmap / index //

char pmm_paging_active = 0;


void init_pmm (uint32_t start)
{
  // Ensure the initial page allocation location is page-aligned.
  // start of bitmap. used in vmm
  pmm_location = (start + 0x1000) & PAGE_MASK;
  _bitmap = (uint8_t *)pmm_location; 
}

// if paging active and there is first_free_index 
// set bitmap by index to 1, update first_free_index
// return bitmap_base + allocated_block_size
// else return last_allocation + block_size
uint32_t pmm_alloc_page()
{	
	// IO::Console::kprint("\npmm_alloc_page() stack: (");
	
	if(pmm_paging_active)
	{	
		
		// if index out bounds
		// _last_available_index - negali buti pasiekiamas
		if(_first_free_index == (_last_available_index))
			panic ("Error:out of memory.");
	
		// _bitmap[_first_free_index] = 1;
		set_bit(_first_free_index);
		
		//from start of bitmap count taken blocks
		uint32_t stack = (uint32_t)
			(_bitmap + _first_free_index * MEM_UNIT_SIZE);
		// IO::Console::kprinthex((int)stack);
		// while((_first_free_index != _last_available_index) && (_bitmap[++_first_free_index] != 0)) ;
		while((_first_free_index != _last_available_index) && (get_bit(++_first_free_index) != 0)) ;	
	
		// IO::Console::kprint(" )\n _first_free_index = ");
		// IO::Console::kprint(_first_free_index);
		// IO::Console::kprintln("");
		
		return stack;
	}
	else
	{
		// pass next block. _bitmap[0] is skipped
		// IO::Console::kprinthex((int)(pmm_location + 0x1000));
		// IO::Console::kprintln(")");
		return pmm_location += 0x1000;
	}
}


void *pmm_get_location()
{
	return _bitmap;
}

// maps bitmap in virtual space:
// finds how many blocks were allocated by vmm
// set taken blocks by indeses to 0, others to 1
// set _first_free_index to taken block count
// set _last_available_index to all block count
void map_pmm_to_virtual_space()
{	
		IO::Console::kprintln("13++");
	map((uint32_t)_bitmap, (uint32_t)_bitmap, PAGE_PRESENT | PAGE_WRITE);
	
		IO::Console::kprintln("13--");
	// indices where allocated by vmm for table
	uint32_t skipped_indices = (pmm_location - (uint32_t)_bitmap) / MEM_UNIT_SIZE + 1;
	// allocated bytes - 1, other - 0
	for (uint32_t i = 0; i < skipped_indices; i++, set_bit(i));
	
	uint32_t rest_of_table = MEM_SIZE / MEM_UNIT_SIZE - skipped_indices;
	for (uint32_t i = 0; i < rest_of_table; i++)
		clear_bit(i + skipped_indices);
	
	set_bit(MEM_SIZE / MEM_UNIT_SIZE);
	
	_first_free_index = skipped_indices;
	_last_available_index = MEM_SIZE / MEM_UNIT_SIZE; 
}

// finds index in bitmap, sets to 0, updates first_free_index 
void pmm_free_page (uint32_t page_address)
{
	IO::Console::kprint("free page() addr: (");
	IO::Console::kprinthex((int)page_address);

	// allocated space - allocation start
	uint32_t addr_on_bitmap = page_address - (uint32_t)_bitmap;
    int bitmap_index = addr_on_bitmap / MEM_UNIT_SIZE;
  
	IO::Console::kprint(") bitmap_index =");
	IO::Console::kprint(bitmap_index);
	
	// _bitmap[bitmap_index] = 0;
	clear_bit(bitmap_index);
	if (_first_free_index > bitmap_index)
		_first_free_index = bitmap_index;		
		
	IO::Console::kprintln("");
	IO::Console::kprint("first_free_index = ");
	IO::Console::kprint(_first_free_index);
	IO::Console::kprintln("");
}

