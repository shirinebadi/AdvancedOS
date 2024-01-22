#include <stdio.h>
#include <fcntl.h>

void read_file(){
    FILE *file;
    char buffer[1024]; 

    // Open the file in binary read mode
    file = fopen("/dev/testq", "r");

    if (file == NULL) {
        perror("Error opening file");
        fclose(file);
    }

    // Read from the file into the buffer
    fgets(buffer, sizeof(buffer), file);
    printf("Read line: %s\n", buffer);

    fclose(file);
}

void write_file(){
    FILE *file;
    char buffer[1024]; 
    const char *data = "Advanced OS question";

    // Open the file in binary read mode
    file = fopen("/dev/testq", "w");

    if (file == NULL) {
        perror("Error opening file");
    }


    size_t bytes_written = fwrite(data, sizeof(char), strlen(data),file);

    if (bytes_written != strlen(data)) {
        perror("Error writing to file");
        fclose(file);
    }

    fclose(file);
}

void seek_file(){
    FILE *file;
    char buffer[1024]; 
    file = fopen("/dev/testq", "r+");


    fseek(file, 5, SEEK_SET);

    long position = ftell(file);
    if (position == -1) {
        perror("Error getting file position");
        fclose(file);
    }

    printf("Current file position: %ld\n", position);

    fclose(file);
}

int main() {
    read_file();

    sleep(8);
    read_file();

    write_file();

    sleep(5);
    
    read_file();
    seek_file();
}
