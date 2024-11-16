#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT1 2201
#define PORT2 2202
#define BUFFER_SIZE 1024
#define MAX_SHIPS 5
#define MAX_BOARD_SIZE 100

typedef struct
{
    int type;
    int rotation;
    int col;
    int row;
    int cells[4][4];
} Ship;

typedef struct
{
    int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    Ship ships[MAX_SHIPS];
    int ship_count;
} PlayerState;


int main()
{
    int server_fd1, server_fd2, client1_fd, client2_fd;
    struct sockaddr_in address1, address2;
    int addrlen = sizeof(address1);

    server_fd1 = socket(AF_INET, SOCK_STREAM, 0);
    server_fd2 = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd1 == 0 || server_fd2 == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address1.sin_family = AF_INET;
    address1.sin_addr.s_addr = INADDR_ANY;
    address1.sin_port = htons(PORT1);

    address2.sin_family = AF_INET;
    address2.sin_addr.s_addr = INADDR_ANY;
    address2.sin_port = htons(PORT2);

    if (bind(server_fd1, (struct sockaddr *)&address1, sizeof(address1)) < 0)
    {
        perror("Bind failed for Player 1");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd2, (struct sockaddr *)&address2, sizeof(address2)) < 0)
    {
        perror("Bind failed for Player 2");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd1, 1) < 0)
    {
        perror("Listen failed for Player 1");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd2, 1) < 0)
    {
        perror("Listen failed for Player 2");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for Player 1 to connect on port %d...\n", PORT1);
    client1_fd = accept(server_fd1, (struct sockaddr *)&address1, (socklen_t *)&addrlen);
    if (client1_fd < 0)
    {
        perror("Accept failed for Player 1");
        exit(EXIT_FAILURE);
    }
    printf("Player 1 connected.\n");

    printf("Waiting for Player 2 to connect on port %d...\n", PORT2);
    client2_fd = accept(server_fd2, (struct sockaddr *)&address2, (socklen_t *)&addrlen);
    if (client2_fd < 0)
    {
        perror("Accept failed for Player 2");
        exit(EXIT_FAILURE);
    }
    printf("Player 2 connected.\n");

    int forfeit_triggered = 0;
    int current_turn = 1;
    int game_started = 0; 
    int initialized = 0;  

while (!forfeit_triggered)
{
    char buffer[BUFFER_SIZE] = {0};
    int nbytes;

    if (current_turn == 1)
    {
        printf("[Server] Waiting for Player 1's move.\n");
        nbytes = read(client1_fd, buffer, BUFFER_SIZE - 1);
        if (nbytes <= 0) return 0;
        buffer[nbytes] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (buffer[0] == 'F' && buffer[1] == '\0')
        {
            printf("[Server] Player 1 forfeited the game.\n");
            forfeit_triggered = 1;
            continue;
        }

        if (buffer[0] == 'B' && game_started == 0)
        {
            int width, height;
            char *ptr = buffer + 2;

            if (sscanf(ptr, "%d %d", &width, &height) == 2) 
{
    char *space_position = strchr(ptr, ' ');  
    if (space_position != NULL && strchr(space_position + 1, ' ') == NULL) 
    {
        if (width >= 10 && height >= 10) 
        {
            printf("[Server] Player 1 set the board to %d x %d.\n", width, height);
            send(client1_fd, "A", strlen("A"), 0);
            game_started = 1;
            current_turn = 2;
            continue;
        }
        else 
        {
            send(client1_fd, "E 200", strlen("E 200"), 0);
            printf("[Server] Invalid board dimensions from Player 1.\n");
            continue;
        }
    }
    else 
    {
        send(client1_fd, "E 200", strlen("E 200"), 0);
        printf("[Server] Invalid parameters for Begin packet from Player 1.\n");
        continue;
    }
}
else 
{
    send(client1_fd, "E 200", strlen("E 200"), 0);
    printf("[Server] Invalid parameters for Begin packet from Player 1.\n");
    continue;
}
        }

        if (game_started == 0)
        {
            send(client1_fd, "E 100", strlen("E 100"), 0);
            printf("[Server] Expected 'B' or 'F' as the first move from Player 1.\n");
            continue;
        }

        if (game_started == 2 && !initialized)
        {
            if (buffer[0] == 'I' && buffer[1] == '\0')
            {
                printf("[Server] Player 1 initialized the game.\n");
                send(client1_fd, "A", strlen("A"), 0);
                initialized = 1;
                current_turn = 2;
                continue;
            }
            else
            {
                send(client1_fd, "E 101", strlen("E 101"), 0);
                printf("[Server] Expected Initialize from Player 1.\n");
                continue;
            }
        }

        if (game_started == 2 && initialized)
        {
            if (buffer[0] == 'S' && buffer[1] == '\0')
            {
                printf("[Server] Player 1 made a valid move: Shoot.\n");
                send(client1_fd, "A", strlen("A"), 0);
                current_turn = 2;
                continue;
            }

            if (buffer[0] == 'Q' && buffer[1] == '\0')
            {
                printf("[Server] Player 1 made a valid move: Query.\n");
                send(client1_fd, "A", strlen("A"), 0);
                current_turn = 2;
                continue;
            }

            if (buffer[0] == 'I' && buffer[1] == '\0')
            {
                printf("[Server] Player 1 made a valid move: Initialize.\n");
                send(client1_fd, "A", strlen("A"), 0);
                current_turn = 2;
                continue;
            }

            send(client1_fd, "E 102", strlen("E 102"), 0);
            printf("[Server] Invalid move from Player 1.\n");
            continue;
        }
    }

    if (current_turn == 2)
    {
        printf("[Server] Waiting for Player 2's move.\n");
        nbytes = read(client2_fd, buffer, BUFFER_SIZE - 1);
        if (nbytes <= 0) return 0;
        buffer[nbytes] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (buffer[0] == 'F' && buffer[1] == '\0')
        {
            printf("[Server] Player 2 forfeited the game.\n");
            forfeit_triggered = 2;
            continue;
        }

        if (buffer[0] == 'B' && game_started == 1)
        {
            if (buffer[1] == '\0')
            {
                printf("[Server] Player 2 accepted the board setup.\n");
                send(client2_fd, "A", strlen("A"), 0);
                game_started = 2;
                current_turn = 1;
                continue;
            }
            else
            {
                send(client2_fd, "E 200", strlen("E 200"), 0);
                printf("[Server] Invalid parameters for Begin packet from Player 2.\n");
                continue;
            }
        }

        if (game_started == 1)
        {
            send(client2_fd, "E 100", strlen("E 100"), 0);
            printf("[Server] Expected 'B' or 'F' as the first move from Player 2.\n");
            continue;
        }

        if (game_started == 2 && !initialized)
        {
            if (buffer[0] == 'I' && buffer[1] == '\0')
            {
                printf("[Server] Player 2 initialized the game.\n");
                send(client2_fd, "A", strlen("A"), 0);
                initialized = 1;
                current_turn = 1;
                continue;
            }
            else
            {
                send(client2_fd, "E 101", strlen("E 101"), 0);
                printf("[Server] Expected Initialize from Player 2.\n");
                continue;
            }
        }

        if (game_started == 2 && initialized)
        {
            if (buffer[0] == 'S' && buffer[1] == '\0')
            {
                printf("[Server] Player 2 made a valid move: Shoot.\n");
                send(client2_fd, "A", strlen("A"), 0);
                current_turn = 1;
                continue;
            }

            if (buffer[0] == 'Q' && buffer[1] == '\0')
            {
                printf("[Server] Player 2 made a valid move: Query.\n");
                send(client2_fd, "A", strlen("A"), 0);
                current_turn = 1;
                continue;
            }

            if (buffer[0] == 'I' && buffer[1] == '\0')
            {
                printf("[Server] Player 2 made a valid move: Initialize.\n");
                send(client2_fd, "A", strlen("A"), 0);
                current_turn = 1;
                continue;
            }

            send(client2_fd, "E 102", strlen("E 102"), 0);
            printf("[Server] Invalid move from Player 2.\n");
            continue;
        }
    }
}

int winner = (forfeit_triggered == 1) ? 2 : 1;
send(client1_fd, (winner == 1) ? "H 1" : "H 0", strlen("H 1"), 0);
send(client2_fd, (winner == 2) ? "H 1" : "H 0", strlen("H 1"), 0);

printf("[Server] Closing connections...\n");
sleep(1);
close(client1_fd);
close(client2_fd);
printf("[Server] Both connections closed.\n");

return 0;
}