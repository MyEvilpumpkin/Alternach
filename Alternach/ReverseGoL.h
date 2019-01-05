#pragma once

// ������� - ���������� ������ � �������� 3x3, ���������� � ���� ���������� ������� �����, ������� � �������
// ���� - ���������� ���������
// ��������������� ���� - ����, � ������� ��� �������� �������� �� ������������� �� ���������� ������ � �����

enum dir { UP, LEFT, DOWN, RIGHT }; // ����������� ������ ���������

int** FreeField(int**, int); // ������������ ������ �� ��� ����
void FreePatterns(); // ������������ ������ �� ��� ���������
void FreeFIELDS(int); // ������������ ������ �� ��� �����
void FindAllPatterns(); // ���������� ���� ���������, ��������� � �� ��������� �����
int MovePattern(int, dir); // ����� ��������
bool MatchCheck(int, int); // �������� �� ������� ���������� � ���� ���������
bool FullMatchCheck(int, int); // �������� �� ���������� ������ �������� � ������
int InversePattern(int); // ����������� ��������, ������������ �������� ������� �� ������� (���������� ������ �� ������)
int* FindNeighboursOneSide(int, dir, bool); // ����� ��������� ���������, ���� ���� �������� ������� ������ � ����� �� ������ (������ ��� �����)
int* FindNeighboursTwoSide(int, int, bool); // ����� ��������� ���������, ���� ���� �������� �������� � ���� ������ (������ � �����)
int** CopyField(int**, int, int); // �������� ����� ����
bool BoundaryCheck(int**, int, int, int, int); // �������� ���������, ����������� �� ������� ���� (�������� �� ������� �� ������ ��������� ����� �� ��������� ����)
void FindAllFields(int**, int, int); // ���������� �����, �������������� ��������� ����
int** ReformField(int**, int, int); // ����������� �� ���� ��������� ���������������� ����