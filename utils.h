//
//         |    _)  |           |
//   |   |  __|  |  |   __|     __ \
//   |   |  |    |  | \__ \     | | |
//  \__,_| \__| _| _| ____/ _| _| |_|
//
//

#ifndef LAMP4_UTILS_H
#define LAMP4_UTILS_H

#include <malloc.h>

char *secondsToStr(int seconds, int negative);

int strCompare(const char *str1, const char *str2);

int isBetween(double value, double lower, double upper);

int strInArray(const char *str, char *arr[], int lenght);

#endif //LAMP4_UTILS_H
