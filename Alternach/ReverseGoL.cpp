#include <thread>
#include <Windows.h>
#include "ReverseGoL.h"

using namespace std;

// Паттерн (int) - комбинация единиц в квадрате 3x3, записанная в виде порядковых номеров ячеек, начиная с единицы
// Расширенный паттерн (Pattern) - структура, предназначенная для хранения расширенной информации о паттерне
// Ячейка (Cell) - структура, предназначенная для хранения в одном месте расширенного паттерна и инвертированного расширенного паттерна
// Поле (int**) - комбинация паттернов
// Поле ячеек (Cell**) - поле, в котором паттерны заменены на ячейки (для сокращения количества рассчётов)
// Преобразованное поле (int**) - поле, в котором все паттерны заменены на эквивалентные им комбинации единиц и нулей (по логике должно быть bool**, но для более удобного преобразования в обычное поле оставлен тип int**)

enum dir { UP, LEFT, DOWN, RIGHT }; // Направления сдвига паттернов

// Альтернатива мьютексу из <mutex>
class mutex {
	SRWLOCK lk {};
public:
	void lock() { AcquireSRWLockExclusive(&lk); }
	void unlock() { ReleaseSRWLockExclusive(&lk); }
} mtx;

Cell* PATTERNS0; // Паттерны не создающие жизнь в центе
int NUM_OF_PATTERNS0; // Количество паттернов, не создающих жизнь в центе
Cell* PATTERNS1; // Паттерны создающие жизнь в центре
int NUM_OF_PATTERNS1; // Количество паттернов, создающих жизнь в центре
extern int*** FIELDS; // Искомые поля
extern int NUM_OF_FIELDS; // Количество искомых полей

// Поиск ячейки эквиваентной паттерну
Cell FindPattern(int pattern) {
	if (pattern > 100 && pattern < 10000) {
		for (int i = 0; i < NUM_OF_PATTERNS1; i++)
			if (pattern == PATTERNS1[i].pattern.pattern)
				return PATTERNS1[i];
		for (int i = 0; i < NUM_OF_PATTERNS0; i++)
			if (pattern == PATTERNS0[i].pattern.pattern)
				return PATTERNS0[i];
	}
	else {
		for (int i = 0; i < NUM_OF_PATTERNS0; i++)
			if (pattern == PATTERNS0[i].pattern.pattern)
				return PATTERNS0[i];
		for (int i = 0; i < NUM_OF_PATTERNS1; i++)
			if (pattern == PATTERNS1[i].pattern.pattern)
				return PATTERNS1[i];
	}
}
// Поиск ячейки эквиваентной инвертированному паттерну
Cell FindInversePattern(int inversePattern) {
	if (inversePattern > 10000 && inversePattern < 1000000) {
		for (int i = 0; i < NUM_OF_PATTERNS1; i++)
			if (inversePattern == PATTERNS1[i].inversePattern.pattern)
				return PATTERNS1[i];
		for (int i = 0; i < NUM_OF_PATTERNS0; i++)
			if (inversePattern == PATTERNS0[i].inversePattern.pattern)
				return PATTERNS0[i];
	} 
	else {
		for (int i = 0; i < NUM_OF_PATTERNS0; i++)
			if (inversePattern == PATTERNS0[i].inversePattern.pattern)
				return PATTERNS0[i];
		for (int i = 0; i < NUM_OF_PATTERNS1; i++)
			if (inversePattern == PATTERNS1[i].inversePattern.pattern)
				return PATTERNS1[i];
	}
}
// Проверка на содержание цифры в паттерне
bool MatchCheck(int digit, Pattern pattern) {
	bool match = false;
	for (int i = 0; i < pattern.numOfLiveCells && !match; i++)
		if (digit == pattern.patternBitwise[i])
			match = true;
	return match;
}
// Проверка на наличие совпадений в двух паттернах
bool MatchCheck(Pattern pattern1, Pattern pattern2) {
	bool match = pattern1.pattern == pattern2.pattern;
	for (int i = 0; i < pattern1.numOfLiveCells && !match; i++)
		if (MatchCheck(pattern1.patternBitwise[i], pattern2))
			match = true;
	return match;
}
// Проверка на содержание одного паттерна в другом
bool FullMatchCheck(Pattern pattern1, Pattern pattern2) {
	bool match = pattern1.pattern <= pattern2.pattern;
	if (pattern1.pattern != pattern2.pattern)
		for (int i = 0; i < pattern1.numOfLiveCells && match; i++)
			if (!MatchCheck(pattern1.patternBitwise[i], pattern2))
				match = false;
	return match;
}
// Преобразование из поля ячеек в обычное поле
int** CellToInt(Cell** field, int m, int n) {
	int** newField = (int**)malloc(m * sizeof(int*));
	for (int i = 0; i < m; i++) {
		newField[i] = (int*)malloc(n * sizeof(int));
		for (int j = 0; j < n; j++)
			newField[i][j] = field[i][j].pattern.pattern;
	}
	return newField;
}
// Заполение ячейки исходя из эквивалентного паттерна
void Cell::FillCell(int pattern) {
	this->pattern.pattern = 0;
	this->pattern.patternBitwise = nullptr;
	this->pattern.numOfLiveCells = 0;
	this->inversePattern.pattern = 0;
	this->inversePattern.patternBitwise = nullptr;
	this->inversePattern.numOfLiveCells = 0;
	this->pattern.pattern = pattern;
	int* temp = nullptr;
	while (pattern) {
		temp = (int*)realloc(temp, (this->pattern.numOfLiveCells + 1) * sizeof(int));
		temp[this->pattern.numOfLiveCells++] = pattern % 10;
		pattern /= 10;
	}
	this->pattern.patternBitwise = (int*)malloc(this->pattern.numOfLiveCells * sizeof(int));
	for (int i = 0; i < this->pattern.numOfLiveCells; i++)
		this->pattern.patternBitwise[i] = temp[this->pattern.numOfLiveCells - i - 1];
	free(temp);
	for (int i = 1; i <= 9; i++)
		if (!MatchCheck(i, this->pattern)) {
			inversePattern.pattern = inversePattern.pattern * 10 + i;
			inversePattern.patternBitwise = (int*)realloc(inversePattern.patternBitwise, (inversePattern.numOfLiveCells + 1) * sizeof(int));
			inversePattern.patternBitwise[inversePattern.numOfLiveCells++] = i;
		}
}
// Освобождание памяти из под паттернов
void FreePatterns() {
	for (int i = 0; i < NUM_OF_PATTERNS0; i++) {
		free(PATTERNS0[i].pattern.patternBitwise);
		free(PATTERNS0[i].inversePattern.patternBitwise);
	}
	free(PATTERNS0);
	for (int i = 0; i < NUM_OF_PATTERNS1; i++) {
		free(PATTERNS1[i].pattern.patternBitwise);
		free(PATTERNS1[i].inversePattern.patternBitwise);
	}
	free(PATTERNS1);
}
// Освобождение памяти из под поля
int** FreeField(int** field, int m) {
	for (int i = 0; i < m; i++)
		free(field[i]);
	free(field);
	return field;
}
// Освобождение памяти из под поля структур
Cell** FreeField(Cell** field, int m) {
	for (int i = 0; i < m; i++)
		free(field[i]);
	free(field);
	return field;
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
			PATTERNS1 = (Cell*)realloc(PATTERNS1, (NUM_OF_PATTERNS1 + 1) * sizeof(Cell));
			PATTERNS1[NUM_OF_PATTERNS1++].FillCell(pattern);
		}
		else {
			PATTERNS0 = (Cell*)realloc(PATTERNS0, (NUM_OF_PATTERNS0 + 1) * sizeof(Cell));
			PATTERNS0[NUM_OF_PATTERNS0++].FillCell(pattern);
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
// Поиск возможных паттернов, если есть соседний паттерн только с одной из сторон (сверху или слева)
Cell* FindNeighboursOneSide(Cell pattern, dir direction, bool option) {
	if (direction == RIGHT)
		direction = LEFT;
	else if (direction == DOWN)
		direction = UP;
	Cell newPattern = FindPattern(MovePattern(pattern.pattern.pattern, direction));
	Cell newInversePattern = FindInversePattern(MovePattern(pattern.inversePattern.pattern, direction));
	Cell* neighbours = nullptr;
	int numOfNeighbours = 0;
	if (option) {
		for (int i = 0; i < NUM_OF_PATTERNS1; i++)
			if (!MatchCheck(newInversePattern.inversePattern, PATTERNS1[i].pattern))
				if (FullMatchCheck(newPattern.pattern, PATTERNS1[i].pattern)) {
					neighbours = (Cell*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(Cell));
					neighbours[numOfNeighbours++] = PATTERNS1[i];
				}
	}
	else if (!option) {
		for (int i = 0; i < NUM_OF_PATTERNS0; i++)
			if (!MatchCheck(newInversePattern.inversePattern, PATTERNS0[i].pattern))
				if (FullMatchCheck(newPattern.pattern, PATTERNS0[i].pattern)) {
					neighbours = (Cell*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(Cell));
					neighbours[numOfNeighbours++] = PATTERNS0[i];
				}
	}
	if (numOfNeighbours) {
		neighbours = (Cell*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(Cell));
		neighbours[numOfNeighbours] = FindPattern(0);
	}
	return neighbours;
}
// Поиск возможных паттернов, если есть соседние паттерны с двух сторон (сверху и слева)
Cell* FindNeighboursTwoSide(Cell upPattern, Cell leftPattern, bool option) {
	Cell* neighbours = nullptr;
	int numOfNeighbours = 0;
	Cell* downNeighbours = FindNeighboursOneSide(upPattern, DOWN, option);
	if (downNeighbours != nullptr) {
		Cell* rightNeighbours = FindNeighboursOneSide(leftPattern, RIGHT, option);
		if (rightNeighbours != nullptr) {
			int numOfDownNeighbours = 0;
			do {
				int numOfRightNeighbours = 0;
				do {
					if (downNeighbours[numOfDownNeighbours].pattern.pattern == rightNeighbours[numOfRightNeighbours].pattern.pattern) {
						neighbours = (Cell*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(Cell));
						neighbours[numOfNeighbours++] = downNeighbours[numOfDownNeighbours];
					}
				} while (rightNeighbours[++numOfRightNeighbours].pattern.pattern);
			} while (downNeighbours[++numOfDownNeighbours].pattern.pattern);
		}
		free(rightNeighbours);
	}
	free(downNeighbours);
	if (numOfNeighbours) {
		neighbours = (Cell*)realloc(neighbours, (numOfNeighbours + 1) * sizeof(Cell));
		neighbours[numOfNeighbours] = FindPattern(0);
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
// Создание копии поля структур
Cell** CopyField(Cell** field, int m, int n) {
	Cell** newField = (Cell**)malloc(m * sizeof(Cell*));
	for (int i = 0; i < m; i++) {
		newField[i] = (Cell*)malloc(n * sizeof(Cell));
		for (int j = 0; j < n; j++)
			newField[i][j] = field[i][j];
	}
	return newField;
}
// Проверка паттернов, находящихся на границе поля (паттерны на границе не должны создавать жизнь за предалами поля)
bool BoundaryCheck(Cell** field, int m, int n, int i, int j) {
	bool ok = true;
	if (!i || i == m - 1 || !j || j == n - 1) {
		int newField[5][5] = {};
		for (int k = 0; k < field[i][j].pattern.numOfLiveCells; k++)
			switch (field[i][j].pattern.patternBitwise[k]) {
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
		if (!i && ok) {
			if (j) {
				newField[1][0] = MatchCheck(1, field[i][j - 1].pattern);
				newField[2][0] = MatchCheck(4, field[i][j - 1].pattern);
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
				newField[2][0] = MatchCheck(4, field[i][j - 1].pattern);
				newField[3][0] = MatchCheck(7, field[i][j - 1].pattern);
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
				newField[0][1] = MatchCheck(1, field[i - 1][j].pattern);
				newField[0][2] = MatchCheck(2, field[i - 1][j].pattern);
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
				newField[0][2] = MatchCheck(2, field[i - 1][j].pattern);
				newField[0][3] = MatchCheck(3, field[i - 1][j].pattern);
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
void FindAllFieldsCell(Cell** field, int m, int n) {
	bool complete = true;
	for (int i = 0; i < m && complete; i++) {
		for (int j = 0; j < n && complete; j++) {
			if (field[i][j].pattern.pattern < 0) {
				Cell pattern = field[i][j];
				complete = false;
				if (!i) {
					if (!j) {
						if (field[i][j].pattern.pattern + 2) {
							thread th1[140];
							Cell** newFields[140];
							for (int k = 0; k < NUM_OF_PATTERNS1; k++) {
								newFields[k] = CopyField(field, m, n);
								newFields[k][i][j] = PATTERNS1[k];
								if (BoundaryCheck(newFields[k], m, n, i, j))
									th1[k] = thread(FindAllFieldsCell, newFields[k], m, n);
							}
							for (int k = 0; k < NUM_OF_PATTERNS1; k++) {
								if (th1[k].joinable())
									th1[k].join();
								FreeField(newFields[k], m);
							}
						}
						else {
							thread th0[372];
							Cell** newFields[372];
							for (int k = 0; k < NUM_OF_PATTERNS0; k++) {
								newFields[k] = CopyField(field, m, n);
								newFields[k][i][j] = PATTERNS0[k];
								if (BoundaryCheck(newFields[k], m, n, i, j))
									th0[k] = thread(FindAllFieldsCell, newFields[k], m, n);
							}
							for (int k = 0; k < NUM_OF_PATTERNS0; k++) {
								if (th0[k].joinable())
									th0[k].join();
								FreeField(newFields[k], m);
							}
						}
					}
					else {
						Cell* rightNeighbours = FindNeighboursOneSide(field[i][j - 1], RIGHT, field[i][j].pattern.pattern + 2);
						if (rightNeighbours != nullptr) {
							int numOfRightNeighbours = 0;
							do {
								field[i][j] = rightNeighbours[numOfRightNeighbours];
								if (BoundaryCheck(field, m, n, i, j))
									FindAllFieldsCell(field, m, n);
							} while (rightNeighbours[++numOfRightNeighbours].pattern.pattern);
						}
						free(rightNeighbours);
					}
				}
				else {
					if (!j) {
						Cell* downNeighbours = FindNeighboursOneSide(field[i - 1][j], DOWN, field[i][j].pattern.pattern + 2);
						if (downNeighbours != nullptr) {
							int numOfDownNeighbours = 0;
							do {
								field[i][j] = downNeighbours[numOfDownNeighbours];
								if (BoundaryCheck(field, m, n, i, j))
									FindAllFieldsCell(field, m, n);
							} while (downNeighbours[++numOfDownNeighbours].pattern.pattern);
						}
						free(downNeighbours);
					}
					else {
						Cell* neighbours = FindNeighboursTwoSide(field[i - 1][j], field[i][j - 1], field[i][j].pattern.pattern + 2);
						if (neighbours != nullptr) {
							int numOfNeighbours = 0;
							do {
								field[i][j] = neighbours[numOfNeighbours];
								if (BoundaryCheck(field, m, n, i, j))
									FindAllFieldsCell(field, m, n);
							} while (neighbours[++numOfNeighbours].pattern.pattern);
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
		FIELDS[NUM_OF_FIELDS++] = CellToInt(field, m, n);
		mtx.unlock();
	}
}
// Преобразование поля в поле структур
void FindAllFields(int** field, int m, int n) {
	Cell** newField = (Cell**)malloc(m * sizeof(Cell*));
	for (int i = 0; i < m; i++) {
		newField[i] = (Cell*)malloc(n * sizeof(Cell));
		for (int j = 0; j < n; j++)
			newField[i][j].pattern.pattern = field[i][j];
	}
	FindAllFieldsCell(newField, m, n);
	for (int i = 0; i < m; i++)
		free(newField[i]);
	free(newField);
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
