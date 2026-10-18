#include "headers.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#define VFS_MAX_FILES 32
#define VFS_NAME_LEN 32
#define VFS_MAGIC 0x4E4F5346

static int vfs_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

struct vfs_file {
    char name[VFS_NAME_LEN];
    uint32_t offset;
    uint32_t size;
};

struct vfs_header {
    uint32_t magic;
    uint32_t file_count;
};

static struct vfs_file vfs_files[VFS_MAX_FILES];
static int vfs_file_count = 0;
static uint8_t *vfs_image = NULL;
static uint32_t vfs_image_size = 0;

void vfs_init(uint8_t *image, uint32_t size) {
    vfs_image = image;
    vfs_image_size = size;

    struct vfs_header *hdr = (struct vfs_header *)image;
    if (hdr->magic != VFS_MAGIC) {
        vfs_file_count = 0;
        return;
    }

    vfs_file_count = hdr->file_count;
    if (vfs_file_count > VFS_MAX_FILES) {
        vfs_file_count = VFS_MAX_FILES;
    }

    struct vfs_file *entries = (struct vfs_file *)(image + sizeof(struct vfs_header));
    for (int i = 0; i < vfs_file_count; i++) {
        vfs_files[i] = entries[i];
    }
}

struct vfs_file *vfs_find(const char *name) {
    for (int i = 0; i < vfs_file_count; i++) {
        if (vfs_files[i].name[0] && !vfs_strcmp(vfs_files[i].name, name)) {
            return &vfs_files[i];
        }
    }
    return NULL;
}

int vfs_read(const char *name, uint8_t *buf, uint32_t buf_size) {
    struct vfs_file *f = vfs_find(name);
    if (!f) return -1;
    if (f->offset + f->size > vfs_image_size) return -1;
    if (f->size > buf_size) return -1;
    for (uint32_t i = 0; i < f->size; i++) {
        buf[i] = vfs_image[f->offset + i];
    }
    return f->size;
}

int vfs_exists(const char *name) {
    return vfs_find(name) != NULL;
}
