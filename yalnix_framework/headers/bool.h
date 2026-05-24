

#ifndef BOOL_DEFINED
#define BOOL_DEFINED

/*
 * Basic boolean type definition for the Yalnix kernel.
 * Uses the C99 built-in _Bool type and defines portable
 * true/false constants for use throughout the system.
 */
typedef _Bool bool;
#define true ((bool)1)
#define false ((bool)0)

#endif
