#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

typedef struct DIRECTION {
  int x;
  int y;
} DIRECTION;

typedef struct SNAKE_SEGMENT {
  int x;
  int y;
} SNAKE_SEGMENT;

typedef struct TROPHY {
  int x;
  int y;
  int time;
  int value;
  bool consumed;
} TROPHY;

struct SNAKE_SEGMENT snakehead;

struct SNAKE_SEGMENT snakebody[1000];

struct TROPHY trophy;

int snakesize = 0;

int arena_x = 0, arena_y = 0;

int speed = 125000;

bool gameover = false;

bool victory = false;

bool gameloss();

bool gamewin();

bool hitarena();

bool hitsnake();

void drawarena(int x, int y);

void createsnake(int x, int y, int start);

void movesnake(int x, int y);

void growsnake(int x, int y);

void setcanonmode(int m);

void oops(char s[], int x);

void createtrophy();

int main() {
  int direction;
  int input;
  char buf[100];
  int startdirection;
  bool startgame = false;
  DIRECTION currentdirection = {0, 0};

  srand(time(NULL));

  setcanonmode(0);

  initscr();
  start_color();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_BLACK, COLOR_YELLOW);
  init_pair(3, COLOR_RED, COLOR_BLUE);
  init_pair(4, COLOR_RED, COLOR_GREEN);
  noecho();
  raw();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  timeout(0);

  getmaxyx(stdscr, arena_y, arena_x);

  clear();

  attron(COLOR_PAIR(2));
  drawarena(arena_x, arena_y);
  attroff(COLOR_PAIR(2));

  attron(COLOR_PAIR(4));
  move((arena_y / 2), arena_x / 2 - 7);
  addstr("Welcome to Snake!");
  move((arena_y / 2) + 2, (arena_x / 2) - 9);
  addstr("Press enter to play.");
  move((arena_y / 2) + 4, (arena_x / 2) - 14);
  addstr("Press q/Q at any time to quit");
  attroff(COLOR_PAIR(4));

  refresh();

  while (startgame == false) {
    cbreak();
    input = getch();
    flushinp();
    if (input == 'q' || input == 'Q') {
      endwin();
      setcanonmode(1);
      exit(1);
    }
    switch (input) {
    case 10:
    case KEY_ENTER:
      startgame = true;
      break;
    case 'q':
    case 'Q':
      endwin();
      setcanonmode(1);
      exit(1);
    default:
      break;
    }
  }

  clear();

  attron(COLOR_PAIR(2));
  drawarena(arena_x, arena_y);
  attroff(COLOR_PAIR(2));

  startdirection = rand() % 4;

  switch (startdirection) {
  case 0:
    currentdirection.x = -1;
    currentdirection.y = 0;
    break;
  case 1:
    currentdirection.x = 1;
    currentdirection.y = 0;
    break;
  case 2:
    currentdirection.x = 0;
    currentdirection.y = -1;
    break;
  case 3:
    currentdirection.x = 0;
    currentdirection.y = 1;
    break;
  }

  attron(COLOR_PAIR(1));
  createsnake(arena_x, arena_y, startdirection);
  attroff(COLOR_PAIR(1));

  trophy.x = -1;
  trophy.y = -1;
  trophy.consumed = false;
  createtrophy();

  while (gameover == false && victory == false) {

    direction = getch();
    flushinp();

    switch (direction) {
    case KEY_LEFT:
      currentdirection.x = -1;
      currentdirection.y = 0;
      break;
    case KEY_RIGHT:
      currentdirection.x = 1;
      currentdirection.y = 0;
      break;
    case KEY_UP:
      currentdirection.x = 0;
      currentdirection.y = -1;
      break;
    case KEY_DOWN:
      currentdirection.x = 0;
      currentdirection.y = 1;
      break;
    case 'q':
    case 'Q':
      endwin();
      setcanonmode(1);
      exit(1);
    default:
      break;
    }

    attron(COLOR_PAIR(1));
    movesnake(currentdirection.x, currentdirection.y);
    attroff(COLOR_PAIR(2));

    trophy.time = trophy.time - speed;

    if (snakehead.x == trophy.x && snakehead.y == trophy.y) {
      trophy.consumed = true;
    }

    if (trophy.time <= 0 || trophy.consumed == true) {
      if (trophy.consumed == true) {
        snakesize += trophy.value;
        attron(COLOR_PAIR(1));
        growsnake(currentdirection.x, currentdirection.y);
        attroff(COLOR_PAIR(1));
      }
      createtrophy();
    }

    gameover = gameloss();

    victory = gamewin();

    usleep(speed);
  }

  if (victory == true) {
    clear();

    attron(COLOR_PAIR(2));
    drawarena(arena_x, arena_y);
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(4));
    move((arena_y / 2), arena_x / 2 - 4);
    addstr("Victory!");
    move((arena_y / 2) + 2, (arena_x / 2) - 11);
    addstr("Press any key to exit");
    move((arena_y / 2) + 4, (arena_x / 2) - 17);
    sprintf(buf, "Your snake was %d characters long.", snakesize);
    addstr(buf);
    attroff(COLOR_PAIR(4));

    refresh();
    cbreak();
    getchar();
  } else if (gameover == true) {
    clear();

    attron(COLOR_PAIR(2));
    drawarena(arena_x, arena_y);
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(4));
    move((arena_y / 2), arena_x / 2 - 6);
    addstr("Game Over!!!");
    move((arena_y / 2) + 2, (arena_x / 2) - 11);
    addstr("Press any key to exit");
    move((arena_y / 2) + 4, (arena_x / 2) - 17);
    sprintf(buf, "Your snake was %d characters long.", snakesize);
    addstr(buf);
    move((arena_y / 2) + 6, (arena_x / 2) - 25);
    sprintf(buf, "Your snake needs to be %d characters long to win.",
            arena_x + arena_y);
    addstr(buf);
    attroff(COLOR_PAIR(4));

    refresh();
    cbreak();
    getchar();
  }

  refresh();
  endwin();
  setcanonmode(1);
}

void drawarena(int x, int y) {

  for (int i = 0; i < x; i++) {
    move(0, i);
    addstr("*");
  }

  for (int i = 0; i < x; i++) {
    move(y - 1, i);
    addstr("*");
  }

  for (int i = 0; i < y; i++) {
    move(i, 0);
    addstr("*");
  }

  for (int i = 0; i < y; i++) {
    move(i, x - 1);
    addstr("*");
  }

  refresh();
}

bool gameloss() {
  bool gameloss = false;
  if (hitarena() == true || hitsnake() == true) {
    gameloss = true;
  }
  return gameloss;
}

bool gamewin() {
  bool gamewin = false;
  if (snakesize >= (arena_x + arena_y)) {
    gamewin = true;
  }
  return gamewin;
}

bool hitarena() {
  bool hitarena = false;
  if (snakehead.x <= 0 || snakehead.x >= (arena_x - 1)) {
    hitarena = true;
  }
  if (snakehead.y <= 0 || snakehead.y >= (arena_y - 1)) {
    hitarena = true;
  }
  return hitarena;
}

bool hitsnake() {
  bool hitsnake = false;
  for (int i = 0; i < snakesize - 1; i++) {
    for (int j = i + 1; j < snakesize - 1; j++) {
      if (snakehead.x == snakebody[i].x && snakehead.y == snakebody[i].y) {
        hitsnake = true;
      }
    }
  }
  return hitsnake;
}

void createsnake(int x, int y, int start) {
  for (int i = 0; i < 5; i++) {
    switch (start) {
    case 0:
      snakebody[i].x = (x / 2) - i;
      snakebody[i].y = y / 2;
      break;
    case 1:
      snakebody[i].x = (x / 2) + i;
      snakebody[i].y = y / 2;
      break;
    case 2:
      snakebody[i].x = (x / 2);
      snakebody[i].y = y / 2 - i;
      break;
    case 3:
      snakebody[i].x = (x / 2);
      snakebody[i].y = y / 2 + i;
      break;
    }
    snakesize++;
  }
  snakehead = snakebody[4];

  for (int i = 0; i < 5; i++) {
    move(snakebody[i].y, snakebody[i].x);
    addstr("@");
  }
  refresh();
  sleep(1);
}

void movesnake(int x, int y) {
  move(snakebody[0].y, snakebody[0].x);
  addstr(" ");
  for (int i = 0; i < snakesize - 1; i++) {
    snakebody[i] = snakebody[i + 1];
  }
  snakebody[snakesize - 1].x = snakebody[snakesize - 1].x + x;
  snakebody[snakesize - 1].y = snakebody[snakesize - 1].y + y;
  snakehead = snakebody[snakesize - 1];

  move(snakebody[snakesize - 1].y, snakebody[snakesize - 1].x);

  addstr("@");
  refresh();
}

void growsnake(int x, int y) {
  for (int i = snakesize - (trophy.value); i < snakesize; i++) {
    snakebody[i].x = snakebody[i - 1].x + x;
    snakebody[i].y = snakebody[i - 1].y + y;
    move(snakebody[i].y, snakebody[i].x);
    addstr("@");
  }
  snakehead = snakebody[snakesize - 1];
  speed -= 2250 * trophy.value;
  refresh();
}

void createtrophy() {
  char buf[1];
  if (trophy.consumed == false) {
    move(trophy.y, trophy.x);
    addstr(" ");
  }
  bool checksnake = false;
  trophy.consumed = false;

  int time = (rand() % 9) + 1;
  trophy.time = time * 1000000;
  trophy.value = (rand() % 9) + 1;
  trophy.x = (rand() % (arena_x - 2)) + 1;
  trophy.y = (rand() % (arena_y - 2)) + 1;
  while (checksnake == false) {
    for (int i = 0; i < snakesize; i++) {
      if (trophy.x == snakebody[i].x && trophy.y == snakebody[i].y) {
        trophy.x = (rand() % (arena_x - 2)) + 1;
        trophy.y = (rand() % (arena_y - 2)) + 1;
        i = snakesize;
      } else {
        checksnake = true;
      }
    }
    move(trophy.y, trophy.x);
    sprintf(buf, "%d", trophy.value);

    attron(COLOR_PAIR(3));
    addstr(buf);
    attroff(COLOR_PAIR(3));
  }
}

void setcanonmode(int m) {
  struct termios info;
  if (tcgetattr(0, &info) == -1) {
    oops("tcgettattr", 1);
  }

  if (m == 1) {
    info.c_lflag |= ICANON;
  }

  else {
    info.c_lflag &= ~ICANON;
  }

  if (tcsetattr(0, TCSANOW, &info) == -1) {
    oops("tcsetattr", 2);
  }
}

void oops(char s[], int x) {
  perror(s);
  exit(x);
}
