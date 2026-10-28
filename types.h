#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint;

typedef enum
{
    e_encode,
    e_decode,
    e_unsupported
} OperationType;

typedef enum
{
    e_success,
    e_failure
} Status;

#endif