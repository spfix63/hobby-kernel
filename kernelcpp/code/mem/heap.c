

#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "../Console.h"

#include <stddef.h>

/* static void alloc_chunk (uint32_t start, uint32_t len);
static void free_chunk (header_t *chunk); */
static void split_chunk (header_t *chunk, uint32_t len);
/*
static void glue_chunk (header_t *chunk);

uint32_t heap_max = HEAP_START; */

// uint32_t current_mem_addr;

// int32_t size_array[11];
// header_t *list_array[11];


header_t *find_nonstandart_node(header_t *node, uint32_t length, header_t **last_node);
header_t *place_nonstandart_node(header_t *nonstandart_node, header_t *node, uint32_t length);

// header_t *heap_first = 0;



// inicijuojam dydziu masyva ir sarasu masyva
// uzsiimam pirma puslapi sau
/*void init_heap ()
{
	current_mem_addr = HEAP_START;

	size_array[0] = 16;
	size_array[1] = 32;
	size_array[2] = 48;
	size_array[3] = 64;
	size_array[4] = 80;
	size_array[5] = 96;
	size_array[6] = 112;
	size_array[7] = 128;
	size_array[8] = 256;
	size_array[9] = 512;
	size_array[10] = -1;
	for (int i = 0; i < 11; i++)
		list_array[i] = NULL;
	
	uint32_t page = pmm_alloc_page ();
	map (current_mem_addr, page, PAGE_PRESENT | PAGE_WRITE);
}
*/

Heap *Heap::currentHeap;
	
Heap::Heap()
{
	current_mem_addr = HEAP_START;

	size_array[0] = 16;
	size_array[1] = 32;
	size_array[2] = 48;
	size_array[3] = 64;
	size_array[4] = 80;
	size_array[5] = 96;
	size_array[6] = 112;
	size_array[7] = 128;
	size_array[8] = 256;
	size_array[9] = 512;
	size_array[10] = -1;
	for (int i = 0; i < 11; i++)
		list_array[i] = NULL;
	
	uint32_t page = pmm_alloc_page ();
	map (current_mem_addr, page, PAGE_PRESENT | PAGE_WRITE);
	IO::Console::kprintln("n2");
}

// palygindamas prasoma dydi su dydziais size_array,
// suranda indeksa sarasu, masyve, prie kurio bus prideta 
// isskirta atmintis.
// jei prasomas atminties dydis nestandartinis(didesnis uz numatytus),
// dedamas prie paskutinio masyvo nario
void *Heap::kmalloc (uint32_t length)
{
	if ((int) length < 0)
		return NULL;
		
	int index;
	//ieskome tinkamo dydzio indekso
	for (index = 0; index < 10; index++)
		if ((int) length < size_array[index])
			break;
			
	header_t *node = list_array[index];
	header_t *last_node = NULL;
	
	// jei nestandartinis dydis
	if (index == 10)
	{		
		IO::Console::kprintln("nonstandart");	
		// neapibreztu dydziu masyve:
		// -isskaidyti i dalis, jei dydis mazesnis, nei turim
		// -sujungti kelis jei dydis per didelis
		// -jei ne vienas, iskirti kaip iprasta
		header_t *nonstandart_node = NULL;
		nonstandart_node = find_nonstandart_node(node, length, &last_node);
			// jei radom node, i ji bandom idedet, jei ne
			// darom taip kaip su standartiniu
			// if (node->length - length > sizeof(header_t))
		if (nonstandart_node) 
		{
			split_chunk(nonstandart_node, length+sizeof(header_t));
			nonstandart_node->allocated = 1;
			return nonstandart_node + 1;			
		}
			// if (nonstandart_node)
				// nonstandart_node = place_nonstandart_node(nonstandart_node, node, length);
			//neradom, elgiames kaip su standartiniu
			// else 
				// break;
		// kaip gauti last_node?
		//ieskoti is naujo, jei nepavyko sutalpinti
		// bet kaip tada zinoti
	}	
	else 
	{		
		// IO::Console::kprintln("standart");	
		// IO::Console::kprint("array index:");	
		// IO::Console::kprint(index);	
		// IO::Console::kprintln("");	
		//ieskome laisvo bloko
		while (node != NULL)
		{
			if (node->allocated == 0)
			{
				node->allocated = 1;
				return node + 1;
			}
			last_node = node;
			node = node->next;
		}
	}
	
	
		
	//susikuriam nauja bloka
	int chunk_size = size_array[index] == -1 ? length : size_array[index];
	header_t *new_node_header = (header_t *)current_mem_addr;
	
	new_node_header->next = NULL;
	new_node_header->allocated = 1;
	new_node_header->length = chunk_size;
	chunk_size += sizeof(header_t);
	
	// patikrinam ar reikes prasyti daugiau puslapiu
	int next_page_addr = (current_mem_addr & 0xFFFFF000) + 0x1000;
	int left_in_page = (next_page_addr - current_mem_addr);
	int pages = (chunk_size - left_in_page) / 0x1000 + 1;
	
	for (int i = 0; i < pages; i++)
	{
		uint32_t page = pmm_alloc_page ();
		map (next_page_addr + i*0x1000, page, PAGE_PRESENT | PAGE_WRITE);
	}
		
	current_mem_addr += chunk_size;
	// pridedam naujai allokuota vieta prie saraso
	if (last_node != NULL)
		last_node->next = new_node_header;
	else
		list_array[index] = new_node_header;	
	
	return new_node_header + 1;
}

// pazymim, kad atmintis nebenaudojama, net jos neneaikinam
void Heap::kfree (void *p)
{
	IO::Console::kprint("kfree addr: (");
	IO::Console::kprinthex((int)p);
	IO::Console::kprintln(")");
	header_t *chunk = (header_t *)p;
	chunk--; //p yra atminties adresas, persokam i headerio pradzia
	chunk->allocated = 0;
}

//surandam atminties viena nestandartiniu dydziu masyve
header_t *find_nonstandart_node(header_t *node, uint32_t length, header_t **last_node)
{
	while (node != NULL)
	{
		
		if (!node->allocated)
		{
			if (*last_node && !(*last_node)->allocated) 
			{
				(*last_node)->length = (*last_node)->length + node->length;
				(*last_node)->next = node->next;
				node = *last_node;
			}
				
			if (node->length >= length) 
			{
				// IO::Console::kprintln("rastas laisvas");	
				break;
			}
		}		
		*last_node = node;
		node = node->next;
	}
	return node;
}

//issikaidom arba suglaudziam nestandartinio masyvo elementus,
//kad isskirtumem atminties tiek kiek tiksliai reikia 
//galbut laikyti ji surusiuota nuo maziausios iki didziausios skyles?
// parametrai:
// nonstandart_node - skyle i kuria bandom idet
// node - sakninis node, naudojamas perejimui
// length - kie issikirti vietos
// ka grazinti?
header_t *place_nonstandart_node(header_t *nonstandart_node, header_t *node, uint32_t length)
{
	header_t *return_node = NULL;
	
	//need to merge. if merge not possible,
	//look for another node
	if (nonstandart_node->length < length) 
	{
		IO::Console::kprintln("merge");	
		header_t* temp = nonstandart_node;
		uint32_t accum = 0;
		// einam nuo duoto node ir bandom surasti ar sujungus sekancius laisvus galesim italpinti
		while ((temp != NULL) && (temp->allocated != 1) && (accum < length))
		{
			//sujungiam i didele skyle
			accum += nonstandart_node->length;
			temp = temp->next;
		}
		//nera pakankamai bloku, kad galetumem iskirti atminti, griztame
		if (accum < length)
			return NULL;
		nonstandart_node->length = accum;
		nonstandart_node->next = temp;
	}
	//jei mes turime pakankamai didele skyle arba
	//sugebejom ja pasidaryti sujungdami gretimus
	if (nonstandart_node->length > length)
	{
		IO::Console::kprintln("split");	
		// paimta is pvz
		// ar mum tikrai reikia tikrinimo, ar visgi paliekam fragmentacija?
		if (nonstandart_node->length - length > sizeof (header_t))
		{
			header_t *leftovers_node = (header_t *) ((uint32_t)nonstandart_node + nonstandart_node->length);
			leftovers_node->next = nonstandart_node->next;
			leftovers_node->allocated = 0;
			leftovers_node->length = nonstandart_node->length - length;

			nonstandart_node->next = leftovers_node;
			nonstandart_node->length = length;
			return_node = nonstandart_node;
		}
	}
	//just assign it
	else 
	{
		IO::Console::kprintln("match");	
		node->allocated = 1;
		return_node = node;
	}
	return return_node + 1;
}

/*
void alloc_chunk (uint32_t start, uint32_t len)
{
  while (start + len > heap_max)
  {
    uint32_t page = pmm_alloc_page ();
    map (heap_max, page, PAGE_PRESENT | PAGE_WRITE);
    heap_max += 0x1000;
  }
}

void free_chunk (header_t *chunk)
{
  chunk->prev->next = 0;

  if (chunk->prev == 0)
  heap_first = 0;

  // While the heap max can contract by a page and still be greater than the chunk address...
  while ( (heap_max-0x1000) >= (uint32_t)chunk )
  {
    heap_max -= 0x1000;
    uint32_t page;
    get_mapping (heap_max, &page);
    pmm_free_page (page);
    unmap (heap_max);
  }
}
*/
void split_chunk (header_t *chunk, uint32_t len)
{
  // In order to split a chunk, once we split we need to know that there will be enough
  // space in the new chunk to store the chunk header, otherwise it just isn't worthwhile.
  if (chunk->length - len > sizeof (header_t))
  {
    header_t *newchunk = (header_t *) ((uint32_t)chunk + chunk->length);
    // newchunk->prev = chunk;
    newchunk->next = 0;
    newchunk->allocated = 0;
    newchunk->length = chunk->length - len;

    chunk->next = newchunk;
    chunk->length = len;
  }
}
/*
void glue_chunk (header_t *chunk)
{
  if (chunk->next && chunk->next->allocated == 0)
  {
    chunk->length = chunk->length + chunk->next->length;
    chunk->next->next->prev = chunk;
    chunk->next = chunk->next->next;
  }

  if (chunk->prev && chunk->prev->allocated == 0)
  {
    chunk->prev->length = chunk->prev->length + chunk->length;
    chunk->prev->next = chunk->next;
    chunk->next->prev = chunk->prev;
    chunk = chunk->prev;
  }

  if (chunk->next == 0)
    free_chunk (chunk);
}*/
