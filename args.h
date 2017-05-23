#if !defined ARGS_H
#define ARGS_H

#include "endpoint.h"

extern void dispay_usage();
extern int check_args(int argc, char **argv);
extern int buildep(nsp::tcpip::endpoint &ep);
extern int gettype();
extern int getpkgsize();
extern int getinterval();

#endif