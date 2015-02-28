#include <sys/idt.h>


/* defined in idt.s */
void _x86_64_asm_lidt(struct gdtr_t* gdtr, uint64_t cs_idx, uint64_t ds_idx);

void reload_idt(void) {
    // Call this function once it is implemented
    // _x86_64_asm_lidt(struct gdtr_t* gdtr, uint64_t cs_idx, uint64_t ds_idx)
}
