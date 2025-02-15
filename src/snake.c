#include <stdio.h>
#include <string.h>

#include "snake_utils.h"
#include "state.h"
#include "assert_msg.h"

int main(int argc, char *argv[]) {
  bool io_stdin = false;
  char *in_filename = NULL;
  char *out_filename = NULL;
  game_state_t *state = NULL;

  // Parse arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i < argc - 1) {
      if (io_stdin) {
        fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
        return 1;
      }
      in_filename = argv[i + 1];
      i++;
      continue;
    } else if (strcmp(argv[i], "--stdin") == 0) {
      if (in_filename != NULL) {
        fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
        return 1;
      }
      io_stdin = true;
      continue;
    }
    if (strcmp(argv[i], "-o") == 0 && i < argc - 1) {
      out_filename = argv[i + 1];
      i++;
      continue;
    }
    fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
    return 1;
  }

  // Do not modify anything above this line.

  /* Task 7 */

  // Read board from file, or create default board
  if (in_filename != NULL) {
    FILE *fp = fopen(in_filename, "r");
    assert_msg_exit(fp != NULL, 255, "Unable to open file: '%s', fp is NULL", in_filename);

    state = load_board(fp);
    assert_msg(state != NULL, "Unable to load board, state is NULL");

    state = initialize_snakes(state);
    assert_msg(state != NULL, "Unable to initialize snakes, state is NULL");
    fclose(fp);
  } else if (io_stdin) {
    state = load_board(stdin);
    assert_msg(state != NULL, "Unable to load board from stdin, state is NULL");

    state = initialize_snakes(state);
    assert_msg(state != NULL, "Unable to initialize snakes from stdin, state is NULL");
  } else {
    state = create_default_state();
  }

  update_state(state, deterministic_food);

  // Write updated board to file or stdout
  if (out_filename != NULL) {
    save_board(state, out_filename);
  } else {
    print_board(state, stdout);
  }

  free_state(state);

  return 0;
}
