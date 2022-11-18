#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "minimax.h"
#include "ticTacToe.h"

// Global variable containing the best possible move
static tictactoe_location_t choice;

/***********HELPER FUNCTIONS************/

// Recursive algorithm for determining whether the current player
// will win assuming perfect play from both players
//  * Recursively simulates all possible combos of moves
//  * Also stores the optimal move in global variable "choice"
static minimax_score_t minimax(tictactoe_board_t board, bool is_Xs_turn,
                               uint8_t depth) {
  uint8_t numSquares = TICTACTOE_BOARD_ROWS * TICTACTOE_BOARD_COLUMNS;
  minimax_score_t bestScores[numSquares]; // Best score possible from a move
  tictactoe_location_t moves[numSquares]; // Move corresponding to bestScores

  minimax_score_t score;

  /*-----------------BASE CASE------------------*/

  score = minimax_computeBoardScore(&board, is_Xs_turn);

  // If the game ended on the previous turn, return the score based
  // on the previous player's move
  if (minimax_isGameOver(score)) {
    // Bias the score by the depth of the recursion so that:
    //  * The computer will favor wins that happen sooner
    //  * The computer will favor losses that happen later
    minimax_score_t finalScore = is_Xs_turn ? score + depth : score - depth;

    return is_Xs_turn ? score + depth : score - depth;
  }

  /*-----------------RECURSION CASE------------------*/

  // Determine the best possible result from playing on each empty square
  // assuming perfect play from the opposing player.

  // Number of moves that have been simulated thus far
  uint8_t currMoveNum = 0;

  // Iterate through each row of the board
  for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
    // Iterate through each column of the board
    for (uint8_t colPos = 0; colPos < TICTACTOE_BOARD_COLUMNS; colPos++) {
      // Only attempt to simulate play on empty squares
      if (board.squares[rowPos][colPos] == MINIMAX_EMPTY_SQUARE) {

        // Simulate playing at this location
        board.squares[rowPos][colPos] =
            is_Xs_turn ? MINIMAX_X_SQUARE : MINIMAX_O_SQUARE;

        // Recursively call minimax to get the best score that can result from
        // playing on this square
        score = minimax(board, !is_Xs_turn, depth + 1);

        // Populate the bestScores array and the corresponding move in the moves
        // array
        tictactoe_location_t move = {.row = rowPos, .column = colPos};
        moves[currMoveNum] = move;
        bestScores[currMoveNum] = score;

        // Undo the change to the board
        board.squares[rowPos][colPos] = MINIMAX_EMPTY_SQUARE;

        currMoveNum++;
      }
    }
  }

  // Number of empty squares is equal to the number of simulated moves
  uint8_t numEmptySquares = currMoveNum;

  // All empty squares have been iterated over. Return the best possible
  // score from the move-score table and set the "choice" global variable
  // to the move corresponding to that score

  minimax_score_t currScore;
  minimax_score_t bestScore = bestScores[0];

  // If it is X's turn, the best score is the highest score
  if (is_Xs_turn) {
    // Determine the highest score in the move-score table
    for (uint8_t i = 0; i < numEmptySquares; i++) {
      currScore = bestScores[i];
      // Keep track of highest score found and its corresponding move
      if (currScore >= bestScore) {
        bestScore = currScore;
        choice = moves[i];
      }
    }
  }

  // Otherwise, it is the lowest score
  else {
    // Determine the lowest score in the move-score table
    for (uint8_t i = 0; i < numEmptySquares; i++) {
      currScore = bestScores[i];
      // Keep track of lowest score found and its corresponding move
      if (currScore <= bestScore) {
        bestScore = currScore;
        choice = moves[i];
      }
    }
  }

  return bestScore;
}

// Defining characters to print in the board
#define MINIMAX_EMPTY_SQUARE_CHAR ' '
#define MINIMAX_O_SQUARE_CHAR 'O'
#define MINIMAX_X_SQUARE_CHAR 'X'
#define MINIMAX_UNDEFINED_SQUARE_CHAR 'E'

// Prints a representation of the board's state to the terminal
static void printBoard(tictactoe_board_t *board) {
  tictactoe_square_state_t currSquare;

  // Iterate through each row of the board
  for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
    // Iterate through each column of the board
    for (uint8_t colPos = 0; colPos < TICTACTOE_BOARD_COLUMNS; colPos++) {

      currSquare = board->squares[rowPos][colPos];

      // Print the corresponding character for each square type
      switch (currSquare) {
      case MINIMAX_EMPTY_SQUARE:
        printf("%c", MINIMAX_EMPTY_SQUARE_CHAR);
        break;
      case MINIMAX_O_SQUARE:
        printf("%c", MINIMAX_O_SQUARE_CHAR);
        break;
      case MINIMAX_X_SQUARE:
        printf("%c", MINIMAX_X_SQUARE_CHAR);
        break;
      default:
        printf("%c", MINIMAX_UNDEFINED_SQUARE_CHAR);
        break;
      }
    }

    // Print a newline at the end of each row
    printf("\n");
  }
}

/***********HEADER FUNCTIONS************/

// This routine is not recursive but will invoke the recursive minimax function.
// You will call this function from the controlling state machine that you will
// implement in a later milestone. It computes the row and column of the next
// move based upon: the current board and player.
//
// When called from the controlling state machine, you will call this function
// as follows:
// 1. If the computer is playing as X, you will call this function with
// is_Xs_turn = true.
// 2. If the computer is playing as O, you will call this function with
// is_Xs_turn = false.
// This function directly passes the  is_Xs_turn argument into the minimax()
// (helper) function.
tictactoe_location_t minimax_computeNextMove(tictactoe_board_t *board,
                                             bool is_Xs_turn) {
  uint8_t depth = 0;

  // Best move will be saved in the "choice" global
  minimax(*board, is_Xs_turn, depth);

  return choice;
}

// Returns the score of the board.
// This returns one of 4 values: MINIMAX_X_WINNING_SCORE,
// MINIMAX_O_WINNING_SCORE, MINIMAX_DRAW_SCORE, MINIMAX_NOT_ENDGAME
// Note: the is_Xs_turn argument indicates which player just took their
// turn and makes it possible to speed up this function.
// Assumptions:
// (1) if is_Xs_turn == true, the last thing played was an 'X'.
// (2) if is_Xs_turn == false, the last thing played was an 'O'.
// Hint: If you know the game was not over when an 'X' was played,
// you don't need to look for 'O's, and vice-versa.
minimax_score_t minimax_computeBoardScore(tictactoe_board_t *board,
                                          bool is_Xs_turn) {
  // Only look for wins from the previous player
  tictactoe_square_state_t prevPlayer;
  prevPlayer = is_Xs_turn ? MINIMAX_O_SQUARE : MINIMAX_X_SQUARE;

  /*-----------------ROW WINS------------------*/

  // Look for row wins, start iterating through rows
  for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
    bool winFoundInRow = true;

    // If all squares in the row are occupied by prevPlayer, then they won
    for (uint8_t colPos = 0; colPos < TICTACTOE_BOARD_COLUMNS; colPos++) {
      // If the current square isn't occupied by prevPlayer, then there is not a
      // win in this row
      if (board->squares[rowPos][colPos] != prevPlayer) {
        winFoundInRow = false;
        break;
      }
    }

    // If all three squares in the row matched, previous player won
    if (winFoundInRow && prevPlayer == MINIMAX_O_SQUARE) {
      return MINIMAX_O_WINNING_SCORE;
    } else if (winFoundInRow && prevPlayer == MINIMAX_X_SQUARE) {
      return MINIMAX_X_WINNING_SCORE;
    }
  }

  /*------------------COLUMN WINS-----------------*/

  // Look for column wins, start iterating through columns
  for (uint8_t colPos = 0; colPos < TICTACTOE_BOARD_COLUMNS; colPos++) {
    bool winFoundInCol = true;

    // If all squares in the column are occupied by prevPlayer, then they won
    for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
      // If the current square isn't occupied by prevPlayer, then there is not a
      // win in this column
      if (board->squares[rowPos][colPos] != prevPlayer) {
        winFoundInCol = false;
        break;
      }
    }

    // If all three squares in the column matched, previous player won
    if (winFoundInCol && prevPlayer == MINIMAX_O_SQUARE) {
      return MINIMAX_O_WINNING_SCORE;
    } else if (winFoundInCol && prevPlayer == MINIMAX_X_SQUARE) {
      return MINIMAX_X_WINNING_SCORE;
    }
  }

  /*----------------DIAGONAL WIN 1-------------------*/

  uint8_t colPos;
  bool diagonalWinFound1 = true;

  // Look for top-right to bottom-left diagonal win
  for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
    colPos = rowPos;

    // If the current square isn't occupied by prevPlayer, then there is not a
    // win in this diagonal
    if (board->squares[rowPos][colPos] != prevPlayer) {
      diagonalWinFound1 = false;
      break;
    }
  }

  // If all three squares in the column matched, previous player won
  if (diagonalWinFound1 && prevPlayer == MINIMAX_O_SQUARE) {
    return MINIMAX_O_WINNING_SCORE;
  } else if (diagonalWinFound1 && prevPlayer == MINIMAX_X_SQUARE) {
    return MINIMAX_X_WINNING_SCORE;
  }

  /*----------------DIAGONAL WIN 2-------------------*/

  bool diagonalWinFound2 = true;

  // Look for bottom-right to top-left diagonal win
  for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
    colPos = TICTACTOE_BOARD_ROWS - rowPos - 1;

    // If the current square isn't occupied by prevPlayer, then there is not a
    // win in this diagonal
    if (board->squares[rowPos][colPos] != prevPlayer) {
      diagonalWinFound2 = false;
      break;
    }
  }

  // If all three squares in the column matched, previous player won
  if (diagonalWinFound2 && prevPlayer == MINIMAX_O_SQUARE) {
    return MINIMAX_O_WINNING_SCORE;
  } else if (diagonalWinFound2 && prevPlayer == MINIMAX_X_SQUARE) {
    return MINIMAX_X_WINNING_SCORE;
  }

  /*----------------TIE OR MIDGAME-------------------*/

  // Iterate through all squares on the board
  // * If any empty squares, the game is not over
  // * Otherwise, the game is a tie

  // Iterate through each row of the board
  for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
    // Iterate through each column of the board
    for (uint8_t colPos = 0; colPos < TICTACTOE_BOARD_COLUMNS; colPos++) {
      // If there is an empty square, game is not over
      if (board->squares[rowPos][colPos] == MINIMAX_EMPTY_SQUARE) {
        return MINIMAX_NOT_ENDGAME;
      }
    }
  }

  // If this point has been reached, all squares are occupied and no one has
  // won. The game is a draw.
  return MINIMAX_DRAW_SCORE;
}

// Init the board to all empty squares.
void minimax_initBoard(tictactoe_board_t *board) {
  tictactoe_square_state_t currSquare;

  // Iterate through each row of the board
  for (uint8_t rowPos = 0; rowPos < TICTACTOE_BOARD_ROWS; rowPos++) {
    // Iterate through each column of the board
    for (uint8_t colPos = 0; colPos < TICTACTOE_BOARD_COLUMNS; colPos++) {
      // Initialize the square to be empty
      board->squares[rowPos][colPos] = MINIMAX_EMPTY_SQUARE;
    }
  }
}

// Determine that the game is over by looking at the score.
bool minimax_isGameOver(minimax_score_t score) {
  // Game is over if the score is one of the endgame scores
  return score != MINIMAX_NOT_ENDGAME;
}