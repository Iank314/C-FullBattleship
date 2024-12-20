#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <limits.h>

#define PORT1 2201
#define PORT2 2202
#define BUFFER_SIZE 1024
#define MAX_SHIPS 5
#define MAX_BOARD_SIZE 250

typedef struct
{
    int type;
    int moved;
    int y_axis;
    int x_axis;
    int cells[4][4];
} Ship;

typedef struct
{
    char player1_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    char player2_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    int width;
    int height;
    int turn;
    int game_ended;
    int play_conn;
    int play_conn2;
    int game_started;
}BattleShip;

int board_width = 0;
int board_height = 0;

const char Shape1[4][4][4] = 
{
    { {'2', '1', '0', '0'}, {'1', '1', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '1', '0', '0'}, {'1', '1', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '1', '0', '0'}, {'1', '1', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '1', '0', '0'}, {'1', '1', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} }
};

const char Shape2[4][4][4] = 
{
    { {'2', '0', '0', '0'}, {'1', '0', '0', '0'}, {'1', '0', '0', '0'}, {'1', '0', '0', '0'} },
    { {'2', '1', '1', '1'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '0', '0', '0'}, {'1', '0', '0', '0'}, {'1', '0', '0', '0'}, {'1', '0', '0', '0'} },
    { {'2', '1', '1', '1'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} }
};

const char Shape3[4][4][4] = 
{
    { {'0', '1', '1', '0'}, {'2', '1', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '0', '0', '0'}, {'1', '1', '0', '0'}, {'0', '1', '0', '0'}, {'0', '0', '0', '0'} },
    { {'0', '1', '1', '0'}, {'2', '1', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '0', '0', '0'}, {'1', '1', '0', '0'}, {'0', '1', '0', '0'}, {'0', '0', '0', '0'} }
};

const char Shape4[4][4][4] =
{
    { {'2', '0', '0', '0'}, {'1', '0', '0', '0'}, {'1', '1', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '1', '1', '0'}, {'1', '0', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '1', '0', '0'}, {'0', '1', '0', '0'}, {'0', '1', '0', '0'}, {'0', '0', '0', '0'} },
    { {'0', '0', '1', '0'}, {'2', '1', '1', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} }
};

const char Shape5[4][4][4] = 
{
    { {'2', '1', '0', '0'}, {'0', '1', '1', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'0', '1', '0', '0'}, {'2', '1', '0', '0'}, {'1', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '1', '0', '0'}, {'0', '1', '1', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'0', '1', '0', '0'}, {'2', '1', '0', '0'}, {'1', '0', '0', '0'}, {'0', '0', '0', '0'} }
};

const char Shape6[4][4][4] = 
{
    { {'0', '1', '0', '0'}, {'0', '1', '0', '0'}, {'2', '1', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '0', '0', '0'}, {'1', '1', '1', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '1', '0', '0'}, {'1', '0', '0', '0'}, {'1', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '1', '1', '0'}, {'0', '0', '1', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} }
};

const char Shape7[4][4][4] = 
{
    { {'2', '1', '1', '0'}, {'0', '1', '0', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'0', '1', '0', '0'}, {'2', '1', '0', '0'}, {'0', '1', '0', '0'}, {'0', '0', '0', '0'} },
    { {'0', '1', '0', '0'}, {'2', '1', '1', '0'}, {'0', '0', '0', '0'}, {'0', '0', '0', '0'} },
    { {'2', '0', '0', '0'}, {'1', '1', '0', '0'}, {'1', '0', '0', '0'}, {'0', '0', '0', '0'} }
};

const char (*AllShapes[7])[4][4] = 
{ 
    Shape1, Shape2, Shape3, Shape4, Shape5, Shape6, Shape7 
};


int read_and_validate_packet(int connection, char *buffer, BattleShip *ship) 
{
    int nbytes = read(connection, buffer, BUFFER_SIZE - 1);
    if (nbytes <= 0) 
    {
        perror("[Server] read() failed");
        return -1;
    }

    buffer[nbytes] = '\0'; 

    printf("[Server] Received from player %d: %s\n", ship->turn, buffer);

    if (buffer[0] == 'F') 
    {
        const char *ff = "H 0";
        const char *gg = "H 1";
        send(connection, ff, strlen(ff) + 1, 0);

        if (ship->turn == 1) 
        {
            send(ship->play_conn2, gg, strlen(gg) + 1, 0);
        } 
        else 
        {
            send(ship->play_conn, gg, strlen(gg) + 1, 0);
        }
        return -1;
    }

    if (nbytes < 2 || buffer[0] != 'I' || buffer[1] != ' ') 
    {
        const char *error;
    if (buffer[0] != 'I')
    {
        error = "E 101";
    }
    else
    {
        error = "E 201";
    }
        send(connection, error, strlen(error) + 1, 0);
        return 0;
    }

    return 1;
}

int parse_packet_parameters(const char *buffer, int *parameters) 
{
    const char *ptr = buffer + 2;
    int param_count = 0;

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

        int current_num = 0;
        while (isdigit(*ptr)) 
        {
            current_num = current_num * 10 + (*ptr - '0');
            ptr++;
        }

        parameters[param_count++] = current_num;

        if (param_count > 20) 
        {
            return 201;
        }

        if (*ptr == '\0') 
        {
            break;
        }
        else if (*ptr == ' ') 
        {
            ptr++;
            if (*ptr == '\0') 
            {
                return 201; 
            }
        }
        else 
        {
            return 201; 
        }
    }

    if (param_count != 20) 
    {
        return 201; 
    }

    return 0; 
}



void initialize_board(char (*board)[MAX_BOARD_SIZE], int height, int width) 
{
    for (int i = 0; i < height; i++) 
    {
        memset(board[i], '0', width);
    }
}
int begin_method(int client_fd, BattleShip *ship) 
{
    char buffer[BUFFER_SIZE] = {0};
    int nbytes = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (nbytes <= 0)
    {
        return -1;
    }
    buffer[nbytes] = '\0';
    buffer[strcspn(buffer, "\r\n")] = '\0';
    if (buffer[0] == 'F')
    {
        char *ff = "H 0";
        char *gg = "H 1";
        send(client_fd, ff, strlen(ff), 0);
        if (ship->turn == 1)
        {
            send(ship->play_conn2, gg, strlen(gg), 0);
        }
        else
        {
            send(ship->play_conn, gg, strlen(gg), 0);
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

        char left[BUFFER_SIZE];
        if (sscanf(ptr, "%d %d%s", &width, &height, left) == 2)
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

int place_ships_on_board(char (*board)[MAX_BOARD_SIZE], int *parameters, BattleShip *ship) 
{
    int prio = 0;

    for (int piece = 0; piece < MAX_SHIPS; piece++) 
    {
        char count = '1' + piece;
        int pos = parameters[piece * 4];
        int moved = parameters[piece * 4 + 1];
        int y_axis = parameters[piece * 4 + 2];
        int x_axis = parameters[piece * 4 + 3];

        if (pos < 1 || pos > 7) 
        {
            if (!prio || 300 < prio) 
            {
                prio = 300;
            }
            continue;
        }
        if (moved < 1 || moved > 4) 
        {
            if (!prio || 301 < prio) 
            {
                prio = 301;
            }
            continue;
        }
        moved--;

        int origin1 = -1, origin2 = -1;
        for (int x = 0; x < 4 && origin2 == -1; x++) 
        {
            for (int y = 0; y < 4 && origin1 == -1; y++) 
            {
                if (AllShapes[pos - 1][moved][y][x] == '2') 
                {
                    origin1 = y;
                    origin2 = x;
                    break;
                }
            }
        }

        int placed = 1;
        if (x_axis < 0 || x_axis >= ship->height || y_axis < 0 || y_axis >= ship->width) 
        {
            if (!prio || 302 < prio) 
            {
                prio = 302;
            }
            placed = 0;
        } 
        else if (board[x_axis][y_axis] != '0') 
        {
            if (!prio || 303 < prio) 
            {
                prio = 303;
            }
            placed = 0;
        }
        for (int x = 0; x < 4 && placed; x++) 
        {
            for (int y = 0; y < 4 && placed; y++) 
            {
                if (AllShapes[pos - 1][moved][y][x] == '1') 
                {
                    int player1 = x_axis + (y - origin1);
                    int player2 = y_axis + (x - origin2);
                    if (player1 < 0 || player1 >= ship->height || player2 < 0 || player2 >= ship->width) 
                    {
                        if (!prio || 302 < prio) prio = 302;
                        placed = 0;
                        break;
                    }

                    if (board[player1][player2] != '0') 
                    {
                        if (!prio || 303 < prio) prio = 303;
                        placed = 0;
                        break;
                    }
                }
            }
        }

        if (placed != 0) 
        {
            board[x_axis][y_axis] = count;

            for (int x = 0; x < 4; x++) 
            {
                for (int y = 0; y < 4; y++) 
                {
                    if (AllShapes[pos - 1][moved][y][x] == '1') 
                    {
                        int player1 = x_axis + (y - origin1);
                        int player2 = y_axis + (x - origin2);
                        board[player1][player2] = count;
                    }
                }
            }
        }
    }
    return prio;
}

int initialize_packet(int connection, BattleShip *ship) 
{
    char buffer[BUFFER_SIZE] = {0};
    int parameters[20] = {0};
    const char *correct = "A";
    int validation_result = read_and_validate_packet(connection, buffer, ship);

    if (validation_result <= 0) 
    {
        return validation_result;
    }

    int parse_result = parse_packet_parameters(buffer, parameters);
    if (parse_result != 0) 
    {
        char error_msg[10];
        snprintf(error_msg, sizeof(error_msg), "E %d", parse_result);
        send(connection, error_msg, strlen(error_msg) + 1, 0);
        return 0;
    }

    char (*board)[MAX_BOARD_SIZE];
    if (ship->turn == 1)
    {
        board = ship->player1_board;
    }
    else
    {
        board = ship->player2_board;
    }

    initialize_board(board, ship->height, ship->width);
    int prio = place_ships_on_board(board, parameters, ship);

    if (prio != 0) 
    {
        char error_msg[10];
        snprintf(error_msg, sizeof(error_msg), "E %d", prio);
        send(connection, error_msg, strlen(error_msg) + 1, 0);
        return 0;
    }

    send(connection, correct, strlen(correct) + 1, 0);
    return 1;
}

int validate_shoot_target(int x_axis, int y_axis, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int width, int height, int connection)
{
    if (x_axis < 0 || x_axis >= height || y_axis < 0 || y_axis >= width)
    {
        const char *error = "E 400";
        send(connection, error, strlen(error) + 1, 0);
        return 0;
    }

    if (board[x_axis][y_axis] == 'M' || board[x_axis][y_axis] == 'H')
    {
        const char *error = "E 401";
        send(connection, error, strlen(error) + 1, 0);
        return 0;
    }

    return 1;
}

int ships_left(BattleShip *ship)
{
    int left = 0;
    char (*board)[MAX_BOARD_SIZE];
if (ship->turn == 1)
{
    board = ship->player2_board;
}
else
{
    board = ship->player1_board;
}

    for (int num = 1; num <= MAX_SHIPS; num++)
    {
        char hold_char = num + '0';
        int still_there = 0;

        for (int i = 0; i < ship->height && !still_there; i++)
        {
            for (int j = 0; j < ship->width && !still_there; j++)
            {
                if (board[i][j] == hold_char)
                {
                    still_there = 1;
                    left++;
                }
            }
        }
    }
    return left;
}

void process_shoot_result(BattleShip *ship, int x_axis, int y_axis, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int connection)
{
    char msg[20];
    if (board[x_axis][y_axis] == '0') 
    {
        board[x_axis][y_axis] = 'M';
        int left = ships_left(ship);
        snprintf(msg, sizeof(msg), "R %d M", left);
        send(connection, msg, strlen(msg) + 1, 0);
    } 
    else 
    {
        board[x_axis][y_axis] = 'H';
        int left = ships_left(ship);
        snprintf(msg, sizeof(msg), "R %d H", left);
        send(connection, msg, strlen(msg) + 1, 0);

        if (left == 0) 
        {
            ship->game_ended = 1;
        }
    }
}

int shoot_method(int connection, char *buffer, BattleShip *ship)
{
    int x_axis, y_axis;
    char extra[BUFFER_SIZE];
    int reads = sscanf(buffer, "S %d %d %s", &x_axis, &y_axis, extra);

    if (reads != 2)
    {
        const char *error = "E 202"; 
        send(connection, error, strlen(error) + 1, 0);
        return 0;
    }

    char (*opponent_board)[MAX_BOARD_SIZE];
    if (ship->turn == 1)
    {
        opponent_board = ship->player2_board;
    }
    else
    {
        opponent_board = ship->player1_board;
    }

    if (!validate_shoot_target(x_axis, y_axis, opponent_board, ship->width, ship->height, connection))
    {
        return 0; 
    }

    char result_msg[BUFFER_SIZE];
    if (opponent_board[x_axis][y_axis] == '0')
    {
        opponent_board[x_axis][y_axis] = 'M'; 
        int left = ships_left(ship);
        snprintf(result_msg, sizeof(result_msg), "R %d M", left);
        send(connection, result_msg, strlen(result_msg) + 1, 0);
    }
    else
    {
        opponent_board[x_axis][y_axis] = 'H'; 
        int left = ships_left(ship);
        snprintf(result_msg, sizeof(result_msg), "R %d H", left);
        send(connection, result_msg, strlen(result_msg) + 1, 0);

        if (left == 0) 
{
    char loser_buffer[BUFFER_SIZE] = {0};
    int loser_connection;
    if (ship->turn == 1)
    {
        loser_connection = ship->play_conn2;
    }
    else
    {
        loser_connection = ship->play_conn;
    }
    read(loser_connection, loser_buffer, BUFFER_SIZE - 1);
    const char *loser_msg = "H 0";
    send(loser_connection, loser_msg, strlen(loser_msg) + 1, 0);
    char winner_buffer[BUFFER_SIZE] = {0};
    int winner_connection = connection;
    read(winner_connection, winner_buffer, BUFFER_SIZE - 1);
    const char *gg = "H 1";
    send(winner_connection, gg, strlen(gg) + 1, 0);

    if (ship->turn == 1)
    {
        printf("[Server] Player 1 wins. All ships sunk for Player 2.\n");
    }
    else
    {
        printf("[Server] Player 2 wins. All ships sunk for Player 1.\n");
    }

    ship->game_ended = 1;

    return -1; 
}

    }

    if (ship->turn == 1)
    {
        printf("[Server] Player 1 wins. All ships sunk for Player 2.\n");
    }
    else
    {
        printf("[Server] Player 2 wins. All ships sunk for Player 1.\n");
    }   
    if (ship->turn == 1)
    {
        ship->turn = 2;
    }
        else
    {
        ship->turn = 1;
    }

    return 1;
}




int count_left(BattleShip *ship) 
{
    int left = 0;
   char (*opponent_board)[MAX_BOARD_SIZE];
if (ship->turn == 1)
{
    opponent_board = ship->player1_board;
}
else
{
    opponent_board = ship->player2_board;
}


    for (int num = 1; num <= 5; num++) 
    {
        char yes_ship = num + '0';
        int hits = 0;

        for (int x_axis = 0; x_axis < ship->height && !hits; x_axis++) 
        {
            for (int y_axis = 0; y_axis < ship->width && !hits; y_axis++) 
            {
                if (opponent_board[x_axis][y_axis] == yes_ship) 
                {
                    hits = 1;
                    left++;
                }
            }
        }
    }
    return left;
}

void query(BattleShip *ship)
{
    char q_res[3 + (7 * ship->width * ship->height) + 1];
    char *ptr = q_res;
    char (*board)[MAX_BOARD_SIZE];
    int play_conn;
    int left = ships_left(ship);

const char values[] = {'G', ' ', left + '0', ' '};
for (int i = 0; i < 4; i++)
{
    *ptr++ = values[i];
}

    if (ship->turn == 1)
    {
        board = ship->player2_board;
        play_conn = ship->play_conn;
    }
    else
    {
        board = ship->player1_board;
        play_conn = ship->play_conn2;
    }

    for (int x_axis = 0; x_axis < ship->height; x_axis++)
    {
        for (int y_axis = 0; y_axis < ship->width; y_axis++)
        {
            if (board[x_axis][y_axis] == 'H')
            {
                *ptr++ = 'H';
                *ptr++ = ' ';
                *ptr++ = x_axis + '0';
                *ptr++ = ' ';
                *ptr++ = y_axis + '0';
                *ptr++ = ' ';
            }
            else if (board[x_axis][y_axis] == 'M')
            {
                *ptr++ = 'M';
                *ptr++ = ' ';
                *ptr++ = x_axis + '0';
                *ptr++ = ' ';
                *ptr++ = y_axis + '0';
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
void cleanup_game_resources(BattleShip *set_ship) 
{
    memset(set_ship->player1_board, 0, sizeof(set_ship->player1_board));
    memset(set_ship->player2_board, 0, sizeof(set_ship->player2_board));

    set_ship->width = 0;
    set_ship->height = 0;
    set_ship->turn = 0;
    set_ship->game_ended = 0;
    set_ship->game_started = 0;

    if (set_ship->play_conn > 0) 
    {
        close(set_ship->play_conn);
        set_ship->play_conn = -1;
    }
    if (set_ship->play_conn2 > 0) 
    {
        close(set_ship->play_conn2);
        set_ship->play_conn2 = -1;
    }
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
    set_ship.game_ended = 0;

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
        int p1_result = begin_method(client1_fd, &set_ship);
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
        int p2_result = begin_method(client2_fd, &set_ship);
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
    cleanup_game_resources(&set_ship);
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
        int p1_result = initialize_packet(client1_fd, &set_ship);
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
        int p2_result = initialize_packet(client2_fd, &set_ship);
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
    cleanup_game_resources(&set_ship);
    close(client1_fd);
    close(client2_fd);
    close(server_fd1);
    close(server_fd2);
    return -1; 
}
while (!set_ship.game_ended) 
{
    int connection;
    if (set_ship.turn == 1)
    {
        connection = set_ship.play_conn;
    }
    else
    {
        connection = set_ship.play_conn2;
    }

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
            char *ff = "H 0";
            char *gg = "H 1";
            int other_connection;
    if (set_ship.turn == 1)
    {
        other_connection = set_ship.play_conn2;
    }
    else
    {
        other_connection = set_ship.play_conn;
    }

            send(connection, ff, strlen(ff) + 1, 0);
            send(other_connection, gg, strlen(gg) + 1, 0);

            if (set_ship.turn == 1)
    {
        printf("[Server] Player 1 forfeited. Player 2 wins.\n");
    }
    else
    {
        printf("[Server] Player 2 forfeited. Player 1 wins.\n");
    }
            set_ship.game_ended = 1;
            break;
        }
        else if (command[0] == 'S') 
        {
            int shoot_result = shoot_method(connection, command, &set_ship);

            if (shoot_result < 0) 
            {
                set_ship.game_ended = 1;
                break;
            }
        }
        else if (command[0] == 'Q') 
        {
            query(&set_ship);
        }
        else 
        {
            send(connection, "E 102", strlen("E 102") + 1, 0);
            printf("[Server] Invalid command from player %d: %s\n", set_ship.turn, command);
        }

        command = strtok(NULL, "\n");
    }
}

cleanup_game_resources(&set_ship);
close(client1_fd);
close(client2_fd);
close(server_fd1);
close(server_fd2);
return 0;
}