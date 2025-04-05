#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "elf.h"
#include "vmm.h"
#include "pmm.h"
#include "process_image.h"

extern process_t* current_proc;

// VMM-säker kopiering till userland
static int vmm_copy_to_user(pagetable_t pt, uintptr_t dst, const void* src, size_t len) {
    if (!vmm_is_user_mapped(pt, dst, len)) return -EFAULT;
    if (dst < 0x1000) return -EFAULT;
    memcpy((void*)dst, src, len);
    return 0;
}

// Mappa ett PT_LOAD-segment
static int map_elf_segment(const Elf64_Phdr* phdr, const uint8_t* file, size_t filesize) {
    uintptr_t vaddr = phdr->p_vaddr;
    size_t memsz = phdr->p_memsz;
    size_t filesz = phdr->p_filesz;
    uintptr_t offset = phdr->p_offset;

    if (vaddr + memsz < vaddr || (vaddr + memsz) > 0x00007FFFFFFFFFFF)
        return -EINVAL;
    if (offset + filesz > filesize)
        return -EIO;

    uintptr_t start = vaddr & ~0xFFF;
    uintptr_t end = (vaddr + memsz + 0xFFF) & ~0xFFF;

    for (uintptr_t addr = start; addr < end; addr += PAGE_SIZE) {
        uintptr_t phys = pmm_alloc_page();
        if (!phys) return -ENOMEM;
        vmm_map_user(current_proc->page_table, addr, phys, PAGE_PRESENT | PAGE_USER | PAGE_RW);
    }

    if (filesz > 0) {
        int r = vmm_copy_to_user(current_proc->page_table, vaddr, file + offset, filesz);
        if (r < 0) return r;
    }

    if (memsz > filesz) {
        static const char zeros[64] = { 0 };
        uintptr_t ptr = vaddr + filesz;
        size_t zero_len = memsz - filesz;

        while (zero_len > 0) {
            size_t chunk = zero_len > sizeof(zeros) ? sizeof(zeros) : zero_len;
            int r = vmm_copy_to_user(current_proc->page_table, ptr, zeros, chunk);
            if (r < 0) return r;
            ptr += chunk;
            zero_len -= chunk;
        }
    }

    return 0;
}

// Ladda ELF-segment
static int load_elf_segments(const uint8_t* file, const Elf64_Ehdr* eh, size_t filesize,
    uintptr_t* base, uintptr_t* top) {
    Elf64_Phdr* ph = (Elf64_Phdr*)(file + eh->e_phoff);
    uintptr_t user_base = UINTPTR_MAX, user_top = 0;

    for (int i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD) continue;
        int r = map_elf_segment(&ph[i], file, filesize);
        if (r < 0) return r;

        uintptr_t seg_base = ph[i].p_vaddr;
        uintptr_t seg_top = ph[i].p_vaddr + ph[i].p_memsz;

        if (seg_base < user_base) user_base = seg_base;
        if (seg_top > user_top) user_top = seg_top;
    }

    *base = user_base;
    *top = user_top;
    return 0;
}

// Bygg användarstack med argv/envp
static int build_user_stack(char* const argv[], char* const envp[], void** stack_top,
    int* argc_out, char*** argv_out, char*** envp_out) {
    const uintptr_t stack_base = 0x7FFFFFF000;
    const size_t stack_size = 0x10000;

    for (uintptr_t addr = stack_base - stack_size; addr < stack_base; addr += PAGE_SIZE) {
        uintptr_t phys = pmm_alloc_page();
        if (!phys) return -ENOMEM;
        vmm_map_user(current_proc->page_table, addr, phys, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    }

    int argc = 0, envc = 0;
    while (argv && argv[argc]) argc++;
    while (envp && envp[envc]) envc++;

    size_t total_str_len = 0;
    for (int i = 0; i < argc; i++) total_str_len += strlen(argv[i]) + 1;
    for (int i = 0; i < envc; i++) total_str_len += strlen(envp[i]) + 1;

    size_t total_size =
        sizeof(int) +
        (argc + 1) * sizeof(char*) +
        (envc + 1) * sizeof(char*) +
        total_str_len + 32;

    uint8_t* temp = malloc(total_size);
    if (!temp) return -ENOMEM;
    memset(temp, 0, total_size);

    uint8_t* p = temp;
    int* argc_ptr = (int*)p;
    *argc_ptr = argc;
    p += sizeof(int);

    char** argv_ptrs = (char**)p;
    p += (argc + 1) * sizeof(char*);
    char** envp_ptrs = (char**)p;
    p += (envc + 1) * sizeof(char*);

    char* str_cursor = (char*)p;
    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]) + 1;
        strcpy(str_cursor, argv[i]);
        argv_ptrs[i] = (char*)(stack_base - stack_size + (str_cursor - (char*)temp));
        str_cursor += len;
    }

    for (int i = 0; i < envc; i++) {
        size_t len = strlen(envp[i]) + 1;
        strcpy(str_cursor, envp[i]);
        envp_ptrs[i] = (char*)(stack_base - stack_size + (str_cursor - (char*)temp));
        str_cursor += len;
    }

    uintptr_t user_stack_ptr = stack_base - total_size;
    user_stack_ptr &= ~0xF;

    int r = vmm_copy_to_user(current_proc->page_table, user_stack_ptr, temp, total_size);
    free(temp);
    if (r < 0) return r;

    if (stack_top) *stack_top = (void*)user_stack_ptr;
    if (argc_out) *argc_out = argc;
    if (argv_out) *argv_out = (char**)(user_stack_ptr + sizeof(int));
    if (envp_out) *envp_out = (char**)(user_stack_ptr + sizeof(int) + (argc + 1) * sizeof(char*));

    return 0;
}

// ELF-loader (huvudfunktion)
int exec_load_elf(const char* path, char* const argv[], char* const envp[], process_image_t* out_img) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -errno;

    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return -errno; }

    if ((size_t)st.st_size < sizeof(Elf64_Ehdr)) {
        close(fd); return -ENOEXEC;
    }

    uint8_t* file = malloc(st.st_size);
    if (!file) { close(fd); return -ENOMEM; }

    ssize_t r = read(fd, file, st.st_size);
    close(fd);
    if (r != st.st_size) { free(file); return -EIO; }

    Elf64_Ehdr* eh = (Elf64_Ehdr*)file;

    if (memcmp(eh->e_ident, ELFMAG, SELFMAG) != 0 ||
        eh->e_ident[EI_CLASS] != ELFCLASS64 ||
        (eh->e_type != ET_EXEC && eh->e_type != ET_DYN)) {
        free(file); return -ENOEXEC;
    }

    uintptr_t user_base, user_top;
    r = load_elf_segments(file, eh, st.st_size, &user_base, &user_top);
    if (r < 0) { free(file); return r; }

    uintptr_t heap_base = (user_top + 0xFFF) & ~0xFFF;
    uintptr_t heap_end = heap_base;

    void* stack_ptr;
    int argc;
    char** argv_ptrs, ** envp_ptrs;
    r = build_user_stack(argv, envp, &stack_ptr, &argc, &argv_ptrs, &envp_ptrs);
    if (r < 0) { free(file); return r; }

    memset(out_img, 0, sizeof(process_image_t));
    out_img->entry_point = (void*)(uintptr_t)eh->e_entry;
    out_img->stack_top = stack_ptr;
    out_img->heap_base = heap_base;
    out_img->heap_end = heap_end;
    out_img->user_base = user_base;
    out_img->user_top = user_top;
    out_img->argc = argc;
    out_img->argv = argv_ptrs;
    out_img->envp = envp_ptrs;

    free(file);
    return 0;
}
