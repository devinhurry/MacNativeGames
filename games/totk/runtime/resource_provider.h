/* Compile-time read-only resource provider. Native builds keep their existing
   files; an application may supply inline operations without per-read dispatch. */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
typedef struct RuntimeResourceFile RuntimeResourceFile;
static inline int runtime_resources_init(void);
static inline int runtime_resources_active(void);
static inline const char *runtime_resources_error(void);
static inline int runtime_resource_volume(const char *volume);
/* A complete native volume may bypass the external file provider. */
static inline int runtime_resource_native(const char *volume,const char **index_dir,const char **data_dir,const char **proof_path);
static inline RuntimeResourceFile *runtime_resource_open(const char *path);
static inline ssize_t runtime_resource_read(RuntimeResourceFile *file,void *buffer,size_t size,uint64_t offset);
static inline uint64_t runtime_resource_size(const RuntimeResourceFile *file);
static inline void runtime_resource_close(RuntimeResourceFile *file);
#define MNG_RESOURCE_PATH_(path) #path
#define MNG_RESOURCE_PATH(path) MNG_RESOURCE_PATH_(path)
#include MNG_RESOURCE_PATH(MNG_RESOURCE_PROVIDER)
#undef MNG_RESOURCE_PATH
#undef MNG_RESOURCE_PATH_
