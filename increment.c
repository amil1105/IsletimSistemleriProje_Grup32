//AMIL SHIKHIYEV G221210561
//Erkin Erdoğan B241210385
//Kianoush Seddighpour G221210571
//Manar AL SAYED ALI G221210558

// increment.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("increment running in %s\n", cwd);
    }
    else {
        perror("getcwd() error");
    }

    int number;

    // Standart girdiden bir tam sayı okunur
    if (scanf("%d", &number) != 1) {
        fprintf(stderr, "Hata: Bir tam sayı girmelisiniz.\n");
        return 1;
    }

    // Tam sayıyı bir artır
    number += 1;

    // Sonucu standart çıktıya yaz
    printf("%d\n", number);

    return 0;
}
