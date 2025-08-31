#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main() {
    // Template to print something
    printf("Hello, World!\n");
    
    // Example using different print methods
    printf("This is a formatted string: %d\n", 42);
    
    // Using write() from unistd.h
    uint8_t  binary=0b00000001;
    printf("Binary representation: %d",);
    for (int bit= 0; bit <8; bit++) {
        printf("%d", (binary & (1 << bit)));
        if ((binary & (1 << bit))==0){
            printf("free position found: %d\n", bit+1);//say 1 indexed

            break;
        }
    }
    
    // Example with string manipulation
  
    
    return 0;
}