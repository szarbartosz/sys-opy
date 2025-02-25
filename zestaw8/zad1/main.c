#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_LENGTH 70
#define WHITESPACE " \t\r\n"

int height;
int width;
unsigned char **img;

int threads_no;
int **histogram_parts;

int calculate_time(struct timespec *start) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    int retval = (end.tv_sec - start->tv_sec) * 1000000;
    retval += (end.tv_nsec - start->tv_nsec) / 1000.0;
    return retval;
}

void read_line(char *buffer, FILE *input) {
    do {
        fgets(buffer, MAX_LINE_LENGTH, input);
    } while (buffer[0] == '#' || buffer[0] == '\n');
}

void load_img(char *image_filename) {
    FILE *input = fopen(image_filename, "r");

    char buffer[MAX_LINE_LENGTH + 1] = {0};

    read_line(buffer, input);
    read_line(buffer, input);

    width = atoi(strtok(buffer, WHITESPACE));
    height = atoi(strtok(NULL, WHITESPACE));

    img = calloc(height, sizeof(char *));
    for (int i = 0; i < height; i++) {
        img[i] = calloc(width, sizeof(char));
    }

    read_line(buffer, input);
    read_line(buffer, input);
    
    char *encoded_gray_value = strtok(buffer, WHITESPACE);
    for (int i = 0; i < width * height; i++) {
        if (encoded_gray_value == NULL) {
            read_line(buffer, input);
            encoded_gray_value = strtok(buffer, WHITESPACE);
        }

        img[i / width][i % width] = atoi(encoded_gray_value);

        encoded_gray_value = strtok(NULL, WHITESPACE);
    }

    fclose(input);
}

void save_histogram(char *histogram_filename) {
    FILE *output = fopen(histogram_filename, "w+");

    int histogram[256] = {0};
    for (int i = 0; i < threads_no; i++) {
        for (int x = 0; x < 256; x++) {
            histogram[x] += histogram_parts[i][x];
        }
    }

    float max_occurence = histogram[0];
    for (int i = 1; i < 256; i++) {
        if (max_occurence < histogram[i]) {
            max_occurence = histogram[i];
        }
    }

    fputs("P2\n", output);
    fputs("256 50\n", output);
    fputs("255\n", output);

    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 256; x++) {
            if (50 - y > 50 * (histogram[x] / max_occurence)) {
                fputs("0\n", output);
            } else {
                fprintf(output, "255\n");
            }
        }
    }

    fclose(output);
}

int sign_worker(int *thread_index) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int chunk_size = 256 / threads_no;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (img[y][x] / chunk_size == *thread_index) {
                histogram_parts[0][img[y][x]]++;
            }
        }
    }
    return calculate_time(&start);
}

int block_worker(int *thread_index) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int k = *thread_index;
    int chunk_size = width / threads_no;
    for (int x = k * chunk_size; x < (k + 1) * chunk_size; x++) {
        for (int y = 0; y < height; y++) {
            histogram_parts[k][img[y][x]]++;
        }
    }

    return calculate_time(&start);
}

int interleaved_worker(int *thread_index) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int k = *thread_index;
    for (int x = k; x < width; x += threads_no) {
        for (int y = 0; y < height; y++) {
            histogram_parts[k][img[y][x]]++;
        }
    }

    return calculate_time(&start);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "to run enter: ./main threads_count mode input_file output_file");
        return 1;
    }

    threads_no = atoi(argv[1]);
    char *encoded_mode = argv[2];
    char *input_file = argv[3];
    char *output_file = argv[4];

    load_img(input_file);
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    histogram_parts = calloc(threads_no, sizeof(int *));
    for (int i = 0; i < threads_no; i++) {
        histogram_parts[i] = calloc(256, sizeof(int));
    }

    pthread_t *threads = calloc(threads_no, sizeof(pthread_t));
    int *args = calloc(threads_no, sizeof(int));

    for (int i = 0; i < threads_no; i++) {
        int (*start)(int *);
        if (strcmp(encoded_mode, "sign") == 0) {
            start = sign_worker;
        }
        if (strcmp(encoded_mode, "block") == 0) {
            start = block_worker;
        }
        if (strcmp(encoded_mode, "interleaved") == 0) {
            start = interleaved_worker;
        }

        args[i] = i;
        pthread_create(&threads[i], NULL, (void *(*)(void *))start, args + i);
    }

    for (int i = 0; i < threads_no; i++) {
        int time;
        pthread_join(threads[i], (void *)&time);
        printf("thread %d took %d microseconds\n", i, time);
    }

    printf("total time: %d microseconds\n", calculate_time(&start));

    save_histogram(output_file);

    free(threads);
    free(args);

    for (int i = 0; i < threads_no; i++) {
        free(histogram_parts[i]);
    }
    free(histogram_parts);
    for (int y = 0; y < height; y++) {
        free(img[y]);
    }
    free(img);
}