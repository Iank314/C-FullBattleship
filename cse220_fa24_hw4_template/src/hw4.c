#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT1 2201  
#define PORT2 2202  
#define BUFFER_SIZE 1024
#define MAX_BOARD_SIZE 20

typedef struct {
    int width, height;
    char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
} GameBoard;

GameBoard board;
int player_ready[2] = {0, 0};

void initialize_board(GameBoard *board, int width, int height) {
    board->width = width;
    board->height = height;
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
            board->board[i][j] = '.';
}

void handle_client(int client_fd, int opponent_fd, int player_number) {
    char buffer[BUFFER_SIZE] = {0};
    int width, height;
    int forfeit_triggered = 0;
    
    while (!forfeit_triggered) {
        memset(buffer, 0, BUFFER_SIZE);
        int nbytes = read(client_fd, buffer, BUFFER_SIZE);
        if (nbytes <= 0) break;

        printf("[Server] Received from Player %d: %s\n", player_number, buffer);

        if (buffer[0] == 'B') {
            if (player_number == 1) {
                if (sscanf(buffer, "B %d %d", &width, &height) == 2 && width >= 10 && height >= 10) {
                    initialize_board(&board, width, height);
                    player_ready[0] = 1;
                    send(client_fd, "A", 1, 0);  
                } else {
                    send(client_fd, "E 200", strlen("E 200"), 0);  
                }
            } else if (player_number == 2 && strcmp(buffer, "B") == 0) {
                player_ready[1] = 1;
                send(client_fd, "A", 1, 0);  
            } else {
                send(client_fd, "E 200", strlen("E 200"), 0);  
            }
        }
        else if (buffer[0] == 'I') {
            if (player_ready[0] && player_ready[1]) {
                send(client_fd, "A", 1, 0);
            } else {
                send(client_fd, "E 101", strlen("E 101"), 0);  
            }
        }
        else if (buffer[0] == 'F') {
            send(client_fd, "H 0", strlen("H 0"), 0);
            send(opponent_fd, "H 1", strlen("H 1"), 0);
            forfeit_triggered = 1;
        }
        else {
            send(client_fd, "E 100", strlen("E 100"), 0); 
        }
    }

    close(client_fd);
    close(opponent_fd);
    printf("[Server] Game ended due to forfeit by Player %d\n", player_number);
}

int main() {
    int server_fd1, server_fd2, client1_fd, client2_fd;
    struct sockaddr_in address1, address2;
    int addrlen = sizeof(address1);

    server_fd1 = socket(AF_INET, SOCK_STREAM, 0);
    server_fd2 = socket(AF_INET, SOCK_STREAM, 0);

    address1.sin_family = AF_INET;
    address1.sin_addr.s_addr = INADDR_ANY;
    address1.sin_port = htons(PORT1);

    address2.sin_family = AF_INET;
    address2.sin_addr.s_addr = INADDR_ANY;
    address2.sin_port = htons(PORT2);

    bind(server_fd1, (struct sockaddr *)&address1, sizeof(address1));
    bind(server_fd2, (struct sockaddr *)&address2, sizeof(address2));

    listen(server_fd1, 1);
    listen(server_fd2, 1);

    client1_fd = accept(server_fd1, (struct sockaddr *)&address1, (socklen_t *)&addrlen);
    client2_fd = accept(server_fd2, (struct sockaddr *)&address2, (socklen_t *)&addrlen);

    handle_client(client1_fd, client2_fd, 1);
    handle_client(client2_fd, client1_fd, 2);

    close(server_fd1);
    close(server_fd2);

    return 0;
}