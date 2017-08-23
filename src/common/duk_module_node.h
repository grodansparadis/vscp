#if !defined(DUK_MODULE_NODE_H_INCLUDED)
#define DUK_MODULE_NODE_H_INCLUDED

#include "duktape.h"

#ifdef __cplusplus
extern "C" {
#endif
    
duk_ret_t duk_module_node_peval_main(duk_context *ctx, const char *path);
void duk_module_node_init(duk_context *ctx);

#ifdef __cplusplus
}
#endif
    
#endif  /* DUK_MODULE_NODE_H_INCLUDED */
