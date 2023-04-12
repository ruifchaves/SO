//resolução quarta feira 02/03/2022
int main (int argc, char ** agrv){
    int line_counter = 0;
    char buffer[MAX_READ_BUFFER]
    int bytes_read = 0;
    int newline = 1;

    while((bytes_read = readln (0, buffer, MAX_READ_BUFFER)) > 0){
        char line_number[10] = "";
        //nl skips empty lines
        if (newline && buffer[0] != '\n'){
            snprintf(line_number, 10, "%d:", line_counter);
            write(1, line_counter, sizeof(line_number));
            line_counter++;
        }
        write(1, buffer, bytes_read);

        //buffer was not big enough to hold the whole line, continue reading the line
        if(buffer[bytes_read - 1] != '\n') newline = 0;
        else newline = 1;
    }
}