#include <iostream>
using namespace std;

enum dir { UP, LEFT, DOWN, RIGHT }; // Направления сдвига паттернов

int NUM_OF_FIELDS; // Количество искомых полей
int* PATTERNS; // Паттерны, создающие единственную живую клетку
int*** FIELDS; // Искомые поля

// Очиста полей
int** FreeField(int** field, int m) {
	for (int i = 0; i < m; i++)
		free(field[i]);
	free(field);
	return field;
}
// Сортировка паттернов в лексикографическом порядке
int* Sort(int* patterns, int numOfPatterns) {
	for (int i = 0; i < numOfPatterns; i++)
		for (int j = i + 1; j <= numOfPatterns; j++)
			if (patterns[i] > patterns[j]) {
				int temp = patterns[i];
				patterns[i] = patterns[j];
				patterns[j] = temp;
			}
	return patterns;
}
// Поиск паттернов, создающих единственную живую клетку
int* FindPatterns() {
	int* patterns = nullptr;
	int numOfPatterns = 0;
	bool field[5][5] = {}; // Поле 5x5 для размещения паттернов
	for (int i = 0; i < 512; i++) {
		int numOfLiveCells = 0, pattern = 0;
		for (int m = 0; m < 3 && numOfLiveCells < 4; m++)
			for (int n = 0; n < 3 && numOfLiveCells < 4; n++) {
				numOfLiveCells += field[m + 1][n + 1] = (i >> (3 * m + n)) & 1; // Подсчет количества живых клеток в паттерне
				if (field[m + 1][n + 1])
					pattern = pattern * 10 + (3 * m + n) + 1; // Создание кода паттерна
			}
		if (numOfLiveCells == 3) {
			int numOfSurviveCells = 0;
			for (int m = 1; m < 4 && numOfSurviveCells < 2; m++)
				for (int n = 1; n < 4 && numOfSurviveCells < 2; n++)
					numOfSurviveCells += (field[m - 1][n - 1] + field[m - 1][n] + field[m - 1][n + 1] + field[m][n - 1] + field[m][n] + field[m][n + 1] + field[m + 1][n - 1] + field[m + 1][n] + field[m + 1][n + 1]) == 3; // Подсчет количества клеток, создаваемых паттерном
			if (numOfSurviveCells == 1) {
				patterns = (int*)realloc(patterns, (numOfPatterns + 1) * sizeof(int));
				patterns[numOfPatterns++] = pattern; // Сохранение паттерна
			}
		}
	}
	return Sort(patterns, numOfPatterns - 1);
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
// Составление транспонированного паттерна
int TransposePattern(int pattern) {
	int numOfDigits = 0;
	int* newPattern = (int*)malloc(3 * sizeof(int));
	while (pattern > 0) {
		switch (pattern % 10) {
		case 2:
			newPattern[numOfDigits++] = 4;
			break;
		case 3:
			newPattern[numOfDigits++] = 7;
			break;
		case 4:
			newPattern[numOfDigits++] = 2;
			break;
		case 6:
			newPattern[numOfDigits++] = 8;
			break;
		case 7:
			newPattern[numOfDigits++] = 3;
			break;
		case 8:
			newPattern[numOfDigits++] = 6;
			break;
		default:
			newPattern[numOfDigits++] = pattern % 10;
			break;
		}
		pattern /= 10;
	}
	newPattern = Sort(newPattern, numOfDigits - 1);
	int inversePattern = 0;
	for (int i = 0; i < numOfDigits; i++)
		inversePattern = (inversePattern * 10) + newPattern[i];
	free(newPattern);
	return inversePattern;
}
// Проверка на наличие совпадений в двух паттернах
bool MatchCheck(int pattern1, int pattern2) {
	bool match = false;
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
	bool match = true;
	if (pattern1 != pattern2)
		while (pattern1 > 0 && match) {
			if (!MatchCheck(pattern1 % 10, pattern2))
				match = false;
			pattern1 /= 10;
		}
	return match;
}
// Составление паттерна, дополняющего исходный паттерн до полного
int InversePattern(int pattern) {
	int newPattern = 0;
	for (int i = 1; i <= 9; i++)
		if (!MatchCheck(i, pattern))
			newPattern = newPattern * 10 + i;
	return newPattern;
}
// Проверка паттерна на соответствие паттерну, создающему единственную живую клетку
bool PatternCheck(int pattern) {
	bool coincidence = false;
	for (int i = 0; i < 22 && !coincidence; i++)
		coincidence = PATTERNS[i] == pattern;
	return coincidence;
}
// Поиск возможных паттернов, если есть соседний паттерн с одной из сторон (сверху или слева)
int* FindNeighboursOneSide(int pattern, dir direction) {
	if (direction == RIGHT)
		direction = LEFT;
	else if (direction == DOWN)
		direction = UP;
	int newPattern = MovePattern(pattern, direction);
	int* neighbours = nullptr;
	int numOfNeighbours = 0;
	if (newPattern > 9) {
		if (direction == LEFT)
			newPattern = TransposePattern(newPattern);
		for (int i = 7; i <= 9; i++) {
			int tempPattern = direction == LEFT ? TransposePattern(newPattern * 10 + i) : newPattern * 10 + i;
			if (PatternCheck(tempPattern)) {
				neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
				neighbours[numOfNeighbours++] = tempPattern;
			}
		}
		if (numOfNeighbours) {
			neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
			neighbours[numOfNeighbours] = 0;
		}
	}
	return neighbours;
}
// Поиск возможных паттернов, если есть соседний паттерн по диагонали (сверху слева)
int* FindNeighboursDiagonal(int pattern) {
	int* neighbours = nullptr;
	int numOfNeighbours = 0;
	int* rightNeighbours = FindNeighboursOneSide(pattern, RIGHT);
	if (rightNeighbours != nullptr) {
		int numOfRightNeighbours = 0;
		do {
			int* rightDownNeighbours = FindNeighboursOneSide(rightNeighbours[numOfRightNeighbours], DOWN);
			if (rightDownNeighbours != nullptr) {
				int numOfRightDownNeighbours = 0;
				do {
					neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
					neighbours[numOfNeighbours++] = rightDownNeighbours[numOfRightDownNeighbours];
				} while (rightDownNeighbours[++numOfRightDownNeighbours]);
			}
			free(rightDownNeighbours);
		} while (rightNeighbours[++numOfRightNeighbours]);
	}
	else {
		int* downNeighbours = FindNeighboursOneSide(pattern, DOWN);
		if (downNeighbours != nullptr) {
			int numOfDownNeighbours = 0;
			do {
				int* downRightNeighbours = FindNeighboursOneSide(downNeighbours[numOfDownNeighbours], RIGHT);
				if (downRightNeighbours != nullptr) {
					int numOfRightDownNeighbours = 0;
					do {
						neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
						neighbours[numOfNeighbours++] = downRightNeighbours[numOfRightDownNeighbours];
					} while (downRightNeighbours[++numOfRightDownNeighbours]);
				}
				free(downRightNeighbours);
			} while (downNeighbours[++numOfDownNeighbours]);
		}
		else {
			int newPattern = MovePattern(MovePattern(pattern, LEFT), UP);
			int newInversePattern = MovePattern(MovePattern(InversePattern(pattern), LEFT), UP);
			for (int i = 0; i < 22; i++)
				if (FullMatchCheck(newPattern, PATTERNS[i]) && !MatchCheck(newInversePattern, PATTERNS[i])) {
					neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
					neighbours[numOfNeighbours++] = PATTERNS[i];
				}
		}
		free(downNeighbours);
	}
	free(rightNeighbours);
	if (numOfNeighbours) {
		neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
		neighbours[numOfNeighbours] = 0;
	}
	return neighbours;
}
// Поиск возможных паттернов, если есть соседние паттерны с двух сторон (сверху и слева)
int* FindNeighboursTwoSide(int upPattern, int leftPattern) {
	int* neighbours = nullptr;
	int numOfNeighbours = 0;
	int* downNeighbours = FindNeighboursOneSide(upPattern, DOWN);
	if (downNeighbours != nullptr) {
		int* rightNeighbours = FindNeighboursOneSide(leftPattern, RIGHT);
		if (rightNeighbours != nullptr) {
			int numOfDownNeighbours = 0;
			do {
				int numOfRightNeighbours = 0;
				do {
					if (downNeighbours[numOfDownNeighbours] == rightNeighbours[numOfRightNeighbours]) {
						neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
						neighbours[numOfNeighbours++] = 0;
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
// Подсчет количества живых клеток в паттерне с учётом окружения
int LiveCellsCount(int** field, int m, int n, int i, int j) {
	int minJ = 0 > j - 2 ? 0 : j - 2;
	int maxJ = n - 1 < j + 2 ? n - 1 : j + 2;
	int minI = 0 > i - 2 ? 0 : i - 2;
	int maxI = m - 1 < i + 2 ? m - 1 : i + 2;
	int pattern = 0, k = 0;
	for (int di = minI; di <= maxI; di++) {
		for (int dj = minJ; dj <= maxJ; dj++) {
			if (field[di][dj] > 1) {
				int num = field[di][dj];
				for (int ii = 0; ii < abs(i - di); ii++)
					if (i - di > 0)
						num = MovePattern(num, UP);
					else
						num = MovePattern(num, DOWN);
				for (int jj = 0; jj < abs(j - dj); jj++)
					if (j - dj > 0)
						num = MovePattern(num, LEFT);
					else
						num = MovePattern(num, RIGHT);
				while (num > 0) {
					if (!MatchCheck(num % 10, pattern)) {
						pattern = pattern * 10 + num % 10;
						k++;
					}
					num /= 10;
				}
			}
		}
	}
	return k;
}
// Проверка паттерна на возможность конфликта с ранее заполненными паттернами
bool OverlapCheck(int** field, int m, int n, int i, int j) {
	int ok = true;
	int minJ = 0 > j - 2 ? 0 : j - 2;
	int maxJ = n - 1 < j + 2 ? n - 1 : j + 2;
	int minI = 0 > i - 2 ? 0 : i - 2;
	int maxI = i;
	for (int di = minI; di <= maxI && ok; di++) {
		for (int dj = minJ; dj <= maxJ && ok; dj++) {
			if (i - di == 2 || abs(j - dj) == 2) {
				if (field[di][dj] > 1) {
					int num = field[di][dj];
					for (int ii = 0; ii < i - di; ii++)
						num = MovePattern(num, UP);
					for (int jj = 0; jj < abs(j - dj); jj++)
						if (j - dj > 0)
							num = MovePattern(num, LEFT);
						else
							num = MovePattern(num, RIGHT);
					ok = FullMatchCheck(num, field[i][j]);
					if (ok) {
						num = InversePattern(field[di][dj]);
						for (int ii = 0; ii < i - di; ii++)
							num = MovePattern(num, UP);
						for (int jj = 0; jj < abs(j - dj); jj++)
							if (j - dj > 0)
								num = MovePattern(num, LEFT);
							else
								num = MovePattern(num, RIGHT);
						ok = !MatchCheck(num, field[i][j]);
					}
				}
				else if (!field[di][dj])
					ok = LiveCellsCount(field, m, n, di, dj) != 3;
			}
			else if ((i != di || j != dj) && !field[di][dj])
				ok = LiveCellsCount(field, m, n, di, dj) != 3;
		}
	}
	return ok;
}
// Поиск полей
void FindAllFields(int** field, int m, int n) {
	bool complete = true;
	for (int i = 0; i < m && complete; i++) {
		for (int j = 0; j < n && complete; j++) {
			if (field[i][j] == 1) {
				complete = false;
				if (!i) {
					if (!j) {
						for (int k = 0; k < 22; k++) {
							int** newField = CopyField(field, m, n);
							newField[i][j] = PATTERNS[k];
							FindAllFields(newField, m, n);
							FreeField(newField, m);
						}
					}
					else {
						if (field[i][j - 1]) {
							int* rightNeighbours = FindNeighboursOneSide(field[i][j - 1], RIGHT);
							if (rightNeighbours != nullptr) {
								int numOfRightNeighbours = 0;
								do {
									int** newField = CopyField(field, m, n);
									newField[i][j] = rightNeighbours[numOfRightNeighbours];
									FindAllFields(newField, m, n);
									FreeField(newField, m);
								} while (rightNeighbours[++numOfRightNeighbours]);
							}
							free(rightNeighbours);
						}
						else {
							for (int k = 0; k < 22; k++) {
								int** newField = CopyField(field, m, n);
								newField[i][j] = PATTERNS[k];
								if (OverlapCheck(newField, m, n, i, j))
									FindAllFields(newField, m, n);
								FreeField(newField, m);
							}
						}
					}
				}
				else {
					if (!j) {
						if (field[i - 1][j]) {
							int* downNeighbours = FindNeighboursOneSide(field[i - 1][j], DOWN);
							if (downNeighbours != nullptr) {
								int numOfDownNeighbours = 0;
								do {
									int** newField = CopyField(field, m, n);
									newField[i][j] = downNeighbours[numOfDownNeighbours];
									if (OverlapCheck(newField, m, n, i, j))
										FindAllFields(newField, m, n);
									FreeField(newField, m);
								} while (downNeighbours[++numOfDownNeighbours]);
							}
							free(downNeighbours);
						}
						else {
							for (int k = 0; k < 22; k++) {
								int** newField = CopyField(field, m, n);
								newField[i][j] = PATTERNS[k];
								if (OverlapCheck(newField, m, n, i, j))
									FindAllFields(newField, m, n);
								FreeField(newField, m);
							}
						}
					}
					else {
						if (!field[i - 1][j - 1] && !field[i - 1][j] && !field[i][j - 1]) {
							for (int k = 0; k < 22; k++) {
								int** newField = CopyField(field, m, n);
								newField[i][j] = PATTERNS[k];
								if (OverlapCheck(newField, m, n, i, j))
									FindAllFields(newField, m, n);
								FreeField(newField, m);
							}
						}
						else if (field[i - 1][j] && field[i][j - 1]) {
							int* neighbours = FindNeighboursTwoSide(field[i - 1][j], field[i][j - 1]);
							if (neighbours != nullptr) {
								int numOfNeighbours = 0;
								do {
									int** newField = CopyField(field, m, n);
									newField[i][j] = neighbours[numOfNeighbours];
									FindAllFields(newField, m, n);
									FreeField(newField, m);
								} while (neighbours[++numOfNeighbours]);
							}
							free(neighbours);
						}
						else if (field[i - 1][j]) {
							int* downNeighbours = FindNeighboursOneSide(field[i - 1][j], DOWN);
							if (downNeighbours != nullptr) {
								int numOfDownNeighbours = 0;
								do {
									int** newField = CopyField(field, m, n);
									newField[i][j] = downNeighbours[numOfDownNeighbours];
									FindAllFields(newField, m, n);
									FreeField(newField, m);
								} while (downNeighbours[++numOfDownNeighbours]);
							}
							free(downNeighbours);
						}
						else if (field[i][j - 1]) {
							int* rightNeighbours = FindNeighboursOneSide(field[i][j - 1], RIGHT);
							if (rightNeighbours != nullptr) {
								int numOfRightNeighbours = 0;
								do {
									int** newField = CopyField(field, m, n);
									newField[i][j] = rightNeighbours[numOfRightNeighbours];
									if (OverlapCheck(newField, m, n, i, j))
										FindAllFields(newField, m, n);
									FreeField(newField, m);
								} while (rightNeighbours[++numOfRightNeighbours]);
							}
							free(rightNeighbours);
						}
						else if (field[i - 1][j - 1]) {
							int* diagonalNeighbours = FindNeighboursDiagonal(field[i - 1][j - 1]);
							if (diagonalNeighbours != nullptr) {
								int numOfDiagonalNeighbours = 0;
								do {
									int** newField = CopyField(field, m, n);
									newField[i][j] = diagonalNeighbours[numOfDiagonalNeighbours];
									if (OverlapCheck(newField, m, n, i, j))
										FindAllFields(newField, m, n);
									FreeField(newField, m);
								} while (diagonalNeighbours[++numOfDiagonalNeighbours]);
							}
							free(diagonalNeighbours);
						}
					}
				}
			}
		}
	}
	if (complete) {
		FIELDS = (int***)realloc(FIELDS, (NUM_OF_FIELDS + 1) * sizeof(int**));
		FIELDS[NUM_OF_FIELDS++] = CopyField(field, m, n);
	}
}
// Ввод поля
int** InputField(int& m, int& n) {
	do {
		cout << "Enter M: ";
		cin >> m;
		if (m <= 0)
			cout << "Incorrect M!!!" << endl;
	} while (m <= 0);
	do {
		cout << "Enter N: ";
		cin >> n;
		if (n <= 0)
			cout << "Incorrect N!!!" << endl;
	} while (n <= 0);
	int** field = (int**)malloc(m * sizeof(int*));
	for (int i = 0; i < m; i++) {
		field[i] = (int*)malloc(n * sizeof(int));
		for (int j = 0; j < n; j++)
			field[i][j] = 0;
	}
	int k;
	do {
		cout << "Enter K: ";
		cin >> k;
		if (k <= 0)
			cout << "Incorrect K!!!" << endl;
	} while (n <= 0);
	for (int i = 0; i < k; i++) {
		int i1;
		do {
			cout << "Enter I: ";
			cin >> i1;
			if ((i1 <= 0) || (i1 > m))
				cout << "Incorrect I!!!" << endl;
		} while (((i1 <= 0) || (i1 > m)));
		int j1;
		do {
			cout << "Enter J: ";
			cin >> j1;
			if ((j1 <= 0) || (j1 > n))
				cout << "Incorrect J!!!" << endl;
		} while (((j1 <= 0) || (j1 > n)));
		field[i1 - 1][j1 - 1] = 1;
	}
	return field;
}
// Составление из поля паттернов поля с клетками
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
// Проверка поля
bool FieldCheck(int** field, int** newField, int m, int n) {
	bool error = false;
	for (int i = 0; i < m + 2 && !error; i++)
		for (int j = 0; j < n + 2 && !error; j++) {
			int minJ = 0 > j - 1 ? 0 : j - 1;
			int maxJ = n + 1 < j + 1 ? n + 1 : j + 1;
			int minI = 0 > i - 1 ? 0 : i - 1;
			int maxI = m + 1 < i + 1 ? m + 1 : i + 1;
			int numOfNeighbours = 0;
			for (int di = minI; di <= maxI; di++)
				for (int dj = minJ; dj <= maxJ; dj++)
					numOfNeighbours += newField[di][dj];
			if (i == 0 || i == m + 1 || j == 0 || j == n + 1)
				error = numOfNeighbours == 3;
			else
				error = !((numOfNeighbours == 3 && field[i - 1][j - 1]) || (numOfNeighbours != 3 && !field[i - 1][j - 1]));
		}
	return !error;
}
// Вывод поля на экран
void PrintField(int** field, int m, int n) {
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++)
			cout << field[i][j] << " ";
		cout << endl;
	}
}

int main() {
	NUM_OF_FIELDS = 0;
	FIELDS = nullptr;
	PATTERNS = FindPatterns();
	int m = 0, n = 0;
	int error = 0;
	int** field = InputField(m, n);
	FindAllFields(field, m, n);
	bool printCorrect = false, printErrors = false;
	printCorrect = true;
	printErrors = true;
	for (int i = 0; i < NUM_OF_FIELDS; i++) {
		int** newField = ReformField(FIELDS[i], m, n);
		if (!FieldCheck(field, newField, m, n)) {
			if (printErrors) {
				PrintField(FIELDS[i], m, n);
				cout << "Error" << endl << endl;
			}
			error++;
		}
		else if (printCorrect) {
			PrintField(FIELDS[i], m, n);
			cout << endl;
		}
		FreeField(newField, m + 2);
	}
	cout << "ERROR = " << error << " CORRECT = " << NUM_OF_FIELDS - error << endl;
	system("pause");
	free(PATTERNS);
	for (int i = 0; i < NUM_OF_FIELDS; i++)
		FreeField(FIELDS[i], m);
	free(FIELDS);
	return 0;
}