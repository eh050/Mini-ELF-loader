/*
 * CS 261: Main driver
 *
 * Name: Elias Hawkins
 */

#include "p1-check.h"
#include "p2-load.h"
#include <stdlib.h>

/*
 * helper function for printing help text
 */
void usage (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
}

int main (int argc, char **argv)
{
    // Helper variables.
    bool print_hdr = false;
    bool print_phdrs = false;
    bool print_brief_mem = false;
    bool print_full_mem = false;

    // getopt to handle CLI arguments.
    int opt;
    while ((opt = getopt(argc, argv, "hHafsmM")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv);
                return EXIT_SUCCESS;
            case 'H':
                print_hdr = true;
                break;
            // -a is equivalent to -H -s -m.
            case 'a':
                print_hdr = true;
                print_phdrs = true;
                print_brief_mem = true;
                break;
            // -f is equivalent to -H -s -M.
            case 'f':
                print_hdr = true;
                print_phdrs = true;
                print_full_mem = true;
                break;
            case 's':
                print_phdrs = true;
                break;
            case 'm':
                print_brief_mem = true;
                break;
            case 'M':
                print_full_mem = true;
                break;
            default:
                usage(argv);
                return EXIT_FAILURE;
        }
    }

    // Check for invalid parameters.
    if (argc - optind != 1) {
        usage(argv);
        exit(EXIT_FAILURE);
    }

    // P1: Check validity regardless of whether the "-H" flag is specified!
    char *fname = argv[optind];
    FILE *file = fopen(fname, "r");

    // P1: Catch the error when the file does not exist.
    if (file == NULL) {
        printf("%s\n", "Failed to read file");
        return EXIT_FAILURE;
    }

    // P1: Catch the error when read_header is false.
    elf_hdr_t hdr;
    if (!(read_header(file, &hdr))) {
        printf("%s\n", "Failed to read file");
        return EXIT_FAILURE;
    }

    // Check if both memory options were passed
    if (print_brief_mem && print_full_mem) {
        usage(argv);
        return EXIT_SUCCESS;
    }

    // P1: Print the header.
    if (print_hdr) {
        dump_header(&hdr);
    }

    // Following along with the P2 video here.
    elf_phdr_t phdr[hdr.e_num_phdr];
    fseek(file, hdr.e_phdr_start, SEEK_SET);

    // Allocate memory on the heap for array.
    byte_t* mem = (byte_t*)calloc(MEMSIZE, 1);

    for (int i = 0; i < hdr.e_num_phdr; i++) {

        // Read the phdr.
        if (!(read_phdr(file, hdr.e_phdr_start + (i * sizeof(elf_phdr_t)),
                        &phdr[i]))) { // I dont think this offset is correct.
            printf("Failed to read file\n");
            free(mem);
            fclose(file);
            return EXIT_FAILURE;
        }

        // Load the segment.
        if (!(load_segment(file, mem, &phdr[i]))) {
            printf("Failed to read file\n");
            free(mem);
            fclose(file);
            return EXIT_FAILURE;
        }

    }

    // Print the phrds.
    if (print_phdrs) {
        dump_phdrs(hdr.e_num_phdr, phdr);
    }

    // Loop through segments for brief memory printing
    for (int i = 0; i < hdr.e_num_phdr; i++) {

        // Define a phdr so we can reference the fields.
        elf_phdr_t *p = &phdr[i];

        // Print brief memory
        if (print_brief_mem) {

            // Start should be VirtAddr and end should be it + size.
            dump_memory(mem, p->p_vaddr, p->p_vaddr + p->p_size);
        }

    }

    // Print full memory using first and last phdr's if brief memory not printed.
    if (print_full_mem) {

        dump_memory(mem, 0, MEMSIZE);

    }

    // Cleanup
    free(mem);
    fclose(file);

    return EXIT_SUCCESS;
}
