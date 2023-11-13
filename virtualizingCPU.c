#include <stdio.h>
#include <stdlib.h>
#include "common.h"  // common.h 헤더 파일을 포함

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage : cpu <string>\n");
        exit(1);
    }
    char* str = argv[1];

    while (1) {
        Spin(1);  // Spin 함수를 사용
        printf("%s\n", str);
    }

    return 0;
}