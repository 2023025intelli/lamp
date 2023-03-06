//
//         |    _)  |
//   |   |  __|  |  |   __|      __|
//   |   |  |    |  | \__ \     (
//  \__,_| \__| _| _| ____/ _| \___|
//
//

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

char *secondsToStr(int seconds, int negative) {
    char *str = malloc(sizeof(char) * 6);
    sprintf(str, "%s%d:%02d", (negative ? "-" : ""), seconds / 60, seconds % 60);
    return str;
}

int isBetween(double value, double lower, double upper) {
    return value > lower ? (value < upper ? 1 : 0) : 0;
}