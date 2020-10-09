#if !defined ARGS_H
#define ARGS_H

#include "compiler.h"

#include "misc.h"

#define SESS_TYPE_SERVER         (0)
#define SESS_TYPE_CLIENT         (1)

extern int arg_check_input(int argc, char **argv);

extern int arg_gettype();
extern void arg_duplicate_client_argument(struct misc_endpoint *server, int *txn, int *interval, int *window);
extern void arg_duplicate_server_argument(struct misc_endpoint *server, int *rxn);
#endif
