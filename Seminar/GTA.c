/*
    Mini GTA-like Spiel (portable Version)

    Steuerung:
    W = hoch
    S = runter
    A = links
    D = rechts
    Q = beenden
*/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define WIDTH 20
#define HEIGHT 10

int playerX = 1;
int playerY = 1;
int gameRunning = 1;

// Map
int map[HEIGHT][WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1},
    {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1},
    {1,0,1,0,0,1,0,1,1,0,0,0,1,0,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,1,1,0,1,0,0,0,1,1,0,1,0,0,0,0,1},
    {1,0,1,0,0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Terminal Setup für direkte Eingabe
char getch() {
    struct termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO); // kein Enter nötig
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// Bildschirm löschen
void clearScreen() {
    system("clear");
}

// Zeichnen
void draw() {
    clearScreen();
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            if (x == playerX && y == playerY)
                printf("A");
            else if (map[y][x] == 1)
                printf("#");
            else
                printf(" ");

        }
        printf("\n");
    }
}

// Input
void input() {
    char key = getch();

    int newX = playerX;
    int newY = playerY;

    switch (key) {
        case 'w': newY--; break;
        case 's': newY++; break;
        case 'a': newX--; break;
        case 'd': newX++; break;
        case 'q': gameRunning = 0; break;
    }

    if (map[newY][newX] == 0) {
        playerX = newX;
        playerY = newY;
    } else {
        printf("\n💥 Crash! Game Over!\n");
        gameRunning = 0;
    }
}

// Loop
void gameLoop() {
    while (gameRunning) {
        draw();
        input();
    }
}

int main() {
    gameLoop();
    return 0;
}