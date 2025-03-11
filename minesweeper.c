#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define FLAG 9
#define MINE -1
#define SAFE 0

#define WIDTH 10
#define HEIGHT 10

#define NUM_MINES 10


int is_mine(int** board, int x, int y) {
    if (x < 0) {return 0;}
    if (y < 0) {return 0;}
    if (x >= WIDTH) {return 0;}
    if (y >= HEIGHT) {return 0;}
    return board[y][x] == MINE;
}

void print_board(int** board, int** covered) {
    printf("\033[2J");  // ANSI escape sequence. Clear Screen and return to 0,0
    printf("               COLUMNS\n");
    printf("       |"); for(int x=0; x<WIDTH;x++) {printf(" %d", x);} printf(" |\n");
    printf("     --|"); for(int x=0; x<WIDTH;x++) {printf("--");} printf("-|\n");
    for (int y=0; y<HEIGHT; y++) {
        if (y+1 == HEIGHT / 2) {
            printf("ROWS %d | ", y);
        }
        else {
            printf("     %d | ", y);
        }
        for (int x=0; x<WIDTH;x++) {
            if (covered[y][x] == FLAG) {printf("F");}  // square flagged!
            else if (covered[y][x]) {printf("#");}  // cover the square
            else if (is_mine(board, x ,y)) {printf("M");}  // if this occurs it is either debug, loss, or end-of-game
            else {printf("%d", board[y][x]);}  // print out the number of adjacent squares
            printf(" ");
        }
        printf("|\n");
    }
    printf("     --|"); for(int x=0; x<WIDTH;x++) {printf("--");} printf("-|\n");
}

void flag(int** covered, int x, int y){
    if (!covered[y][x]) {return;}  // Swept an uncovered place
    else if (covered[y][x] == FLAG) {covered[y][x] = 1;}  // Unflag
    else {covered[y][x] = FLAG;}
}

int sweep(int** board, int** covered, int x, int y){
    if (!covered[y][x]) {return SAFE;}  // Swept an uncovered place
    if (covered[y][x] == FLAG) {covered[y][x] = 1;}  // Unflag it, but do not uncover it
    covered[y][x] = 0;

    if (board[y][x] == SAFE) {
        int node,nx,ny,dx,dy,sx,sy;
        int stack_height = 0;
        int* stack = (int*) calloc(100, sizeof(int));
        int* marked = (int*) calloc(100, sizeof(int));

        for (sx=0;sx<WIDTH;sx++){
            for(sy=0;sy<HEIGHT;sy++){
                if (covered[sy][sx] == 0) {marked[sx+WIDTH*sy] = 1;}
            }
        }

        stack[stack_height++] = x + (y*WIDTH); marked[x + (y*WIDTH)] = 1;

        while (stack_height > 0) {
            // take node off stack
            node = stack[--stack_height];
            nx = node % WIDTH; ny = node / WIDTH;
            // uncover! (do action)
            covered[ny][nx] = 0;
            // get adjacent
            for (dx=-1;dx <=1;dx++){
                for (dy=-1;dy <=1;dy++){
                    sy = ny+dy; sx = nx+dx;
                    if (sy >= 0 && sx >= 0 && sy < HEIGHT && sx < WIDTH){
                        // check blank
                        if (board[sy][sx] == SAFE) {
                            // check unsearched
                            if (marked[(WIDTH*sy) + sx] == 0) {
                                stack[stack_height++] = sx + (sy*WIDTH); 
                                marked[sx + (sy*WIDTH)] = 1;
                            }
                        }
                        else if (board[sy][sx] != MINE) {
                            marked[sx + (sy*WIDTH)] = 1;
                            covered[sy][sx] = 0;
                        }
                    }
                }
            }
        }
        free(stack);
        free(marked);

    }
    return board[y][x];
}

int check_win(int** covered) {
    int x, y, count;
    for (x=0;x<WIDTH;x++){
        for(y=0;y<HEIGHT;y++){
            if (covered[y][x]) {
                count++;
                if (count > NUM_MINES) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int main(void) {
    int** board;
    int** covered;

    int x, y, i, dx, dy;

    srand(time(NULL));   // Initialization, should only be called once.

    // Initialize the board
    board = (int**) calloc(HEIGHT, sizeof(int*));
    for (i=0; i< HEIGHT; i++) {
        board[i] = (int*) calloc(WIDTH, sizeof(int));
    }
    // Initialise Cover Map
    covered = (int**) calloc(HEIGHT, sizeof(int*));
    for (i=0; i< HEIGHT; i++) {
        covered[i] = (int*) calloc(WIDTH, sizeof(int));
    }
    for (int y=0; y<HEIGHT; y++) {
        for (int x=0; x<WIDTH;x++) {
            covered[y][x]=1;
        }
    }

    // Put the Mines down
    for (i=0; i<NUM_MINES; i++) {
        x = rand() % WIDTH;
        y = rand() % HEIGHT;
        while (board[y][x] == MINE) {
            x = rand() % WIDTH;
            y = rand() % HEIGHT;
        }
        board[y][x] = MINE;
    }

    // Assign the Mine-Adjacency Numbers
    int mine_count;
    for (x=0;x<WIDTH;x++){
        for (y=0;y<HEIGHT;y++){
            if (is_mine(board, x, y)) {}
            else {
                mine_count = 0;
                for (dy=-1; dy<=1;dy++) {
                    for (dx=-1; dx<=1;dx++) {
                        if (is_mine(board, x+dx, y+dy)) {
                            mine_count++;
                        }
                    }
                }
                board[y][x] = mine_count;
            }
        }
    }

    // Play Loop
    int in_play = 1, lose = 0, win = 0, input_valid = 0, hit;

    char action;
    int row, col;

    while (in_play) {
        print_board(board, covered);

        printf("Available Moves: \n - SWEEP square on column X row Y 'S X Y'\n - FLAG square on column X row Y 'F X Y'\n");
        printf("e.g. S23 to 'SWEEP' row X column Y\n");

        while (!input_valid) {
            // SCANF / User Input - https://www.w3schools.com/c/c_user_input.php
            printf("INPUT: "); scanf("%c%d%d", &action, &col, &row);

            if (action !=  'F' && action != 'S') {
                // invalid input
                printf("Invalid - Action must lead with an 'S' or an 'F'\n");
            }
            else if (row > HEIGHT || row < 0 || col > WIDTH || col < 0) {
                // invalid input
                printf("Invalid - Indexes must be within range! (written)\n");
            }
            else {
                input_valid = 1;
            }
        }
        input_valid = 0;

        if (action == 'F') {
            flag(covered, col, row);
        }
        if (action == 'S') {
            hit = sweep(board, covered, col, row);
        }

        lose = (hit == MINE);

        win = check_win(covered);

        in_play = !lose && !win;
    }

    // End of game

    // Display Uncovered game board
    for (x=0;x<WIDTH;x++){
        for(y=0;y<HEIGHT;y++){
            covered[y][x] = 0;
        }
    }    
    print_board(board, covered);

    if (lose) {
        printf("sorry you hit a mine :/\n");
    }
    else if (win) {
        printf("Congratulations! Successfully swept mines\n");
    }

    // Cross my T's
    for (i=0; i< HEIGHT; i++) {
        free(board[i]);
        free(covered[i]);
    }
    free(board);
    free(covered);
}