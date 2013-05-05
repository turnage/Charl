#ifndef OPERATOR_H
#define OPERATOR_H

#include <charl.h>

/* Change data in the operator. */
void operator_set_name (const char *name);

/* Exchange keys with the server. */
const char *operator_authenticate ();

/* Identify with the server. */
const char *operator_identify (const char *alias, const char *word);

/* Connect to the server. */
const char *operator_connect (ENetAddress add);

#endif /* OPERATOR_H */