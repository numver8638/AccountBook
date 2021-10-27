#pragma once

#include <stdbool.h>
#include <stddef.h>

int GetInt(const char* msg, bool allowZero);
int GetString(const char* msg, char* buffer, size_t length);