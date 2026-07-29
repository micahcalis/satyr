#pragma once

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SYR_LOG(format, ...) printf("[Satyr] " format "\n", ##__VA_ARGS__)
#define SYR_ERROR(format, ...) fprintf(stderr, "[Satyr Error] " format "\n", ##__VA_ARGS__)
