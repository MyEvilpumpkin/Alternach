#include <iostream>
#include <thread>
#include <windows.h>
#include "ReverseGoL.h"

class srwlock {
	SRWLOCK _lk {};
public:
	void lock() { AcquireSRWLockExclusive(&_lk); }
	void unlock() { ReleaseSRWLockExclusive(&_lk); }
	srwlock() = default;
};

using namespace std;

srwlock mtx;

int* PATTERNS0; // Паттерны не создающие жизнь в центе
int NUM_OF_PATTERNS0; // Количество паттернов, не создающих жизнь в центе
int* PATTERNS1; // Паттерны создающие жизнь в центре
int NUM_OF_PATTERNS1; // Количество паттернов, создающих жизнь в центре
extern int*** FIELDS; // Искомые поля
extern int NUM_OF_FIELDS; // Количество искомых полей

// Освобождение памяти из под поля
int** FreeField(int** field, int m) {
	for (int i = 0; i < m; i++)
		free(field[i]);
	free(field);
	return field;
}
// Освобождание памяти из под паттернов
void FreePatterns() {
	free(PATTERNS0);
	free(PATTERNS1);
}
// Освобождение памяти из под полей
void FreeFIELDS(int m) {
	for (int i = 0; i < NUM_OF_FIELDS; i++)
		FreeField(FIELDS[i], m);
	free(FIELDS);
	FIELDS = nullptr;
	NUM_OF_FIELDS = 0;
}
// Нахождение всех паттернов, создающих и не создающих жизнь
void FindAllPatterns() {
	PATTERNS0 = PATTERNS1 = nullptr;
	NUM_OF_PATTERNS0 = NUM_OF_PATTERNS1 = 0;
	bool field[3][3] = {};
	for (int i = 0; i < 512; i++) {
		int numOfLiveCells = 0, pattern = 0;
		for (int m = 0; m < 3; m++)
			for (int n = 0; n < 3; n++) {
				numOfLiveCells += field[m][n] = (i >> (3 * m + n)) & 1;
				if (field[m][n])
					pattern = pattern * 10 + (3 * m + n) + 1;
			}
		if (numOfLiveCells - field[1][1] == 3 || numOfLiveCells == 3) {
			PATTERNS1 = (int*)realloc(PATTERNS1, (NUM_OF_PATTERNS1 + 1) * sizeof(int));
			PATTERNS1[NUM_OF_PATTERNS1++] = pattern;
		}
		else {
			PATTERNS0 = (int*)realloc(PATTERNS0, (NUM_OF_PATTERNS0 + 1) * sizeof(int));
			PATTERNS0[NUM_OF_PATTERNS0++] = pattern;
		}
	}
}
// Сдвиг паттерна
int MovePattern(int pattern, dir direction) {
	int newPattern = 0, dec = 1;
	while (pattern > 0) {
		int digit = pattern % 10;
		pattern /= 10;
		if (direction == LEFT) {
			if (digit != 1 && digit != 4 && digit != 7) {
				digit--;
				newPattern += digit * dec;
				dec *= 10;
			}
		}
		else if (direction == UP) {
			if (digit > 3) {
				digit -= 3;
				newPattern += digit * dec;
				dec *= 10;
			}
		}
		else if (direction == RIGHT) {
			if (digit != 3 && digit != 6 && digit != 9) {
				digit++;
				newPattern += digit * dec;
				dec *= 10;
			}
		}
		else if (direction == DOWN) {
			if (digit < 7) {
				digit += 3;
				newPattern += digit * dec;
				dec *= 10;
			}
		}
	}
	return newPattern;
}
// Проверка на наличие совпадений в двух паттернах
bool MatchCheck(int pattern1, int pattern2) {
	bool match = pattern1 == pattern2;
	while (pattern1 > 0 && !match) {
		int temp = pattern2;
		while (temp > 0 && !match) {
			if (pattern1 % 10 == temp % 10)
				match = true;
			temp /= 10;
		}
		pattern1 /= 10;
	}
	return match;
}
// Проверка на содержание одного паттерна в другом
bool FullMatchCheck(int pattern1, int pattern2) {
	bool match = pattern1 <= pattern2;
	if (pattern1 != pattern2)
		while (pattern1 > 0 && match) {
			if (!MatchCheck(pattern1 % 10, pattern2))
				match = false;
			pattern1 /= 10;
		}
	return match;
}
// Составление паттерна, дополняющего исходный паттерн до полного (состоящего только из единиц)
int InversePattern(int pattern) {
	int newPattern = 0;
	for (int i = 1; i <= 9; i++)
		if (!MatchCheck(i, pattern))
			newPattern = newPattern * 10 + i;
	return newPattern;
}
// Поиск возможных паттернов, если есть соседний паттерн только с одной из сторон (сверху или слева)
int* FindNeighboursOneSide(int pattern, dir direction, bool option) {
	if (direction == RIGHT)
		direction = LEFT;
	else if (direction == DOWN)
		direction = UP;
	int newPattern = MovePattern(pattern, direction);
	int newInversePattern = MovePattern(InversePattern(pattern), direction);
	int* neighbours = nullptr;
	int numOfNeighbours = 0;
	if (option) {
		for (int i = 0; i < NUM_OF_PATTERNS1; i++)
			if (!MatchCheck(newInversePattern, PATTERNS1[i]))
				if (FullMatchCheck(newPattern, PATTERNS1[i])) {
					neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
					neighbours[numOfNeighbours++] = PATTERNS1[i];
				}
	}
	else if (!option) {
		for (int i = 0; i < NUM_OF_PATTERNS0; i++)
			if (!MatchCheck(newInversePattern, PATTERNS0[i]))
				if (FullMatchCheck(newPattern, PATTERNS0[i])) {
					neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
					neighbours[numOfNeighbours++] = PATTERNS0[i];
				}
	}
	if (numOfNeighbours) {
		neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
		neighbours[numOfNeighbours] = 0;
	}
	return neighbours;
}
// Поиск возможных паттернов, если есть соседние паттерны с двух сторон (сверху и слева)
int* FindNeighboursTwoSide(int upPattern, int leftPattern, bool option) {
	int* neighbours = nullptr;
	int numOfNeighbours = 0;
	int* downNeighbours = FindNeighboursOneSide(upPattern, DOWN, option);
	if (downNeighbours != nullptr) {
		int* rightNeighbours = FindNeighboursOneSide(leftPattern, RIGHT, option);
		if (rightNeighbours != nullptr) {
			int numOfDownNeighbours = 0;
			do {
				int numOfRightNeighbours = 0;
				do {
					if (downNeighbours[numOfDownNeighbours] == rightNeighbours[numOfRightNeighbours]) {
						neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
						neighbours[numOfNeighbours++] = downNeighbours[numOfDownNeighbours];
					}
				} while (rightNeighbours[++numOfRightNeighbours]);
			} while (downNeighbours[++numOfDownNeighbours]);
		}
		free(rightNeighbours);
	}
	free(downNeighbours);
	if (numOfNeighbours) {
		neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
		neighbours[numOfNeighbours] = 0;
	}
	return neighbours;
}
// Создание копии поля
int** CopyField(int** field, int m, int n) {
	int** newField = (int**)malloc(m * sizeof(int*));
	for (int i = 0; i < m; i++) {
		newField[i] = (int*)malloc(n * sizeof(int));
		for (int j = 0; j < n; j++)
			newField[i][j] = field[i][j];
	}
	return newField;
}
// Проверка паттернов, находящихся на границе поля (паттерны на границе не должны создавать жизнь за предалами поля)
bool BoundaryCheck(int** field, int m, int n, int i, int j) {
	bool ok = true;
	if (!i || i == m - 1 || !j || j == n - 1) {
		int pattern = field[i][j];
		int newField[5][5] = {};
		while (pattern > 0) {
			switch (pattern % 10) {
			case 1:
				newField[1][1] = 1;
				break;
			case 2:
				newField[1][2] = 1;
				break;
			case 3:
				newField[1][3] = 1;
				break;
			case 4:
				newField[2][1] = 1;
				break;
			case 5:
				newField[2][2] = 1;
				break;
			case 6:
				newField[2][3] = 1;
				break;
			case 7:
				newField[3][1] = 1;
				break;
			case 8:
				newField[3][2] = 1;
				break;
			case 9:
				newField[3][3] = 1;
				break;
			}
			pattern /= 10;
		}
		if (!i && ok) {
			if (j) {
				newField[1][0] = MatchCheck(1, field[i][j - 1]);
				newField[2][0] = MatchCheck(4, field[i][j - 1]);
			}
			if (j != n - 1)
				for (int dj = 1; dj <= 2 && ok; dj++) {
					int di = 1;
					int numOfNeighbours = newField[di - 1][dj - 1] + newField[di - 1][dj] + newField[di - 1][dj + 1] + newField[di][dj - 1] + newField[di][dj + 1] + newField[di + 1][dj - 1] + newField[di + 1][dj] + newField[di + 1][dj + 1];
					if ((numOfNeighbours == 3) || (numOfNeighbours + newField[di][dj] == 3))
						ok = false;
					else if (newField[di][dj - 1] + newField[di][dj] + newField[di][dj + 1] == 3)
						ok = false;
				}
			else
				for (int dj = 2; dj <= 3 && ok; dj++) {
					int di = 1;
					int numOfNeighbours = newField[di - 1][dj - 1] + newField[di - 1][dj] + newField[di - 1][dj + 1] + newField[di][dj - 1] + newField[di][dj + 1] + newField[di + 1][dj - 1] + newField[di + 1][dj] + newField[di + 1][dj + 1];
					if ((numOfNeighbours == 3) || (numOfNeighbours + newField[di][dj] == 3))
						ok = false;
					else if (newField[di][dj - 1] + newField[di][dj] + newField[di][dj + 1] == 3)
						ok = false;
				}
		}
		if (i == m - 1 && ok) {
			if (j) {
				newField[2][0] = MatchCheck(4, field[i][j - 1]);
				newField[3][0] = MatchCheck(7, field[i][j - 1]);
			}
			if (j != n - 1)
				for (int dj = 1; dj <= 2 && ok; dj++) {
					int di = 3;
					int numOfNeighbours = newField[di - 1][dj - 1] + newField[di - 1][dj] + newField[di - 1][dj + 1] + newField[di][dj - 1] + newField[di][dj + 1] + newField[di + 1][dj - 1] + newField[di + 1][dj] + newField[di + 1][dj + 1];
					if ((numOfNeighbours == 3) || (numOfNeighbours + newField[di][dj] == 3))
						ok = false;
					else if (newField[di][dj - 1] + newField[di][dj] + newField[di][dj + 1] == 3)
						ok = false;
				}
			else
				for (int dj = 2; dj <= 3 && ok; dj++) {
					int di = 3;
					int numOfNeighbours = newField[di - 1][dj - 1] + newField[di - 1][dj] + newField[di - 1][dj + 1] + newField[di][dj - 1] + newField[di][dj + 1] + newField[di + 1][dj - 1] + newField[di + 1][dj] + newField[di + 1][dj + 1];
					if ((numOfNeighbours == 3) || (numOfNeighbours + newField[di][dj] == 3))
						ok = false;
					else if (newField[di][dj - 1] + newField[di][dj] + newField[di][dj + 1] == 3)
						ok = false;
				}
		}
		if (!j && ok) {
			if (i) {
				newField[0][1] = MatchCheck(1, field[i - 1][j]);
				newField[0][2] = MatchCheck(2, field[i - 1][j]);
			}
			if (i != m - 1)
				for (int di = 1; di <= 2 && ok; di++) {
					int dj = 1;
					int numOfNeighbours = newField[di - 1][dj - 1] + newField[di - 1][dj] + newField[di - 1][dj + 1] + newField[di][dj - 1] + newField[di][dj + 1] + newField[di + 1][dj - 1] + newField[di + 1][dj] + newField[di + 1][dj + 1];
					if ((numOfNeighbours == 3) || (numOfNeighbours + newField[di][dj] == 3))
						ok = false;
					else if (newField[di - 1][dj] + newField[di][dj] + newField[di + 1][dj] == 3)
						ok = false;
				}
			else
				for (int di = 2; di <= 3 && ok; di++) {
					int dj = 1;
					int numOfNeighbours = newField[di - 1][dj - 1] + newField[di - 1][dj] + newField[di - 1][dj + 1] + newField[di][dj - 1] + newField[di][dj + 1] + newField[di + 1][dj - 1] + newField[di + 1][dj] + newField[di + 1][dj + 1];
					if ((numOfNeighbours == 3) || (numOfNeighbours + newField[di][dj] == 3))
						ok = false;
					else if (newField[di - 1][dj] + newField[di][dj] + newField[di + 1][dj] == 3)
						ok = false;
				}
		}
		if (j == n - 1 && ok) {
			if (i) {
				newField[0][2] = MatchCheck(2, field[i - 1][j]);
				newField[0][3] = MatchCheck(3, field[i - 1][j]);
			}
			if (i != m - 1)
				for (int di = 1; di <= 2 && ok; di++) {
					int dj = 3;
					int numOfNeighbours = newField[di - 1][dj - 1] + newField[di - 1][dj] + newField[di - 1][dj + 1] + newField[di][dj - 1] + newField[di][dj + 1] + newField[di + 1][dj - 1] + newField[di + 1][dj] + newField[di + 1][dj + 1];
					if ((numOfNeighbours == 3) || (numOfNeighbours + newField[di][dj] == 3))
						ok = false;
					else if (newField[di - 1][dj] + newField[di][dj] + newField[di + 1][dj] == 3)
						ok = false;
				}
			else
				for (int di = 2; di <= 3 && ok; di++) {
					int dj = 3;
					int numOfNeighbours = newField[di - 1][dj - 1] + newField[di - 1][dj] + newField[di - 1][dj + 1] + newField[di][dj - 1] + newField[di][dj + 1] + newField[di + 1][dj - 1] + newField[di + 1][dj] + newField[di + 1][dj + 1];
					if ((numOfNeighbours == 3) || (numOfNeighbours + newField[di][dj] == 3))
						ok = false;
					else if (newField[di - 1][dj] + newField[di][dj] + newField[di + 1][dj] == 3)
						ok = false;
				}
		}
	}
	return ok;
}
// Нахождение полей, предшествующих исходному полю
void FindAllFields(int** field, int m, int n) {
	bool complete = true;
	for (int i = 0; i < m && complete; i++) {
		for (int j = 0; j < n && complete; j++) {
			if (field[i][j] < 0) {
				int pattern = field[i][j];
				complete = false;
				if (!i) {
					if (!j) {
						if (field[i][j] + 2) {
							thread th1[140];
							int** newFields[140];
							for (int k = 0; k < NUM_OF_PATTERNS1; k++) {
								newFields[k] = CopyField(field, m, n);
								newFields[k][i][j] = PATTERNS1[k];
								if (BoundaryCheck(newFields[k], m, n, i, j))
									th1[k] = thread(FindAllFields, newFields[k], m, n);
							}
							for (int k = 0; k < NUM_OF_PATTERNS1; k++) {
								if (th1[k].joinable())
									th1[k].join();
								FreeField(newFields[k], m);
							}
						}
						else {
							thread th0[372];
							int** newFields[372];
							for (int k = 0; k < NUM_OF_PATTERNS0; k++) {
								newFields[k] = CopyField(field, m, n);
								newFields[k][i][j] = PATTERNS0[k];
								if (BoundaryCheck(newFields[k], m, n, i, j))
									th0[k] = thread(FindAllFields, newFields[k], m, n);
							}
							for (int k = 0; k < NUM_OF_PATTERNS0; k++) {
								if (th0[k].joinable())
									th0[k].join();
								FreeField(newFields[k], m);
							}
						}
					}
					else {
						int* rightNeighbours = FindNeighboursOneSide(field[i][j - 1], RIGHT, field[i][j] + 2);
						if (rightNeighbours != nullptr) {
							int numOfRightNeighbours = 0;
							do {
								field[i][j] = rightNeighbours[numOfRightNeighbours];
								if (BoundaryCheck(field, m, n, i, j))
									FindAllFields(field, m, n);
							} while (rightNeighbours[++numOfRightNeighbours]);
						}
						free(rightNeighbours);
					}
				}
				else {
					if (!j) {
						int* downNeighbours = FindNeighboursOneSide(field[i - 1][j], DOWN, field[i][j] + 2);
						if (downNeighbours != nullptr) {
							int numOfDownNeighbours = 0;
							do {
								field[i][j] = downNeighbours[numOfDownNeighbours];
								if (BoundaryCheck(field, m, n, i, j))
									FindAllFields(field, m, n);
							} while (downNeighbours[++numOfDownNeighbours]);
						}
						free(downNeighbours);
					}
					else {
						int* neighbours = FindNeighboursTwoSide(field[i - 1][j], field[i][j - 1], field[i][j] + 2);
						if (neighbours != nullptr) {
							int numOfNeighbours = 0;
							do {
								field[i][j] = neighbours[numOfNeighbours];
								if (BoundaryCheck(field, m, n, i, j))
									FindAllFields(field, m, n);
							} while (neighbours[++numOfNeighbours]);
						}
						free(neighbours);
					}
				}
				field[i][j] = pattern;
			}
		}
	}
	if (complete) {
		mtx.lock();
		FIELDS = (int***)realloc(FIELDS, (NUM_OF_FIELDS + 1) * sizeof(int**));
		FIELDS[NUM_OF_FIELDS++] = CopyField(field, m, n);
		mtx.unlock();
	}
}
// Составление из поля паттернов преобразованного поля
int** ReformField(int** field, int m, int n) {
	int** newField = (int**)malloc((m + 2) * sizeof(int*));
	for (int i = 0; i < m + 2; i++) {
		newField[i] = (int*)malloc((n + 2) * sizeof(int));
		for (int j = 0; j < n + 2; j++)
			newField[i][j] = false;
	}
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			if (field[i][j]) {
				int num = field[i][j];
				while (num > 0) {
					switch (num % 10) {
					case 1:
						newField[i][j] = 1;
						break;
					case 2:
						newField[i][j + 1] = 1;
						break;
					case 3:
						newField[i][j + 2] = 1;
						break;
					case 4:
						newField[i + 1][j] = 1;
						break;
					case 5:
						newField[i + 1][j + 1] = 1;
						break;
					case 6:
						newField[i + 1][j + 2] = 1;
						break;
					case 7:
						newField[i + 2][j] = 1;
						break;
					case 8:
						newField[i + 2][j + 1] = 1;
						break;
					case 9:
						newField[i + 2][j + 2] = 1;
						break;
					}
					num /= 10;
				}
			}
	return newField;
}
