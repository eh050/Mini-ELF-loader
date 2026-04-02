/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: Elias Hawkins
 */

#include "p2-load.h"

#define R 4
#define W 2
#define X 1

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    // Check if the file or the pointer are NULL.
    if (file == NULL || phdr == NULL) {
        return false;
    }

    // Set the byte offset so we can read from were we're supposed to.
    if (fseek(file, offset, SEEK_SET) != 0) {
        return false;
    }

    // Read into a size struct to check.
    size_t p = fread(phdr, sizeof(elf_phdr_t), 1, file);

    // Check if the file is large enough.
    if (p != 1) {
        return false;
    }

    // Check the magic number.
    if (phdr->magic != 0xDEADBEEF) {
        return false;
    }

    return true;
}

bool load_segment (FILE *file, byte_t *memory, elf_phdr_t *phdr)
{

    // Check if the file, pointer, or memory are NULL.
    if (file == NULL || phdr == NULL || memory == NULL) {
        return false;
    }

    // Check the magic number.
    if (phdr->magic != 0xDEADBEEF) {
        return false;
    }

    // Reject unknown segment types. (Valid are: DATA, CODE, STACK, HEAP, UNKNOWN)
    switch (phdr->p_type) {
        case CODE:
        case DATA:
        case STACK:
        case HEAP:
        case UNKNOWN:
            break;
        default:
            return false;
    }

    // Reject segments that would write past the end of virtual memory
    if ((phdr->p_offset + phdr->p_vaddr + phdr->p_size) > MEMSIZE) {
        return false;
    }

    // Note that memory should be a pointer to the beginning of the address space
    if (fseek(file, phdr->p_offset, SEEK_SET) != 0) {
        return false;
    }

    // Don't need to read anything from 0-byte segments.
    if (phdr->p_size != 0) {
        // Check is we can read the phdr into the beginning of the address space + the virtual address.
        if (fread(memory + phdr->p_vaddr, 1, phdr->p_size, file) != phdr->p_size) {
            return false;
        }
    }

    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

// Print the Mini-ELF program headers passed in as an array via phdrs. There will be numphdrs of them.
void dump_phdrs (uint16_t numphdrs, elf_phdr_t *phdrs)
{
    // Print table column headers
    printf(" %-9s %-9s %-9s %-9s %-9s %-5s\n", "Segment", "Offset", "Size", "VirtAddr", "Type",
           "Flags");

    // Loop over all the program headers
    for (int i = 0; i < numphdrs; i++) {

        // Define a phdr so we can reference the fields.
        elf_phdr_t *p = &phdrs[i];

        // Loop through type and create a string with it.
        char *type_str;
        switch(p->p_type) {
            case DATA:
                type_str = "DATA";
                break;
            case CODE:
                type_str = "CODE";
                break;
            case STACK:
                type_str = "STACK";
                break;
            case HEAP:
                type_str = "HEAP";
                break;
            default:
                type_str = "UNKNOWN";
                break;
        }

        // Loop through flags using boolean algebra to create a string.
        char flags[4] = "   "; // Have to do an extra character for the null terminator string.

        if (p->p_flags & R) {
            flags[0] = 'R';
        }

        if (p->p_flags & W) {
            flags[1] = 'W';
        }

        if (p->p_flags & X) {
            flags[2] = 'X';
        }

        // Format to match the header columns.
        printf("  %02d       0x%04x    0x%04x    0x%04x    %-8s  %s\n", i, p->p_offset, p->p_size,
               p->p_vaddr, type_str, flags);

    }
}

void dump_memory (byte_t *memory, uint16_t start, uint16_t end)
{
    // Print the header row.
    printf("Contents of memory from %04x to %04x:\n", start, end);

    // Helper variable.
    int row_counter = start;

    // Print until start = end.
    while (row_counter < end) {


        // Print the row number.
        printf("  %04x  ", row_counter - (row_counter % 16));

        int blank_bits = row_counter % 16;

        // Print blank bits in case of unaligned.
        for (int i = 0; i < blank_bits; i++) {
            printf("   ");
            if (i == 7) {
                printf(" ");
            }
        }

        // Helper variable to count bits in row.
        int bit_counter = 0;

        // Print individual bits.
        for (int i = blank_bits; (i < 16) && (row_counter + bit_counter < end); i++) {

            // Print spaces.
            if (i != blank_bits) {
                printf(" ");
            }

            // Print bits.
            printf("%02x", memory[row_counter +  bit_counter]);
            bit_counter++;

            // Print double space if needed.
            if (i == 7 && (row_counter + bit_counter) < end) {
                printf(" ");
            }
        }

        // Print new row and increment the counter.
        printf("\n");
        row_counter += bit_counter;

    }

}
