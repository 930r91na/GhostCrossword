#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "gameLogicUtils.h"
#include "initBoard.h"
#include "parameters.h"


bool isdigit(char i);

void giveUserInstructions() {
    char buffer;
    printf(BLU "¡Bienvenido al Juego de Crucigramas!\n");
    printf(" Reglas: \n Completa el tablero con los términos correctos antes de que desaparezcan. \n Las palabras se borran cada minuto si no has respondido. \n Asegúrate de que si la palabra debe estar en plural, también la ingreses en plural. \n Las palabras deben estar en mayúsculas, aunque la entrada no distingue entre mayúsculas y minúsculas. \n Ganarás cuando completes el crucigrama." RESET "\n");
    printf("Presiona 'y' cuando estés listo para comenzar, o 'e' para salir: ");
    scanf(" %c", &buffer);
    while (buffer != 'y' && buffer != 'e') {
        printf("Entrada inválida. Por favor presiona 'y' para empezar o 'e' para salir: ");
        scanf(" %c", &buffer);
    }
    if (buffer == 'e') {
        printf("Saliendo del juego como solicitó el usuario.\n");

        kill(getppid(), SIGKILL);
    } else {
        printf("El usuario está listo. Esperando la inicialización del juego...\n");
    }
}

void printAnsweredBoard() {
    // Print the board
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c", crosswordBoardWithAnswers[i][j]);
        }
        printf("\n");
    }
}

bool isTermInHistory(char *word) {
    for (int i = 0; i < historyOfWordsIndex; i++) {
        if (strcmp(historyOfWords[i], word) == 0) {
            return true;
        }
    }
    return false;
}

bool checkAllTermsInBoard() {
    int t = 0;
    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (termsInBoard[i].term.word != NULL) {
            t++;
        }
    }
    if (t == NUMBER_OF_TERMS) {
        return true;
    }
    return false;
}

int max(int a, int b){
    return a > b ? a : b;
}

void printTermsHints() {
    printf("Hints:\n");
    // Storing all terms in arrays
    char horizontalTerms[NUMBER_OF_TERMS][150];
    char verticalTerms[NUMBER_OF_TERMS][150];
    int hCount = 0, vCount = 0;

    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (termsInBoard[i].isHorizontal && !termsInBoard[i].isKnown) {
            sprintf(horizontalTerms[hCount++], "Term %d: %s", termsInBoard[i].index, termsInBoard[i].term.description);
        } else if (!termsInBoard[i].isHorizontal && !termsInBoard[i].isKnown) {
            sprintf(verticalTerms[vCount++], "Term %d: %s", termsInBoard[i].index, termsInBoard[i].term.description);
        }
    }

    // Printing terms in columns
    printf("HORIZONTAL%70sVERTICAL\n", "");
    for (int i = 0; i < max(hCount, vCount); i++) {
        if (i < hCount) {
            printf("%-70s", horizontalTerms[i]);
        } else {
            printf("%-70s", "");
        }

        if (i < vCount) {
            printf("%s", verticalTerms[i]);
        }
        printf("\n");
    }
}

// Generate a potential termToAppear

void *termToAppearGenerator(void *args) {
    while (true) {
        // Constantly checking if a termToAppear is in need
        if (termToAppear.term.word != NULL && !termsInBoard[termToAppear.index].isKnown) {
            continue;
        }

        termInBoard tempTermToAppear;
        int indexTerm = rand() % (NUMBER_OF_TERMS - 1) + 1;

        // Find a term that is not known and is different from the term to appear
        while (termsInBoard[indexTerm].isKnown ||
               (tempTermToAppear.term.word != NULL && strcmp(termsInBoard[indexTerm].term.word, tempTermToAppear.term.word) == 0)) {

            indexTerm = rand() % (NUMBER_OF_TERMS - 1) + 1;
        }

        tempTermToAppear.term.word = NULL;
        tempTermToAppear.index = indexTerm;
        tempTermToAppear.isKnown = false;


        termInBoard termToReplace = termsInBoard[indexTerm];

        // Parameters to search a word with the common intersection
        char toHave = termToReplace.term.word[termToReplace.intersection];
        int spaceUpLeft = 0;
        int spaceDownRight = 0;
        coordinate intersectionCoordinate;
        spaceUpLeft = termToReplace.intersection;
        spaceDownRight = strlen(termToReplace.term.word) - termToReplace.intersection - 1;

        // Get global position a coordinate
        if (termsInBoard[indexTerm].isHorizontal) {
            intersectionCoordinate.row = termToReplace.starts.row;
            intersectionCoordinate.column = termToReplace.starts.column + termToReplace.intersection;

            // Get extra space that may exist
            coordinate temp = termToReplace.starts;
            while (temp.column > 0 && crosswordBoardWithAnswers[temp.row][temp.column-- - 1] == '*') {
                spaceUpLeft++;
            }
            temp = termToReplace.starts;
            while (temp.row < BOARD_SIZE &&
                   crosswordBoardWithAnswers[temp.row][temp.column++ + strlen(termToReplace.term.word)] == '*') {
                spaceDownRight++;
            }
        } else {
            intersectionCoordinate.row = termToReplace.starts.row + termToReplace.intersection;
            intersectionCoordinate.column = termToReplace.starts.column;

            // Get extra space that may exist
            coordinate temp = termToReplace.starts;
            while (temp.row > 0 && crosswordBoardWithAnswers[temp.row-- - 1][temp.column] == '*') {
                spaceUpLeft++;
            }
            temp = termToReplace.starts;
            while (temp.column < BOARD_SIZE && crosswordBoardWithAnswers[temp.row++ + strlen(termToReplace.term.word)][temp.column] == '*') {
                spaceDownRight++;
            }
        }


        coordinate start;
        int tleft = 0;

        // Search a valid candidate

        do{
            tempTermToAppear.term = searchReplacement(&start, &tleft, spaceUpLeft, spaceDownRight,
                                                      intersectionCoordinate, toHave);
        } while (strcmp(tempTermToAppear.term.word, termToReplace.term.word) == 0 || isTermAlreadyInBoard(tempTermToAppear.term));
        // If no term was found, continue
        if (tempTermToAppear.term.word == NULL) {
            continue;
        }
        // Assign valid candidate to global termToAppear

        tempTermToAppear.isHorizontal = termToReplace.isHorizontal;
        // Get Intersection and starts
        if (tempTermToAppear.isHorizontal) {
            tempTermToAppear.starts.row = termsInBoard[indexTerm].starts.row;
            tempTermToAppear.starts.column = start.column + 1;

        } else {
            tempTermToAppear.starts.column = termsInBoard[indexTerm].starts.column;
            tempTermToAppear.starts.row = start.row + 1;
        }
        tempTermToAppear.intersection = tleft;
        termToReplaceIndex = indexTerm;
        if (termCollidesWithBoardCharacters(tempTermToAppear)) {
            continue;
        }
        termToAppear = tempTermToAppear;
    }
}


int answerChecker(char answer[10]) {
    // Check if the answer is correct in any of the terms
    for (int i = 0; i < NUMBER_OF_TERMS; i++) {
        if (strcmp(termsInBoard[i].term.word, answer) == 0) {
            return termsInBoard[i].index;
        }
    }
    return -1;
}

void converterToUpperCase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - 32;
        }
    }
}

bool processUserAnswer() {
    int termNumber;
    printf("Enter your answer: ");
    char answer[11];
    scanf("%10s", answer);

    converterToUpperCase(answer);

    if (answerChecker(answer) != -1) {
        termNumber = answerChecker(answer);
        clockTime = 0;
        printf("Correct answer! Term %d found.\n", termNumber);
        termsInBoard[termNumber].isKnown = true;
        reInitBoard = true;
        return true;
    } else {
        printf("Incorrect answer. Try again.\n");
        return false;
    }
}

void printFormattedBoard(char displayBoard[BOARD_SIZE][BOARD_SIZE][20]) {
    // Print the top border of the board
    printf("\n+");
    for (int j = 0; j < BOARD_SIZE; j++) {
        printf("-------+");
    }
    printf("\n");

    // Print the board with formatting
    for (int i = 0; i < BOARD_SIZE; i++) {
        // Print each row with vertical dividers
        printf("|");
        for (int j = 0; j < BOARD_SIZE; j++) {
            // Ensure each cell has a uniform width
            if (strcmp(displayBoard[i][j], "-") == 0) {
                // If the string is "-", print in white
                printf(WHT " %-5s " RESET "|", displayBoard[i][j]);
            } else if (isdigit(displayBoard[i][j][0])) {
                // If the first character is a digit, print in red
                printf(RED " %-5s " RESET "|", displayBoard[i][j]);
            } else {
                // If the first character is a letter, print in green
                printf(GRN " %-5s " RESET "|", displayBoard[i][j]);
            }
        }
        printf("\n");

        // Print the row border
        printf("+");
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("-------+");
        }
        printf("\n");
    }
}

bool isdigit(char i) {
    return i >= '0' && i <= '9';
}

