#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

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
    char player1_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    char player2_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    int width;
    int height;
    int turn;
    int game_over;
    int play_conn;
    int play_conn2;
    int game_started;
}BattleShip;

int board_width = 0;
int board_height = 0;


void get_piece_layout(int piece[4][4], int piece_type, int piece_rotation)
{
    memset(piece, 0, sizeof(int) * 4 * 4);

    switch (piece_type)
    {
        case 1: 
            for (int i = 0; i < 2; i++)
                for (int j = 0; j < 2; j++)
                    piece[i][j] = (i == 0 && j == 0) ? 2 : 1;
            break;

        case 2: 
            if (piece_rotation == 1 || piece_rotation == 3) 
            {
                piece[0][0] = 2;
                for (int i = 1; i < 4; i++)
                    piece[i][0] = 1;
            }
            else if (piece_rotation == 2 || piece_rotation == 4) 
            {
                piece[0][0] = 2;
                for (int i = 1; i < 4; i++)
                    piece[0][i] = 1;
            }
            break;

        case 3: 
            if (piece_rotation == 1 || piece_rotation == 3)
            {
                piece[1][0] = 2;
                piece[1][1] = 1;
                piece[0][1] = 1;
                piece[0][2] = 1;
            }
            else if (piece_rotation == 2 || piece_rotation == 4)
            {
                piece[0][0] = 2;
                piece[1][0] = 1;
                piece[1][1] = 1;
                piece[2][1] = 1;
            }
            break;

        case 4: 
            if (piece_rotation == 1)
            {
                piece[0][0] = 2;
                piece[1][0] = 1;
                piece[2][0] = 1;
                piece[2][1] = 1;
            }
            else if (piece_rotation == 2)
            {
                piece[0][0] = 2;
                piece[0][1] = 1;
                piece[0][2] = 1;
                piece[1][0] = 1;
            }
            else if (piece_rotation == 3)
            {
                piece[0][1] = 2;
                piece[1][1] = 1;
                piece[2][1] = 1;
                piece[2][0] = 1;
            }
            else if (piece_rotation == 4)
            {
                piece[0][2] = 2;
                piece[1][0] = 1;
                piece[1][1] = 1;
                piece[1][2] = 1;
            }
            break;

        case 5: 
            if (piece_rotation == 1 || piece_rotation == 3)
            {
                piece[0][1] = 2;
                piece[0][2] = 1;
                piece[1][0] = 1;
                piece[1][1] = 1;
            }
            else if (piece_rotation == 2 || piece_rotation == 4)
            {
                piece[0][0] = 2;
                piece[1][0] = 1;
                piece[1][1] = 1;
                piece[2][1] = 1;
            }
            break;

        case 6: 
            if (piece_rotation == 1)
            {
                piece[0][1] = 2;
                piece[1][1] = 1;
                piece[2][1] = 1;
                piece[2][0] = 1;
            }
            else if (piece_rotation == 2)
            {
                piece[0][0] = 2;
                piece[1][0] = 1;
                piece[1][1] = 1;
                piece[1][2] = 1;
            }
            else if (piece_rotation == 3)
            {
                piece[0][0] = 2;
                piece[0][1] = 1;
                piece[1][0] = 1;
                piece[2][0] = 1;
            }
            else if (piece_rotation == 4)
            {
                piece[0][2] = 2;
                piece[1][0] = 1;
                piece[1][1] = 1;
                piece[1][2] = 1;
            }
            break;

        case 7: // T piece
            if (piece_rotation == 1)
            {
                piece[0][1] = 2;
                piece[1][0] = 1;
                piece[1][1] = 1;
                piece[1][2] = 1;
            }
            else if (piece_rotation == 2)
            {
                piece[0][1] = 2;
                piece[1][0] = 1;
                piece[1][1] = 1;
                piece[2][1] = 1;
            }
            else if (piece_rotation == 3)
            {
                piece[1][1] = 2;
                piece[0][0] = 1;
                piece[0][1] = 1;
                piece[0][2] = 1;
            }
            else if (piece_rotation == 4)
            {
                piece[1][1] = 2;
                piece[0][1] = 1;
                piece[1][0] = 1;
                piece[2][1] = 1;
            }
            break;

        default:
            break;
    }
}


int handle_begin_packet(int client_fd, BattleShip *ship) 
{
    char buffer[BUFFER_SIZE] = {0};
    int nbytes = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (nbytes <= 0)
        return -1;

    buffer[nbytes] = '\0';
    buffer[strcspn(buffer, "\r\n")] = '\0';

    if (buffer[0] == 'F')
    {
        char *forfeit_msg = "H 0";
        char *winner_msg = "H 1";

        send(client_fd, forfeit_msg, strlen(forfeit_msg), 0);
        if (ship->turn == 1)
        {
            send(ship->play_conn2, winner_msg, strlen(winner_msg), 0);
        }
        else
        {
            send(ship->play_conn, winner_msg, strlen(winner_msg), 0);
        }
        printf("[Server] Player forfeited the game.\n");
        return -1;
    }

    if (buffer[0] != 'B')
    {
        send(client_fd, "E 100", strlen("E 100"), 0);
        printf("[Server] Invalid packet type.\n");
        return 0;
    }

    if (ship->game_started == 0)
    {
        int width, height;
        char *ptr = buffer + 2;

        char remaining[BUFFER_SIZE];
        if (sscanf(ptr, "%d %d%s", &width, &height, remaining) == 2)
        {
            if (width >= 10 && height >= 10)
            {
                ship->width = width;
                ship->height = height;
                printf("[Server] Player set the board to %d x %d.\n", ship->width, ship->height);
                send(client_fd, "A", strlen("A"), 0);
                ship->game_started = 1;
                return 1;
            }
            else
            {
                send(client_fd, "E 200", strlen("E 200"), 0);
                printf("[Server] Invalid board dimensions.\n");
                return 0;
            }
        }
        else
        {
            send(client_fd, "E 200", strlen("E 200"), 0);
            printf("[Server] Invalid parameters for Begin packet.\n");
            return 0;
        }
    }
    else
    {
        if (strlen(buffer) == 1)
        {
            printf("[Server] Player accepted the board setup.\n");
            send(client_fd, "A", strlen("A"), 0);
            return 1;
        }
        else
        {
            send(client_fd, "E 200", strlen("E 200"), 0);
            printf("[Server] Invalid parameters for Begin packet.\n");
            return 0;
        }
    }
}
int read_and_validate_packet(int connection, char *buffer, BattleShip *ship) 
{
    int nbytes = read(connection, buffer, BUFFER_SIZE);
    if (nbytes <= 0) 
    {
        perror("[Server] read() failed");
        return -1;
    }

    printf("[Server] Received from player %d: %s\n", ship->turn, buffer);

    if (buffer[0] == 'F') 
    {
        char *forfeit_msg = "H 0";
        char *winner_msg = "H 1";
        send(connection, forfeit_msg, strlen(forfeit_msg) + 1, 0);

        if (ship->turn == 1) 
        {
            send(ship->play_conn2, winner_msg, strlen(winner_msg) + 1, 0);
        } 
        else 
        {
            send(ship->play_conn, winner_msg, strlen(winner_msg) + 1, 0);
        }
        return -1;
    }

    if (buffer[0] != 'I' || buffer[1] != ' ') 
    {
        const char *error = buffer[0] != 'I' ? "E 101" : "E 201";
        send(connection, error, strlen(error) + 1, 0);
        return 0;
    }
    return 1;
}

int parse_packet_parameters(const char *buffer, int *parameters) 
{
    const char *ptr = buffer + 2;
    int param_count = 0;
    int current_num = 0;

    while (*ptr != '\0') 
    {
        if (!isdigit(*ptr)) 
        {
            return 201;
        }

        if (*ptr == '0' && isdigit(*(ptr + 1))) 
        {
            return 201;
        }

        current_num = 0;
        while (isdigit(*ptr)) 
        {
            current_num = current_num * 10 + (*ptr - '0');
            ptr++;
        }

        parameters[param_count++] = current_num;

        if (*ptr != '\0') 
        {
            if (*ptr != ' ') 
            {
                return 201;
            }
            ptr++;

            if (*ptr == ' ' || *ptr == '\0') 
            {
                return 201;
            }
        }
    }

    if (param_count != 20) 
    {
        return 201;
    }

    return 0;
}

void initialize_board(BattleShip *ship, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE])
{
    for (int i = 0; i < ship->height; i++)
    {
        for (int j = 0; j < ship->width; j++)
        {
            board[i][j] = '0';
        }
    }
}

int handle_initialize_packet(int connection, BattleShip *ship) 
{
    char (*board)[MAX_BOARD_SIZE] = (ship->turn == 1) ? ship->player1_board : ship->player2_board;

    for (int i = 0; i < ship->height; i++) 
    {
        memset(board[i], '0', ship->width);
    }

    char buffer[BUFFER_SIZE] = {0};
    int parameters[20] = {0};

    int validation_result = read_and_validate_packet(connection, buffer, ship);
    if (validation_result <= 0) 
    {
        return validation_result;
    }

    int parse_result = parse_packet_parameters(buffer, parameters);
    if (parse_result) 
    {
        char error_msg[10];
        snprintf(error_msg, sizeof(error_msg), "E %d", parse_result);
        send(connection, error_msg, strlen(error_msg) + 1, 0);
        return 0;
    }

    int prio = 0;

    for (int piece = 0; piece < 5; piece++) 
    {
        int shape = parameters[piece * 4];
        int rotation = parameters[piece * 4 + 1];
        int col = parameters[piece * 4 + 2];
        int row = parameters[piece * 4 + 3];
        char ship_num = '1' + piece;

        if (shape < 1 || shape > 7) 
        {
            if (!prio || 300 < prio) 
                prio = 300; 
            continue;
        }

        if (rotation < 1 || rotation > 4) 
        {
            if (!prio || 301 < prio) 
                prio = 301; 
            continue;
        }

        rotation--;

        int stem_y = -1, stem_x = -1;
        int piece_layout[4][4];
        get_piece_layout(piece_layout, shape, rotation);

        for (int y = 0; y < 4 && stem_y == -1; y++) 
        {
            for (int x = 0; x < 4 && stem_x == -1; x++) 
            {
                if (piece_layout[y][x] == 2) 
                {
                    stem_y = y;
                    stem_x = x;
                    break;
                }
            }
        }

        int sizeCheck = 1;

        if (row < 0 || row >= ship->height || col < 0 || col >= ship->width) 
        {
            if (!prio || 302 < prio) 
                prio = 302; 
            sizeCheck = 0;
        } 
        else if (board[row][col] != '0') 
        {
            if (!prio || 303 < prio) 
                prio = 303; 
            sizeCheck = 0;
        } 
        else 
        {
            for (int y = 0; y < 4 && sizeCheck; y++) 
            {
                for (int x = 0; x < 4 && sizeCheck; x++) 
                {
                    if (piece_layout[y][x] == 1) 
                    {
                        int board_y = row + (y - stem_y);
                        int board_x = col + (x - stem_x);

                        if (board_x < 0 || board_x >= ship->width || board_y < 0 || board_y >= ship->height) 
                        {
                            if (!prio || 302 < prio) 
                                prio = 302; 
                            sizeCheck = 0;
                            break;
                        }

                        if (board[board_y][board_x] != '0') 
                        {
                            if (!prio || 303 < prio) 
                                prio = 303;
                            sizeCheck = 0;
                            break;
                        }
                    }
                }
            }
        }

        if (sizeCheck) 
        {
            board[row][col] = ship_num;

            for (int y = 0; y < 4; y++) 
            {
                for (int x = 0; x < 4; x++) 
                {
                    if (piece_layout[y][x] == 1) 
                    {
                        int board_y = row + (y - stem_y);
                        int board_x = col + (x - stem_x);
                        board[board_y][board_x] = ship_num;
                    }
                }
            }
        }
    }

    if (prio) 
    {
        char error_msg[10];
        snprintf(error_msg, sizeof(error_msg), "E %d", prio);
        send(connection, error_msg, strlen(error_msg) + 1, 0);
        return 0;
    }
    const char *ack = "A";
    send(connection, ack, strlen(ack) + 1, 0);
    return 1;
}



int validate_shoot_target(int row, int col, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int width, int height, int connection)
{
    if (row < 0 || row >= height || col < 0 || col >= width)
    {
        const char *error = "E 400";
        send(connection, error, strlen(error) + 1, 0);
        return 0;
    }

    if (board[row][col] == 'M' || board[row][col] == 'H')
    {
        const char *error = "E 401";
        send(connection, error, strlen(error) + 1, 0);
        return 0;
    }

    return 1;
}

int check_ships_remaining(BattleShip *ship)
{
    int ships_left = 0;
    char (*board)[MAX_BOARD_SIZE] = (ship->turn == 1) ? ship->player2_board : ship->player1_board;

    for (int ship_num = 1; ship_num <= MAX_SHIPS; ship_num++)
    {
        char ship_char = ship_num + '0';
        int ship_exists = 0;

        for (int i = 0; i < ship->height && !ship_exists; i++)
        {
            for (int j = 0; j < ship->width && !ship_exists; j++)
            {
                if (board[i][j] == ship_char)
                {
                    ship_exists = 1;
                    ships_left++;
                }
            }
        }
    }

    return ships_left;
}

void process_shoot_result(BattleShip *ship, int row, int col, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int connection)
{
    char msg[20];
    if (board[row][col] == '0') 
    {
        board[row][col] = 'M';
        int ship_count = check_ships_remaining(ship);
        snprintf(msg, sizeof(msg), "R %d M", ship_count);
        send(connection, msg, strlen(msg) + 1, 0);
    } 
    else 
    {
        board[row][col] = 'H';
        int ship_count = check_ships_remaining(ship);
        snprintf(msg, sizeof(msg), "R %d H", ship_count);
        send(connection, msg, strlen(msg) + 1, 0);

        if (ship_count == 0) 
        {
            ship->game_over = 1;
        }
    }
}

int handle_shoot_packet(int connection, char *buffer, BattleShip *ship)
{
    int row, col;
    char extra[BUFFER_SIZE];
    int inputs = sscanf(buffer, "S %d %d %s", &row, &col, extra);

    if (inputs != 2)
    {
        const char *error = "E 202"; 
        send(connection, error, strlen(error) + 1, 0);
        return 0;
    }

    char (*opponent_board)[MAX_BOARD_SIZE] = (ship->turn == 1) ? ship->player2_board : ship->player1_board;

    if (!validate_shoot_target(row, col, opponent_board, ship->width, ship->height, connection))
    {
        return 0; 
    }

    char result_msg[BUFFER_SIZE];
    if (opponent_board[row][col] == '0')
    {
        opponent_board[row][col] = 'M'; 
        int remaining_ships = check_ships_remaining(ship);
        snprintf(result_msg, sizeof(result_msg), "R %d M", remaining_ships);
        send(connection, result_msg, strlen(result_msg) + 1, 0);
    }
    else
    {
        opponent_board[row][col] = 'H'; 
        int remaining_ships = check_ships_remaining(ship);
        snprintf(result_msg, sizeof(result_msg), "R %d H", remaining_ships);
        send(connection, result_msg, strlen(result_msg) + 1, 0);

        if (remaining_ships == 0) 
{
    char loser_buffer[BUFFER_SIZE] = {0};
    int loser_connection = (ship->turn == 1) ? ship->play_conn2 : ship->play_conn;
    read(loser_connection, loser_buffer, BUFFER_SIZE - 1);
    const char *loser_msg = "H 0";
    send(loser_connection, loser_msg, strlen(loser_msg) + 1, 0);
    char winner_buffer[BUFFER_SIZE] = {0};
    int winner_connection = connection;
    read(winner_connection, winner_buffer, BUFFER_SIZE - 1);
    const char *winner_msg = "H 1";
    send(winner_connection, winner_msg, strlen(winner_msg) + 1, 0);

    printf("[Server] Player %d wins. All ships sunk for Player %d.\n", ship->turn, (ship->turn == 1) ? 2 : 1);

    ship->game_over = 1;

    return -1; 
}

    }

    printf("[Server] Switching turn from Player %d to Player %d.\n", ship->turn, (ship->turn == 1) ? 2 : 1);
    ship->turn = (ship->turn == 1) ? 2 : 1;

    return 1;
}




int count_remaining_ships(BattleShip *ship) 
{
    int remaining_ships = 0;
    char (*opponent_board)[MAX_BOARD_SIZE] = (ship->turn == 1) ? ship->player2_board : ship->player1_board;

    for (int ship_number = 1; ship_number <= 5; ship_number++) 
    {
        char ship_identifier = ship_number + '0';
        int ship_found = 0;

        for (int row = 0; row < ship->height && !ship_found; row++) 
        {
            for (int col = 0; col < ship->width && !ship_found; col++) 
            {
                if (opponent_board[row][col] == ship_identifier) 
                {
                    ship_found = 1;
                    remaining_ships++;
                }
            }
        }
    }
    return remaining_ships;
}

void handle_query(BattleShip *ship)
{
    char q_res[3 + (7 * ship->width * ship->height) + 1];
    char *ptr = q_res;
    char (*current_board)[MAX_BOARD_SIZE];
    int play_conn;
    int remaining_ships = check_ships_remaining(ship);

    *ptr++ = 'G';
    *ptr++ = ' ';
    *ptr++ = remaining_ships + '0';
    *ptr++ = ' ';

    if (ship->turn == 1)
    {
        current_board = ship->player2_board;
        play_conn = ship->play_conn;
    }
    else
    {
        current_board = ship->player1_board;
        play_conn = ship->play_conn2;
    }

    for (int row = 0; row < ship->height; row++)
    {
        for (int col = 0; col < ship->width; col++)
        {
            if (current_board[row][col] == 'H')
            {
                *ptr++ = 'H';
                *ptr++ = ' ';
                *ptr++ = row + '0';
                *ptr++ = ' ';
                *ptr++ = col + '0';
                *ptr++ = ' ';
            }
            else if (current_board[row][col] == 'M')
            {
                *ptr++ = 'M';
                *ptr++ = ' ';
                *ptr++ = row + '0';
                *ptr++ = ' ';
                *ptr++ = col + '0';
                *ptr++ = ' ';
            }
        }
    }

    if (ptr > q_res)
    {
        *(ptr - 1) = '\0';
    }
    else
    {
        *ptr = '\0';
    }

    send(play_conn, q_res, strlen(q_res) + 1, 0);
}

int main() 
{
    int server_fd1, server_fd2, client1_fd, client2_fd;
    struct sockaddr_in address1, address2, client_address1, client_address2;
    socklen_t addrlen1 = sizeof(client_address1), addrlen2 = sizeof(client_address2);
    int opt = 1;

    server_fd1 = socket(AF_INET, SOCK_STREAM, 0);
    server_fd2 = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd1 < 0 || server_fd2 < 0) 
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed for Player 1");
        close(server_fd1);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd2, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed for Player 2");
        close(server_fd2);
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
        close(server_fd1);
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd2, (struct sockaddr *)&address2, sizeof(address2)) < 0) 
    {
        perror("Bind failed for Player 2");
        close(server_fd1);
        close(server_fd2);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd1, 1) < 0) 
    {
        perror("Listen failed for Player 1");
        close(server_fd1);
        close(server_fd2);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd2, 1) < 0) 
    {
        perror("Listen failed for Player 2");
        close(server_fd1);
        close(server_fd2);
        exit(EXIT_FAILURE);
    }

    printf("[Server] Waiting for Player 1 to connect on port %d...\n", PORT1);
    client1_fd = accept(server_fd1, (struct sockaddr *)&client_address1, &addrlen1);
    if (client1_fd < 0) 
    {
        perror("Accept failed for Player 1");
        close(server_fd1);
        close(server_fd2);
        exit(EXIT_FAILURE);
    }
    printf("[Server] Player 1 connected.\n");

    printf("[Server] Waiting for Player 2 to connect on port %d...\n", PORT2);
    client2_fd = accept(server_fd2, (struct sockaddr *)&client_address2, &addrlen2);
    if (client2_fd < 0) 
    {
        perror("Accept failed for Player 2");
        close(server_fd1);
        close(client1_fd);
        close(server_fd2);
        exit(EXIT_FAILURE);
    }
    printf("[Server] Player 2 connected.\n");

    BattleShip set_ship = {0};
    set_ship.play_conn = client1_fd;
    set_ship.play_conn2 = client2_fd;
    set_ship.turn = 1;
    set_ship.game_over = 0;

for (int i = 0; i < MAX_BOARD_SIZE; i++) 
{
    for (int j = 0; j < MAX_BOARD_SIZE; j++) 
    {
        set_ship.player1_board[i][j] = '0';
        set_ship.player2_board[i][j] = '0';
    }
}

set_ship.turn = 1; 
int begin_phase_complete = 0;
int p1_result = 0;
int error_occurred = 0; 

while (!begin_phase_complete && !error_occurred) 
{
    if (set_ship.turn == 1) 
    {
        int p1_result = handle_begin_packet(client1_fd, &set_ship);
        if (p1_result == -1) 
        {
            error_occurred = 1; 
        } 
        else if (p1_result == 1) 
        {
            set_ship.turn = 2;
        }
    } 
    else if (set_ship.turn == 2) 
    {
        int p2_result = handle_begin_packet(client2_fd, &set_ship);
        if (p2_result == -1) 
        {
            error_occurred = 1; 
        } 
        else if (p2_result == 1) 
        {
            begin_phase_complete = 1;
        }
    }
}
if (error_occurred) 
{
    close(client1_fd);
    close(client2_fd);
    close(server_fd1);
    close(server_fd2);
    return -1; 
}
printf("[Server] Both players completed the begin phase.\n");

set_ship.turn = 1; 
int initialization_phase_complete = 0;
error_occurred = 0; 

while (!initialization_phase_complete && !error_occurred) 
{
    if (set_ship.turn == 1) 
    {
        int p1_result = handle_initialize_packet(client1_fd, &set_ship);
        if (p1_result == -1) 
        {
            error_occurred = 1; 
        } 
        else if (p1_result == 1) 
        {
            set_ship.turn = 2;
        }
    } 
    else if (set_ship.turn == 2) 
    {
        int p2_result = handle_initialize_packet(client2_fd, &set_ship);
        if (p2_result == -1) 
        {
            error_occurred = 1; 
        } 
        else if (p2_result == 1) 
        {
            initialization_phase_complete = 1;
        }
    }
}

if (initialization_phase_complete) 
{
    set_ship.turn = 1; 
}

if (error_occurred) 
{
    close(client1_fd);
    close(client2_fd);
    close(server_fd1);
    close(server_fd2);
    return -1; 
}
while (!set_ship.game_over) 
{
    int connection = (set_ship.turn == 1) ? set_ship.play_conn : set_ship.play_conn2;

    char buffer[BUFFER_SIZE] = {0};
    int nbytes = read(connection, buffer, BUFFER_SIZE - 1);

    if (nbytes <= 0) 
    {
        perror("[Server] read() failed");
        break; 
    }

    buffer[nbytes] = '\0';
    printf("[Server] Received from player %d: %s\n", set_ship.turn, buffer);

    char buffer_copy[BUFFER_SIZE];
    strncpy(buffer_copy, buffer, BUFFER_SIZE);
    char *command = strtok(buffer_copy, "\n");

    while (command) 
    {
        printf("[Server] Player %d command: %s\n", set_ship.turn, command);

        if (command[0] == 'F') 
        {
            char *forfeit_msg = "H 0";
            char *winner_msg = "H 1";
            int other_connection = (set_ship.turn == 1) ? set_ship.play_conn2 : set_ship.play_conn;

            send(connection, forfeit_msg, strlen(forfeit_msg) + 1, 0);
            send(other_connection, winner_msg, strlen(winner_msg) + 1, 0);

            printf("[Server] Player %d forfeited. Player %d wins.\n", set_ship.turn, (set_ship.turn == 1) ? 2 : 1);
            set_ship.game_over = 1;
            break;
        }
        else if (command[0] == 'S') 
        {
            int shoot_result = handle_shoot_packet(connection, command, &set_ship);

            if (shoot_result < 0) 
            {
                set_ship.game_over = 1;
                break;
            }
        }
        else if (command[0] == 'Q') 
        {
            handle_query(&set_ship);
        }
        else 
        {
            send(connection, "E 102", strlen("E 102") + 1, 0);
            printf("[Server] Invalid command from player %d: %s\n", set_ship.turn, command);
        }

        command = strtok(NULL, "\n");
    }
}

close(client1_fd);
close(client2_fd);
close(server_fd1);
close(server_fd2);
return 0;
}