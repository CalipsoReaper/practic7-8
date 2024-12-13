#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#define FIFO_GUESS "/tmp/fifo_guess"
#define FIFO_RESULT "/tmp/fifo_result"

void cleanup() {
    unlink(FIFO_GUESS);
    unlink(FIFO_RESULT);
}

int main() {
    mkfifo(FIFO_GUESS, 0666);
    mkfifo(FIFO_RESULT, 0666);
    atexit(cleanup);  // Удалить файлы FIFO

    pid_t child_pid = fork();
    if (child_pid == 0) {
    
        srand(time(NULL) ^ getpid());

        int guess_pipe = open(FIFO_GUESS, O_WRONLY);
        int result_pipe = open(FIFO_RESULT, O_RDONLY);

        while (1) {
            int guess = rand() % 100 + 1;
            printf("Процесс %d предполагает: %d\n", getpid(), guess);
            write(guess_pipe, &guess, sizeof(int));

            int result;
            read(result_pipe, &result, sizeof(int));

            if (result == 1) {
                printf("Процесс %d угадал!\n", getpid());
                break;
            } else {
                printf("Процесс %d не угадал. Продолжаю...\n", getpid());
            }
        }

        close(guess_pipe);
        close(result_pipe);
    } else {
        // Родительский процесс 
        srand(time(NULL));
        int secret_number = rand() % 100 + 1;
        printf("Загадано число: %d\n", secret_number);

        int guess_pipe = open(FIFO_GUESS, O_RDONLY);
        int result_pipe = open(FIFO_RESULT, O_WRONLY);

        while (1) {
            int guess;
            read(guess_pipe, &guess, sizeof(int));
            printf("Получено предположение: %d\n", guess);

            int result = (guess == secret_number) ? 1 : 0;
            write(result_pipe, &result, sizeof(int));

            if (result == 1) {
                printf("Число угадано! Начинается новая игра...\n");
                secret_number = rand() % 100 + 1;
                printf("Новое загаданное число: %d\n", secret_number);
            }
        }

        close(guess_pipe);
        close(result_pipe);
    }

    return 0;
}
