#include <stdio.h>

// Rotate one row left by 1
void rotateLeft(int *row, int cols) {
    if (cols <= 1) return;
    int first = row[0];
    for (int j = 0; j < cols - 1; j++) {
        row[j] = row[j + 1];
    }
    row[cols - 1] = first;
}

// Shift all rows in a 2D array
void marqueeLogic(int rows, int cols, int arr[rows][cols]) {
    for (int i = 0; i < rows; i++) {
        rotateLeft(arr[i], cols);
    }
}