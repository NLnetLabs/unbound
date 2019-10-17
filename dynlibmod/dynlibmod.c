#include "config.h"
#include "util/module.h"
#include "util/config_file.h"
#include "dynlibmod/dynlibmod.h"

#if HAVE_WINDOWS_H
#include <windows.h>
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

        //
        // Output message string on stderr.
        //
        log_info("dynlibmod: %s (%ld)", MessageBuffer, dwLastError);
        //WriteFile(
        //    GetStdHandle(STD_ERROR_HANDLE),
        //    MessageBuffer,
        //    dwBufferLength,
        //    &dwBytesWritten,
        //    NULL
        //    );

        //
        // Free the buffer allocated by the system.
        //
        LocalFree(MessageBuffer);
    }

}
#else
#include <dlfcn.h>
#define __DYNSYM void*
#define __LOADSYM dlsym
void log_dlerror() {
    log_err("dynlibmod: %s", dlerror());
}
#endif

/**
 * Global state for the module.
 */

typedef int (*func_init_t)(int, struct config_file*);
typedef int (*func_deinit_t)(int);
typedef int (*func_operate_t)(int, enum module_ev event, struct module_qstate* qstate, void*);
typedef int (*func_inform_t)(int, struct module_qstate* qstate, struct module_qstate* super, void*);
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

	/** Module qstate. */
	struct module_qstate* qstate;
};

struct dynlibmod_qstate {

	/** Module per query data. */
	void* data;
};

/** dynlib module init */
int dynlibmod_init(struct module_env* env, int id) {
    struct dynlibmod_env* de = (struct dynlibmod_env*)calloc(1, sizeof(struct dynlibmod_env));
    if (!de)
    {
        log_err("dynlibmod: malloc failure");
        return 0;
    }

    env->modinfo[id] = (void*) de;

    de->fname = env->cfg->dynlib_file;
    if (de->fname == NULL || de->fname[0] == 0) {
        log_err("dynlibmod: no dynamic library given.");
        return 0;
    }
    log_info("Trying to load library %s", de->fname);
#ifndef HAVE_WINDOWS_H
    void* dynamic_library = dlopen(de->fname, RTLD_LAZY | RTLD_GLOBAL);
#else
    HMODULE dynamic_library = LoadLibrary(de->fname);
#endif
    if (dynamic_library == NULL) {
        log_dlerror();
        log_err("dynlibmod: unable to load dynamic library.");
        return 0;
    } else {
        __DYNSYM initializer = __LOADSYM(dynamic_library,"init");
        if (initializer == NULL) {
            log_err("dynlibmod: unable to load init procedure from dynamic library.");
#ifndef HAVE_WINDOWS_H
            log_err("dynlibmod: %s", dlerror());
#endif
            return 0;
        } else {
            de->func_init = (func_init_t) initializer;
        }
        __DYNSYM deinitializer = __LOADSYM(dynamic_library,"deinit");
        if (deinitializer == NULL) {
            log_err("dynlibmod: unable to load deinit procedure from dynamic library.");
#ifndef HAVE_WINDOWS_H
            log_err("dynlibmod: %s", dlerror());
#endif
            return 0;
        } else {
            de->func_deinit = (func_deinit_t) deinitializer;
        }
        __DYNSYM operate = __LOADSYM(dynamic_library,"operate");
        if (operate == NULL) {
            log_err("dynlibmod: unable to load operate procedure from dynamic library.");
#ifndef HAVE_WINDOWS_H
            log_err("dynlibmod: %s", dlerror());
#endif
            return 0;
        } else {
            de->func_operate = (func_operate_t) operate;
        }
        __DYNSYM inform = __LOADSYM(dynamic_library,"inform_super");
        if (inform == NULL) {
            log_err("dynlibmod: unable to load inform_super procedure from dynamic library.");
#ifndef HAVE_WINDOWS_H
            log_err("dynlibmod: %s", dlerror());
#endif
            return 0;
        } else {
            de->func_inform = (func_inform_t) inform;
        }
    }
    de->func_init(id, env->cfg);
    return 1;
}

/** dynlib module deinit */
void dynlibmod_deinit(struct module_env* env, int id) {
    struct dynlibmod_env* de = env->modinfo[id];
    if(de == NULL)
        return;
    de->func_deinit(id);
    de->fname = NULL;
    free(de);
}

/** dynlib module operate on a query */
void dynlibmod_operate(struct module_qstate* qstate, enum module_ev event,
    int id, struct outbound_entry* outbound) {
    struct dynlibmod_env* de = qstate->env->modinfo[id];
    struct dynlibmod_qstate* dq = (struct dynlibmod_qstate*)qstate->minfo[id];

    void * data = dq == NULL ? NULL : dq->data;
    int ret = de->func_operate(id, event, qstate, data);
    if (ret != 1) {
        log_err("dynlibmod: dynamic library returned bad code from operate %d.", ret);
        qstate->ext_state[id] = module_error;
    }
}

/** dynlib module  */
void dynlibmod_inform_super(struct module_qstate* qstate, int id,
    struct module_qstate* super) {
    struct dynlibmod_env* de = qstate->env->modinfo[id];
    struct dynlibmod_qstate* dq = (struct dynlibmod_qstate*)qstate->minfo[id];

    void * data = dq == NULL ? NULL : dq->data;
    int ret = de->func_inform(id, qstate, super, data);
    if (ret != 1) {
        log_err("dynlibmod: dynamic library returned bad code from inform_super %d.", ret);
        qstate->ext_state[id] = module_error;
    }
}

/** dynlib module cleanup query state */
void dynlibmod_clear(struct module_qstate* qstate, int id) {
    struct dynlibmod_qstate* dq;
    if (qstate == NULL)
        return;

    dq = (struct dynlibmod_qstate*)qstate->minfo[id];
    verbose(VERB_ALGO, "dynlibmod: clear, id: %d, dq:%p", id, dq);
    if(dq != NULL) {
        /* Free qstate */
        free(dq);
    }

    qstate->minfo[id] = NULL;
}

/** dynlib module alloc size routine */
size_t dynlibmod_get_mem(struct module_env* env, int id) {
    struct dynlibmod_env* de = (struct dynlibmod_env*)env->modinfo[id];
    verbose(VERB_ALGO, "dynlibmod: get_mem, id: %d, de:%p", id, de);
    if(!de)
        return 0;
    return sizeof(*de);
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
