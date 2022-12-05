#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

//  A page table entry (PTE) has the 
//  corresponding physical page number. If the valid bit is not set,
//  the the PTE is invalid
typedef struct {
        unsigned char valid: 1;
        unsigned char physical_page_no: 4;
} PageTableEntry;

//  The page table has 16 entries
#define PAGE_TABLE_SIZE 16

//  A page table is an array of page table entriess
typedef struct {
    PageTableEntry entries[PAGE_TABLE_SIZE];
} PageTable;

//  A TLB entry consists of the following fields
//  1 bit that indicates whether the TLB entry is valid
//  2 bits indicating the last used order
//  4 bits for the physical page
typedef struct
{
    unsigned int valid: 1;
    unsigned short order: 2;
    unsigned short virtual_page_no: 4;
    unsigned short physical_page_no: 4;
} TLB_Entry;

//  The numeber of TLB entries in the TLB
#define TLB_SIZE    4

//  A TLB is an array of entries of TLB_SIZE
typedef struct 
{
    TLB_Entry entries[TLB_SIZE];
} TLB;

//  might be also given with some contents to fill in
void PTE_init(PageTable* ptab) {
    // memset(pt->entries, 0, sizeof(pt->entries));
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        ptab->entries[i].valid = 1;
        ptab->entries[i].physical_page_no = i;
    }
}

void TLB_init(TLB* tlb) {
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb->entries[i].valid = 1;
        tlb->entries[i].order = i;
        tlb->entries[i].virtual_page_no = 0;
        tlb->entries[i].physical_page_no = 0;
    }
}

//  front 4 bits is the virtual page number
unsigned short to_page(u_int16_t v_address) {
    // unsigned short mask = (1 << 4) - 1;
    return (unsigned short) v_address >> 12;
    //  return (unsigned short) ((v_address >> 12) & 0x0F), access last 4 bits,
    //  (v_address >> 12) & 0x0F | val will set it to val
    //  0xF0 will access the front 4 bits
}

//  return the matching index of TLB entry, return -1 if it is not found
int TLB_contains(TLB* tlb, unsigned short page_no) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb->entries[i].virtual_page_no == page_no) {
            return i;
        }
    }
    return -1;
} 

//  update the order of each entry
void TLB_update(TLB* tlb, int index) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (i != index && tlb->entries[i].order < tlb->entries[index].order) {
            tlb->entries[i].order++;
        }
    }
    tlb->entries[index].order = 0;
}

//  calculate the physical address by replacing the virtural page number 
//  with the physical page number
u_int16_t TLB_paddress(TLB* tlb, u_int16_t v_address, int index) {
    return v_address << 4 >> 4 | tlb->entries[index].physical_page_no >> 12 << 12;
}

//  find the lru entry(order: 3), return its index
//  3 indicates it was used furthest in the past
int TLB_lru(TLB* tlb) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb->entries[i].order == 3) {
            return i;
        }
    }
    fprintf(stderr, "something is wrong");
    return -1;
}

//  do the update thing
u_int16_t TLB_main(TLB* tlb, PageTable* ptab, u_int16_t v_address) {
    unsigned short page_no = to_page(v_address); // getting the virtual page number
    int index = TLB_contains(tlb, page_no);
    if (index != -1) { // if there is a hit
        TLB_update(tlb, index);
        //  return address
        return TLB_paddress(tlb, v_address, index);
    } else {
        //  load the entry from the page table
        PageTableEntry* pte = &ptab->entries[page_no];
        if (pte->valid == 1) {
            int lru = TLB_lru(tlb);
            tlb->entries[lru].physical_page_no = pte->physical_page_no;
            TLB_update(tlb, lru);
        } else {
            fprintf(stderr, "segmentation fault");
        }
    }

}

//  print the cache
void TLB_print(TLB* tlb) {
    printf("index: \t\t 0\t 1\t 2\t 3\n");
    printf("TLB entries:\t[%d]\t[%d]\t[%d]\t[%d]\n", 
    tlb->entries[0].order, tlb->entries[1].order, 
    tlb->entries[2].order, tlb->entries[3].order);
}

int main() {
    PageTable pte;
    TLB tlb;
    PTE_init(&pte);
    TLB_init(&tlb);
    TLB_print(&tlb);
    return 0;
}