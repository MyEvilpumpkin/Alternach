#pragma once

// Структура, описывающая расширенный паттерн
struct Pattern {
	int pattern; // Паттерн
	int* patternBitwise; // Паттерн в виде массива
	int numOfLiveCells; // Количество живых клеток в паттерне
	Pattern() : pattern(0), patternBitwise(nullptr), numOfLiveCells(0) {};
};

// Структура, описывающая ячейку
struct Cell {
	Pattern pattern; // Расширенный паттерн
	Pattern inversePattern; // Инветрированный расширенный паттерн
	void FillCell(int); // Заполение ячейки исходя из эквивалентного паттерна
};

bool MatchCheck(int, Pattern); // Проверка на содержание цифры в паттерне
int** FreeField(int**, int); // Освобождение памяти из под поля
void FreePatterns(); // Освобождание памяти из под паттернов
void FreeFIELDS(int); // Освобождение памяти из под полей
void FindAllPatterns(); // Нахождение всех паттернов, создающих и не создающих жизнь
int** CopyField(int**, int, int); // Создание копии поля
void FindAllFields(int**, int, int); // Нахождение полей, предшествующих исходному полю
int** ReformField(int**, int, int); // Составление из поля паттернов преобразованного поля
