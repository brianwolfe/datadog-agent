#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "../../ebpf/c/bpf_helpers.h"

struct syscall_cache_t {
    union {
        struct {
            int flags;
            umode_t mode;
            struct dentry *dentry;
        } open;

        struct {
            umode_t mode;
            struct inode *dir;
            struct dentry *dentry;
        } mkdir;

        struct {
            struct path_key_t path_key;
        } unlink;

        struct {
            struct path_key_t path_key;
        } rmdir;

        struct {
            struct inode *src_dir;
            struct dentry *src_dentry;
            struct path_key_t random_key;
        } rename;

        struct {
            struct dentry *dentry;
            umode_t mode;
            uid_t user;
            gid_t group;
        } setattr;
    };
};

struct bpf_map_def SEC("maps/syscalls") syscalls = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(u64),
    .value_size = sizeof(struct syscall_cache_t),
    .max_entries = 256,
    .pinning = 0,
    .namespace = "",
};

void __attribute__((always_inline)) cache_syscall(struct syscall_cache_t *syscall) {
    u64 key = bpf_get_current_pid_tgid(); \
    bpf_map_update_elem(&syscalls, &key, syscall, BPF_ANY);
}

struct syscall_cache_t * __attribute__((always_inline)) peek_syscall() {
    u64 key = bpf_get_current_pid_tgid();
    return (struct syscall_cache_t *) bpf_map_lookup_elem(&syscalls, &key);
}

struct syscall_cache_t * __attribute__((always_inline)) pop_syscall() {
    u64 key = bpf_get_current_pid_tgid();
    struct syscall_cache_t *syscall = (struct syscall_cache_t*) bpf_map_lookup_elem(&syscalls, &key);
    if (syscall)
        bpf_map_delete_elem(&syscalls, &key);
    return syscall;
}

#endif