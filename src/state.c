#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"
#include "assert_msg.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_body(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  game_state_t *result = malloc(sizeof(game_state_t));
  if (!result) {
    return NULL;
  }

  size_t COLS = 20;
  result->num_rows = 18;
  result->num_snakes = 1;

  result->snakes = malloc(sizeof(snake_t));
  if (!result->snakes) {
    free(result);
    return NULL;
  }

  result->board = malloc(result->num_rows * sizeof(char *));
  if (!result->board) {
    free(result->snakes);
    free(result);
    return NULL;
  }

  const char *default_board[18] = {
      "####################\n", "#                  #\n",
      "# d>D    *         #\n", "#                  #\n",
      "#                  #\n", "#                  #\n",
      "#                  #\n", "#                  #\n",
      "#                  #\n", "#                  #\n",
      "#                  #\n", "#                  #\n",
      "#                  #\n", "#                  #\n",
      "#                  #\n", "#                  #\n",
      "#                  #\n", "####################\n"};

  char **board = result->board;
  for (int i = 0; i < result->num_rows; i++) {
    board[i] = malloc((COLS + 2) * sizeof(char));
    if (!board[i]) {
      for (int j = 0; j < i; j++)
        free(board[j]);
      free(result->snakes);
      free(result->board);
      free(result);
    }
    strcpy(board[i], default_board[i]);
  }

  board[2][9] = '*';

  result->snakes[0].head_col = 4;
  result->snakes[0].head_row = 2;
  result->snakes[0].tail_col = 2;
  result->snakes[0].tail_row = 2;
  result->snakes[0].live = true;

  board[2][2] = 'd';
  board[2][3] = '>';
  board[2][4] = 'D';

  return result;
}

/* Task 2 */
void free_state(game_state_t *state) {
  for (int i = 0; i < state->num_rows; i++) {
    free(state->board[i]);
  }
  free(state->board);
  free(state->snakes);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  if (!state) {
    return;
  }

  if (!state->board) {
    return;
  }

  for (int i = 0; i < state->num_rows; i++) {
    if (!state->board[i]) {
      break;
    }
    fprintf(fp, "%s", state->board[i]);
  }

  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row,
                         unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
    return true;
  }
  return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  switch (c) {
  case 'W':
  case 'A':
  case 'S':
  case 'D':
  case 'x':
    return true;
  default:
    return false;
  }
}

static bool is_body(char c) {
  switch (c) {
  case '^':
  case '<':
  case '>':
  case 'v':
    return true;
  default:
    return false;
  }
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  if (is_head(c) || is_tail(c) || is_body(c)) {
    return true;
  }

  return false;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  switch (c) {
  case '^':
    return 'w';
  case '<':
    return 'a';
  case 'v':
    return 's';
  case '>':
    return 'd';
  default:
    return '\0';
  }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  switch (c) {
  case 'W':
    return '^';
  case 'A':
    return '<';
  case 'S':
    return 'v';
  case 'D':
    return '>';
  default:
    return '\0';
  }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  switch (c) {
  case 'v':
  case 's':
  case 'S':
    return cur_row + 1;
  case '^':
  case 'w':
  case 'W':
    return cur_row - 1;
  case '<':
  case 'a':
  case 'A':
  case '>':
  case 'd':
  case 'D':
    return cur_row;
  default:
    assert_msg(0, "Unexpected character in get_next_row(): '%c'", c);
  }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  switch (c) {
  case '>':
  case 'd':
  case 'D':
    return cur_col + 1;
  case '<':
  case 'a':
  case 'A':
    return cur_col - 1;
  case 'v':
  case 's':
  case 'S':
  case '^':
  case 'w':
  case 'W':
    return cur_col;
  default:
    assert_msg(0, "Unexpected character in get_next_col(): '%c'", c);
  }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake
  is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  if (state == NULL || state->snakes == NULL || state->board == NULL) {
    return '\0';
  }

  snake_t *snakes = state->snakes;

  if (snum >= state->num_snakes) {
    return '\0';
  }

  snake_t snake = snakes[snum];

  if (snake.head_row >= state->num_rows) {
    return '\0';
  }

  char snake_head_char = state->board[snake.head_row][snake.head_col];

  unsigned int next_col = get_next_col(snake.head_col, snake_head_char);
  unsigned int next_row = get_next_row(snake.head_row, snake_head_char);

  if (next_row >= state->num_rows) {
    return '\0';
  }

  return state->board[next_row][next_col];
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the
  head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  if (state == NULL || state->snakes == NULL || state->board == NULL) {
    return;
  }

  if (snum >= state->num_snakes) {
    return;
  }

  snake_t *snake = &state->snakes[snum];

  unsigned int curr_head_col = snake->head_col;
  unsigned int curr_head_row = snake->head_row;
  char snake_head_char = state->board[curr_head_row][curr_head_col];

  unsigned int next_col = get_next_col(snake->head_col, snake_head_char);
  unsigned int next_row = get_next_row(snake->head_row, snake_head_char);

  if (next_row >= state->num_rows) {
    return;
  }

  state->board[next_row][next_col] = snake_head_char;

  char body_char = head_to_body(snake_head_char);
  if (body_char != '\0') {
    state->board[curr_head_row][curr_head_col] = body_char;
  }

  snake->head_row = next_row;
  snake->head_col = next_col;

  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  if (state == NULL || state->snakes == NULL || state->board == NULL) {
    return;
  }

  if (snum >= state->num_snakes) {
    return;
  }

  snake_t *snake = &state->snakes[snum];

  unsigned int curr_tail_row = snake->tail_row;
  unsigned int curr_tail_col = snake->tail_col;

  char tail_char = state->board[curr_tail_row][curr_tail_col];

  state->board[snake->tail_row][snake->tail_col] = ' ';

  unsigned int new_tail_row = get_next_row(curr_tail_row, tail_char);
  unsigned int new_tail_col = get_next_col(curr_tail_col, tail_char);

  if (new_tail_row >= state->num_rows) {
    return;
  }

  char body_char = state->board[new_tail_row][new_tail_col];
  char new_tail_char = body_to_tail(body_char);
  if (new_tail_char != '\0') {
    state->board[new_tail_row][new_tail_col] = new_tail_char;
  }

  snake->tail_row = new_tail_row;
  snake->tail_col = new_tail_col;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  if (state == NULL || state->snakes == NULL || state->board == NULL) {
    return;
  }

  for (unsigned int i = 0; i < state->num_snakes; i++) {
    snake_t *snake = &state->snakes[i];
    if (snake == NULL || !snake->live) {
      continue;
    }

    char next_square_char = next_square(state, i);
    if (next_square_char == '\0' || next_square_char == '\n' ||
        next_square_char == '#' || is_snake(next_square_char)) {
      snake->live = false;

      if (snake->head_row < state->num_rows) {
        state->board[snake->head_row][snake->head_col] = 'x';
      }

    } else if (next_square_char == '*') {
      update_head(state, i);
      add_food(state);
    } else if (next_square_char == ' ') {
      update_head(state, i);
      update_tail(state, i);
    }
  }
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  if (fp == NULL) {
    return NULL;
  }

  size_t capacity = 64;
  size_t length = 0;
  char *buffer = malloc(capacity);
  if (buffer == NULL) {
    return NULL;
  }

  while (fgets(buffer + length, (int)(capacity - length), fp)) {
    length += strlen(buffer + length);

    if (buffer[length - 1] == '\n')
      break;

    capacity *= 2;
    char *new_buffer = realloc(buffer, capacity);
    if (new_buffer == NULL) {
      free(buffer);
      return NULL;
    }
    buffer = new_buffer;
  }

  if (length == 0) {
    free(buffer);
    return NULL;
  }

  char *exact_buffer = realloc(buffer, length + 1);
  return exact_buffer ? exact_buffer : buffer;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  if (fp == NULL) {
    return NULL;
  }

  game_state_t *state = malloc(sizeof(game_state_t));
  if (state == NULL) {
    return NULL;
  }

  state->num_snakes = 0;
  state->snakes = NULL;

  unsigned int num_rows_capacity = 8;
  unsigned int num_rows = 0;
  state->board = malloc(num_rows_capacity * sizeof(char *));
  if (state->board == NULL) {
    free(state);
  }

  char *line;
  while ((line = read_line(fp))) {
    if (num_rows >= num_rows_capacity) {
      num_rows_capacity *= 2;
      char **new_board =
          realloc(state->board, num_rows_capacity * sizeof(char *));
      if (new_board == NULL) {
        free_state(state);
        return NULL;
      }
      state->board = new_board;
    }

    state->board[num_rows] = line;
    num_rows++;
  }
  state->num_rows = num_rows;

  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  assert_msg(state != NULL, "state should not be null");
  assert_msg(state->snakes != NULL, "snakes should not be null");
  assert_msg(state->num_snakes > snum,
             "snum '%u' should be in range of available snakes", snum);

  unsigned int curr_row = state->snakes[snum].tail_row;
  unsigned int curr_col = state->snakes[snum].tail_col;
  char curr = state->board[curr_row][curr_col];

  // Find board cell where value is_head
  while (!is_head(curr)) {
    curr_row = get_next_row(curr_row, curr);
    curr_col = get_next_col(curr_col, curr);

    assert_msg(curr_row < state->num_rows, "Going off the board, row: '%u'",
               curr_row);

    curr = state->board[curr_row][curr_col];
  }

  state->snakes[snum].head_col = curr_col;
  state->snakes[snum].head_row = curr_row;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  assert_msg(state != NULL, "state should not be null");

  size_t snake_capacity = 5;
  state->num_snakes = 0;
  state->snakes = malloc(snake_capacity * sizeof(snake_t));
  assert_msg(state->snakes != NULL, "Unable to allocate memory for snakes");

  for (unsigned int i = 0; i < state->num_rows; i++) {
    assert_msg(state->board[i] != NULL,
               "Unable to initialize snakes because board row '%u' is NULL", i);

    for (unsigned int j = 0; state->board[i][j] != '\0'; j++) {
      if (is_tail(state->board[i][j])) {
        if (state->num_snakes >= snake_capacity) {
          snake_capacity *= 2;
          snake_t *new_snakes =
              realloc(state->snakes, snake_capacity * sizeof(snake_t));
          if (!new_snakes) {
            free(state->snakes);
            assert_msg(
                0, "Unable to realloc snakes for row: '%u', capacity: '%zu'", i,
                snake_capacity);
          }
          state->snakes = new_snakes;
        }

        state->snakes[state->num_snakes].tail_row = i;
        state->snakes[state->num_snakes].tail_col = j;
        state->snakes[state->num_snakes].live = true;

        unsigned int old_num_snakes = state->num_snakes;
        state->num_snakes++;
        find_head(state, old_num_snakes);
      }
    }
  }

  return state;
}
