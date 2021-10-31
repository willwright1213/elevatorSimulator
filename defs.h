#ifndef DEFS_H
#define DEFS_H

#define RESET_SIGNAL -1
#define FIRE_SIGNAL 0
#define OVERLOAD_SIGNAL 1
#define HELP_SIGNAL 2
#define POWER_SIGNAL 3


#define OPEN_DOOR 0
#define CLOSE_DOOR 1
#define CAPACITY_LIMIT 1000

typedef enum {UP, DOWN} Direction;
typedef enum {STOP, IDLE, MOVING} Status;

#endif // DEFS_H
