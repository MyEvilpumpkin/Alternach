#include <iostream>
using namespace std;

enum dir { UP, LEFT, DOWN, RIGHT }; // Направления сдвига паттернов

int* PATTERNS0;
int NUM_OF_PATTERNS0;
int* PATTERNS1;
int NUM_OF_PATTERNS1;
int*** FIELDS; // Искомые поля
int NUM_OF_FIELDS; // Количество искомых полей

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

void FindAllPatterns() {
	PATTERNS0 = PATTERNS1 = nullptr;
	NUM_OF_PATTERNS0 = NUM_OF_PATTERNS1 = 0;
	bool field[3][3] = {}; // Поле 5x5 для размещения паттернов
	for (int i = 0; i < 512; i++) {
		int numOfLiveCells = 0, pattern = 0;
		for (int m = 0; m < 3; m++)
			for (int n = 0; n < 3; n++) {
				numOfLiveCells += field[m][n] = (i >> (3 * m + n)) & 1; // Подсчет количества живых клеток в паттерне
				if (field[m][n])
					pattern = pattern * 10 + (3 * m + n) + 1; // Создание кода паттерна
			}
		if (numOfLiveCells == 3) {
			PATTERNS1 = (int*)realloc(PATTERNS1, (NUM_OF_PATTERNS1 + 1) * sizeof(int));
			PATTERNS1[NUM_OF_PATTERNS1++] = pattern; // Сохранение паттерна
		}
		else {
			PATTERNS0 = (int*)realloc(PATTERNS0, (NUM_OF_PATTERNS0 + 1) * sizeof(int));
			PATTERNS0[NUM_OF_PATTERNS0++] = pattern; // Сохранение паттерна
		}
	}
	Sort(PATTERNS0, NUM_OF_PATTERNS0 - 1);
	Sort(PATTERNS1, NUM_OF_PATTERNS1 - 1);
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

int LiveCellsCount(int pattern) {
	int numOfLiveCells = 0;
	while(pattern > 0) {
		pattern /= 10;
		numOfLiveCells++;
	}
	return numOfLiveCells;
}
// Поиск возможных паттернов, если есть соседний паттерн с одной из сторон (сверху или слева)
int* FindNeighboursOneSide(int pattern, dir direction, bool option) {
	if (direction == RIGHT)
		direction = LEFT;
	else if (direction == DOWN)
		direction = UP;
	int newPattern = MovePattern(pattern, direction);
	int newInversePattern = MovePattern(InversePattern(pattern), direction);
	int* neighbours = nullptr;
	int numOfNeighbours = 0;
	int numOfLiveCells = LiveCellsCount(newPattern);
	if (numOfLiveCells == 3 && option) {
		neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
		neighbours[numOfNeighbours++] = newPattern;
	} else if (numOfLiveCells < 3 && option) {
		for (int i = 0; i < NUM_OF_PATTERNS1; i++)
			if (FullMatchCheck(newPattern, PATTERNS1[i]) && !MatchCheck(newInversePattern, PATTERNS1[i])) {
				neighbours = (int*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(int));
				neighbours[numOfNeighbours++] = PATTERNS1[i];
			}
	} else if (!option) {
		for (int i = 0; i < NUM_OF_PATTERNS0; i++)
			if (FullMatchCheck(newPattern, PATTERNS0[i]) && !MatchCheck(newInversePattern, PATTERNS0[i])) {
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

bool BoundaryCheck(int pattern, int m, int n, int i, int j) {
	bool ok = true;
	if (!i || i == m - 1 || !j || j == n - 1) {
		int field[5][5] = {};
		while (pattern > 0) {
			switch (pattern % 10) {
			case 1:
				field[1][1] = 1;
				break;
			case 2:
				field[1][2] = 1;
				break;
			case 3:
				field[1][3] = 1;
				break;
			case 4:
				field[2][1] = 1;
				break;
			case 5:
				field[2][2] = 1;
				break;
			case 6:
				field[2][3] = 1;
				break;
			case 7:
				field[3][1] = 1;
				break;
			case 8:
				field[3][2] = 1;
				break;
			case 9:
				field[3][3] = 1;
				break;
			}
			pattern /= 10;
		}
		if (!i && ok) {
			for (int dj = 1; dj < 4 && ok; dj++) {
				int di = 1;
				ok = (field[di - 1][dj - 1] + field[di - 1][dj] + field[di - 1][dj + 1] + field[di][dj - 1] + field[di][dj] + field[di][dj + 1] + field[di + 1][dj - 1] + field[di + 1][dj] + field[di + 1][dj + 1]) != 3;
			}
		}
		if (i == m - 1 && ok) {
			for (int dj = 1; dj < 4 && ok; dj++) {
				int di = 3;
				ok = (field[di - 1][dj - 1] + field[di - 1][dj] + field[di - 1][dj + 1] + field[di][dj - 1] + field[di][dj] + field[di][dj + 1] + field[di + 1][dj - 1] + field[di + 1][dj] + field[di + 1][dj + 1]) != 3;
			}
		}
		if (!j && ok) {
			for (int di = 1; di < 4 && ok; di++) {
				int dj = 1;
				ok = (field[di - 1][dj - 1] + field[di - 1][dj] + field[di - 1][dj + 1] + field[di][dj - 1] + field[di][dj] + field[di][dj + 1] + field[di + 1][dj - 1] + field[di + 1][dj] + field[di + 1][dj + 1]) != 3;
			}
		}
		if (j == n - 1 && ok) {
			for (int di = 1; di < 4 && ok; di++) {
				int dj = 3;
				ok = (field[di - 1][dj - 1] + field[di - 1][dj] + field[di - 1][dj + 1] + field[di][dj - 1] + field[di][dj] + field[di][dj + 1] + field[di + 1][dj - 1] + field[di + 1][dj] + field[di + 1][dj + 1]) != 3;
			}
		}
	}
	return ok;
}

void FindAllFields(int** field, int m, int n) {
	bool complete = true;
	for (int i = 0; i < m && complete; i++) {
		for (int j = 0; j < n && complete; j++) {
			if (field[i][j] < 0) {
				complete = false;
				if (!i) {
					if (!j) {
						if (field[i][j] + 2) {
							for (int k = 0; k < NUM_OF_PATTERNS1; k++) {
								int** newField = CopyField(field, m, n);
								newField[i][j] = PATTERNS1[k];
								if (BoundaryCheck(newField[i][j], m, n, i, j))
									FindAllFields(newField, m, n);
								FreeField(newField, m);
							}
						} else {
							for (int k = 0; k < NUM_OF_PATTERNS0; k++) {
								int** newField = CopyField(field, m, n);
								newField[i][j] = PATTERNS0[k];
								if (BoundaryCheck(newField[i][j], m, n, i, j))
									FindAllFields(newField, m, n);
								FreeField(newField, m);
							}
						}
					} else {
						int* rightNeighbours = FindNeighboursOneSide(field[i][j - 1], RIGHT, field[i][j] + 2);
						if (rightNeighbours != nullptr) {
							int numOfRightNeighbours = 0;
							do {
								int** newField = CopyField(field, m, n);
								newField[i][j] = rightNeighbours[numOfRightNeighbours];
								if (BoundaryCheck(newField[i][j], m, n, i, j))
									FindAllFields(newField, m, n);
								FreeField(newField, m);
							} while (rightNeighbours[++numOfRightNeighbours]);
						}
						free(rightNeighbours);
					}
				} else {
					if (!j) {
						int* downNeighbours = FindNeighboursOneSide(field[i - 1][j], DOWN, field[i][j] + 2);
						if (downNeighbours != nullptr) {
							int numOfDownNeighbours = 0;
							do {
								int** newField = CopyField(field, m, n);
								newField[i][j] = downNeighbours[numOfDownNeighbours];
								if (BoundaryCheck(newField[i][j], m, n, i, j))
									FindAllFields(newField, m, n);
								FreeField(newField, m);
							} while (downNeighbours[++numOfDownNeighbours]);
						}
						free(downNeighbours);
					}
					else {
						int* neighbours = FindNeighboursTwoSide(field[i - 1][j], field[i][j - 1], field[i][j] + 2);
						if (neighbours != nullptr) {
							int numOfNeighbours = 0;
							do {
								int** newField = CopyField(field, m, n);
								newField[i][j] = neighbours[numOfNeighbours];
								if (BoundaryCheck(newField[i][j], m, n, i, j))
									FindAllFields(newField, m, n);
								FreeField(newField, m);
							} while (neighbours[++numOfNeighbours]);
						}
						free(neighbours);
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
			numOfNeighbours -= newField[i][j];
			if (i == 0 || i == m + 1 || j == 0 || j == n + 1)
				error = numOfNeighbours == 3;
			else
				error = !(((numOfNeighbours == 3 || numOfNeighbours + field[i - 1][j - 1] == 3) && field[i - 1][j - 1]) || ((numOfNeighbours + field[i - 1][j - 1] != 3) && !field[i - 1][j - 1]));
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
	FindAllPatterns();
	int m = 0, n = 0;
	int error = 0;
	int** field = InputField(m, n);
	int** Field = CopyField(field, m, n);
	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			Field[i][j] -= 2;
	FindAllFields(Field, m, n);
	FreeField(Field, m);
	bool printCorrect = false, printErrors = false;
	printCorrect = true;
	printErrors = true;
	/*int** newField = ReformField(FIELDS[121], m, n);
	PrintField(newField, m+2, n+2);
	FieldCheck(field, newField, m, n);*/
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
	free(PATTERNS0);
	free(PATTERNS1);
	for (int i = 0; i < NUM_OF_FIELDS; i++)
		FreeField(FIELDS[i], m);
	free(FIELDS);
	return 0;
}