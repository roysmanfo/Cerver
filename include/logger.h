#pragma once

#include <stdio.h>

#define debug(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#define log(fmt, ...) printf("[LOG] " fmt, ##__VA_ARGS__)
#define pget(fmt, ...) printf("[GET] " fmt, ##__VA_ARGS__)
#define ppost(fmt, ...) printf("[POST] " fmt, ##__VA_ARGS__)
