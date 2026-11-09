/* Native RomFS verification, usable before starting a game. No runtime state is changed. */
#pragma once
#include <stddef.h>
#include <stdint.h>
enum { RUNTIME_ROMFS_VERIFY_STRICT=1, RUNTIME_ROMFS_VERIFY_FULL=2 };
typedef struct {
 const int *fds; unsigned parts;
 uint64_t size; const char *sha256;
 const void *index; size_t index_size; const char *index_sha256;
 const char *proof_path, *stage;
 unsigned flags;
 uint64_t *hashed_bytes;
} RuntimeRomfsVerification;
/* Zero on success; failure returns -1 and a message instead of terminating the process. */
int runtime_romfs_verify(const RuntimeRomfsVerification *input, char *error, size_t error_size);
