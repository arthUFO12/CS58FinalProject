
#define PIPE_FLAG 0x0
#define LOCK_FLAG 0x1
#define CVAR_FLAG 0x2

#define FLAG_MASK 0xc0000000
#define FLAG_SHIFT 30
#define ID_MASK (~FLAG_MASK)

#define IS_PIPE_ID(n) ((((unsigned int)(n)) >> FLAG_SHIFT) == PIPE_FLAG)
#define IS_LOCK_ID(n) ((((unsigned int)(n)) >> FLAG_SHIFT) == LOCK_FLAG)
#define IS_CVAR_ID(n) ((((unsigned int)(n)) >> FLAG_SHIFT) == CVAR_FLAG)

#define GET_ID(n) (((unsigned int)(n)) & ID_MASK)
#define CREATE_ID(flag, id) ((((unsigned int)(flag)) << FLAG_SHIFT) & (((unsigned int)(id)) & ID_MASK))
