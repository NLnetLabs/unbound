/*
 * This is an example to show how dynamic libraries can be made to work with
 * unbound. To build a .so file simply run:
 *   gcc -I../.. -shared -Wall -Werror -fpic  -o helloworld.so helloworld.c
 * And to build for windows, first make unbound with the --with-dynlibmod
 * switch, then use this command:
 *   x86_64-w64-mingw32-gcc -m64 -I../.. -shared -Wall -Werror -fpic -o helloworld.dll helloworld.c -L../.. -l:libunbound.a
 * to cross-compile a 64-bit Windows DLL.
 */

#include "../../config.h"
#include "../../util/module.h"

#ifdef HAVE_WINDOWS_H
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT int init(int id, struct config_file* cfg) {
    log_info("Hello world from init");
    return 1;
}

EXPORT int deinit(int id) {
    log_info("Hello world from deinit");
    return 1;
}

EXPORT int operate(int id, enum module_ev event, struct module_qstate* qstate, void* data) {
    log_info("Hello world from operate");
    if (event == module_event_new || event == module_event_pass) {
        qstate->ext_state[id] = module_wait_module;
    } else if (event == module_event_moddone) {
        qstate->ext_state[id] = module_finished;
    } else {
        qstate->ext_state[id] = module_error;
    }
    return 1;
}

EXPORT int inform_super(int id, struct module_qstate* qstate, struct module_qstate* super, void* data) {
    log_info("Hello world from inform_super");
    return 1;
}
