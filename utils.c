#include <malloc.h>
#include "utils.h"

int strInArray(const char *str, char *arr[], int lenght) {
    for (int i = 0; i < lenght; i++) {
        if (strCompare(str, arr[i]) == 0) return 1;
    }
    return 0;
}

int strCompare(const char *str1, const char *str2) {
    while (*str1 && *str1 == *str2) str1++, str2++;
    return *str1 - *str2;
}

char *seconds_to_str(int seconds) {
    char *str = malloc(sizeof(char) * 24);
    sprintf(str, "%d:%02d", seconds / 60, seconds % 60);
    return str;
}