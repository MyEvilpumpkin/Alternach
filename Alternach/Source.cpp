#include <iostream>
#include <ctime>
#include "ReverseGoL.h"
using namespace std;

int*** FIELDS; // Искомые поля
int NUM_OF_FIELDS; // Количество искомых полей

// Ввод исходного поля
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
		if (k < 0)
			cout << "Incorrect K!!!" << endl;
	} while (k < 0);
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
// Проверка поля на наличие ошибок
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
	clock_t time = clock();
	FindAllFields(Field, m, n);
	time = clock() - time;
	cout << "TICKS = " << time << " SECONDS = " << time / CLK_TCK << endl;
	FreeField(Field, m);
	bool printCorrect = false, printErrors = false;
	//printCorrect = true;
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
	FreePatterns();
	FreeFIELDS(m);
	return 0;
}