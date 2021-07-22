#include <stdio.h>
#include <string.h>

void main() {
    char buf[300];
    char * piece;
    char * string_splited[10];
    int i = 0;

    printf("Enter a message: ");
    fgets(buf, sizeof(buf), stdin);

    piece = strtok(buf, " ");

    while (piece != NULL)
    {
        printf("%s\n", piece);
        string_splited[i] = piece; 
        piece = strtok(NULL, " ");
        i++;

        if (i >= 3) {
            break;
        }
    }

    for (int j = 0; j < i; j++)
        printf("%s\n", string_splited[j]);
}