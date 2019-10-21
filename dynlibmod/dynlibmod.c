#include "config.h"
#include "util/module.h"
#include "util/config_file.h"
#include "dynlibmod/dynlibmod.h"

#if HAVE_WINDOWS_H
#include <windows.h>
#define __DYNMOD HMODULE
#define __DYNSYM FARPROC
#define __LOADSYM GetProcAddress
void log_dlerror() {
    DWORD dwLastError = GetLastError();
    LPSTR MessageBuffer;
    DWORD dwBufferLength;
    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM ;
    if(dwBufferLength = FormatMessageA(
        dwFormatFlags,
        NULL, // module to get message from (NULL == system)
        dwLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
        (LPSTR) &MessageBuffer,
        0,
        NULL
        ))
    {
        DWORD dwBytesWritten;
        log_err("dynlibmod: %s (%ld)", MessageBuffer, dwLastError);
        LocalFree(MessageBuffer);
    }

}
HMODULE open_library(const char* fname) {
    return LoadLibrary(fname);
}
#else
#include <dlfcn.h>
#define __DYNMOD void*
#define __DYNSYM void*
#define __LOADSYM dlsym
void log_dlerror() {
    log_err("dynlibmod: %s", dlerror());
}
void* open_library(const char* fname) {
    return dlopen(fname, RTLD_LAZY | RTLD_GLOBAL);
}
#endif


/**
 * Global state for the module.
 */

typedef void (*func_init_t)(struct module_env*, int);
typedef void (*func_deinit_t)(struct module_env*, int);
typedef void (*func_operate_t)(struct module_qstate*, enum module_ev, int, struct outbound_entry*);
typedef void (*func_inform_t)(struct module_qstate*, int, struct module_qstate*);
typedef void (*func_clear_t)(struct module_qstate*, int);
typedef size_t (*func_get_mem_t)(struct module_env*, int);

struct dynlibmod_env {

	/** Dynamic library filename. */
	const char* fname;

	/** Module init function */
	func_init_t func_init;
	/** Module deinit function */
	func_deinit_t func_deinit;
	/** Module operate function */
	func_operate_t func_operate;
	/** Module super_inform function */
	func_inform_t func_inform;
	/** Module clear function */
	func_clear_t func_clear;
	/** Module get_mem function */
	func_get_mem_t func_get_mem;

	/** Module qstate. */
	struct module_qstate* qstate;
};

/**
 * counter for dynamic library module instances
 * incremeted by dynlibmod_init
 */
int dynlib_mod_count = 0;

/** dynlib module init */
int dynlibmod_init(struct module_env* env, int id) {
    int dynlib_mod_idx = dynlib_mod_count++;
    struct config_strlist* cfg_item = env->cfg->dynlib_file;
    struct dynlibmod_env* de = (struct dynlibmod_env*)calloc(1, sizeof(struct dynlibmod_env));
    __DYNMOD dynamic_library;
    if (!de)
    {
        log_err("dynlibmod[%d]: malloc failure", dynlib_mod_idx);
        return 0;
    }

    env->modinfo[id] = (void*) de;

    de->fname = NULL;
    for(int i = dynlib_mod_idx;
        i != 0 && cfg_item != NULL;
        i--, cfg_item = cfg_item->next) {}

    if (cfg_item == NULL || cfg_item->str == NULL || cfg_item->str[0] == 0) {
        log_err("dynlibmod[%d]: no dynamic library given.", dynlib_mod_idx);
        return 0;
    } else {
        de->fname = cfg_item->str;
    }
    verbose(VERB_ALGO, "dynlibmod[%d]: Trying to load library %s", dynlib_mod_idx, de->fname);
    dynamic_library = open_library(de->fname);
    if (dynamic_library == NULL) {
        log_dlerror();
        log_err("dynlibmod[%d]: unable to load dynamic library \"%s\".", dynlib_mod_idx, de->fname);
        return 0;
    } else {
        __DYNSYM initializer = __LOADSYM(dynamic_library,"init");
        if (initializer == NULL) {
            log_dlerror();
            log_err("dynlibmod[%d]: unable to load init procedure from dynamic library \"%s\".", dynlib_mod_idx, de->fname);
            return 0;
        } else {
            de->func_init = (func_init_t) initializer;
        }
        __DYNSYM deinitializer = __LOADSYM(dynamic_library,"deinit");
        if (deinitializer == NULL) {
            log_dlerror();
            log_err("dynlibmod[%d]: unable to load deinit procedure from dynamic library \"%s\".", dynlib_mod_idx, de->fname);
            return 0;
        } else {
            de->func_deinit = (func_deinit_t) deinitializer;
        }
        __DYNSYM operate = __LOADSYM(dynamic_library,"operate");
        if (operate == NULL) {
            log_dlerror();
            log_err("dynlibmod[%d]: unable to load operate procedure from dynamic library \"%s\".", dynlib_mod_idx, de->fname);
            return 0;
        } else {
            de->func_operate = (func_operate_t) operate;
        }
        __DYNSYM inform = __LOADSYM(dynamic_library,"inform_super");
        if (inform == NULL) {
            log_dlerror();
            log_err("dynlibmod[%d]: unable to load inform_super procedure from dynamic library \"%s\".", dynlib_mod_idx, de->fname);
            return 0;
        } else {
            de->func_inform = (func_inform_t) inform;
        }
        __DYNSYM clear = __LOADSYM(dynamic_library,"clear");
        if (clear == NULL) {
            log_dlerror();
            log_err("dynlibmod[%d]: unable to load clear procedure from dynamic library \"%s\".", dynlib_mod_idx, de->fname);
            return 0;
        } else {
            de->func_clear = (func_clear_t) clear;
        }
        __DYNSYM get_mem = __LOADSYM(dynamic_library,"get_mem");
        if (get_mem == NULL) {
            log_dlerror();
            log_err("dynlibmod[%d]: unable to load get_mem procedure from dynamic library \"%s\".", dynlib_mod_idx, de->fname);
            return 0;
        } else {
            de->func_get_mem = (func_get_mem_t) get_mem;
        }
    }
    de->func_init(env, id);
    return 1;
}

/** dynlib module deinit */
void dynlibmod_deinit(struct module_env* env, int id) {
    struct dynlibmod_env* de = env->modinfo[id];
    if(de == NULL)
        return;
    de->func_deinit(env, id);
    de->fname = NULL;
    free(de);
}

/** dynlib module operate on a query */
void dynlibmod_operate(struct module_qstate* qstate, enum module_ev event,
    int id, struct outbound_entry* outbound) {
    struct dynlibmod_env* de = qstate->env->modinfo[id];

    de->func_operate(qstate, event, id, outbound);
}

/** dynlib module  */
void dynlibmod_inform_super(struct module_qstate* qstate, int id,
    struct module_qstate* super) {
    struct dynlibmod_env* de = qstate->env->modinfo[id];

    de->func_inform(qstate, id, super);
}

/** dynlib module cleanup query state */
void dynlibmod_clear(struct module_qstate* qstate, int id) {
    struct dynlibmod_env* de = qstate->env->modinfo[id];

    de->func_clear(qstate, id);
}

/** dynlib module alloc size routine */
size_t dynlibmod_get_mem(struct module_env* env, int id) {
    struct dynlibmod_env* de = (struct dynlibmod_env*)env->modinfo[id];
    verbose(VERB_ALGO, "dynlibmod: get_mem, id: %d, de:%p", id, de);
    if(!de)
        return 0;

    size_t size = de->func_get_mem(env, id);
    return size + sizeof(*de);
}

/**
 * The module function block
 */
static struct module_func_block dynlibmod_block = {
   "dynlib",
   &dynlibmod_init, &dynlibmod_deinit, &dynlibmod_operate, &dynlibmod_inform_super,
   &dynlibmod_clear, &dynlibmod_get_mem
};

struct module_func_block* dynlibmod_get_funcblock(void)
{
   return &dynlibmod_block;
}
