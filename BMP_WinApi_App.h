#pragma once
#include <windows.h>
#include <wingdi.h>
#include "fstream"
#include <CommCtrl.h>

#define OnMenuClicked 1
#define ExitProgamm   2
#define OnLoadFile    3
#define SobelClick	  4
#define Average_3x3	  5
#define Average_5x5	  6
#define Subtract	  7

HWND hwnd_main;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BITMAPFILEHEADER* FILEHEADER = new BITMAPFILEHEADER;
BITMAPINFOHEADER* INFOHEADER = new BITMAPINFOHEADER;
HBITMAP hBitmap;
HINSTANCE hInst;
OPENFILENAMEA ofn;
FILE* image;
FILE* test_open;
BYTE* buffer = new BYTE;		// Трехканальный буфер, используется при считывании изображения из файла и при выводе в окно через WMPAINT
BYTE* buffer_ch = new BYTE;		// Одноканальный буфер, хранит растр одного канала
BYTE* buffer_adv = new BYTE;	// Расширенный одноканальный буффер, дополняет растр нулями по всему периметру для корректной работы маски на граничных участках изображения
BYTE* buffer_result = new BYTE; // Трехканальный буфер, в который записываются и размножаются на каждый канал данные одноканального буфера

char filename[260];
int padding;
int ch;

// Gx и Gy - маски оператора Собеля
double Gx[3][3] = { {1, 2, 1},
					{0, 0, 0},
					{-1, -2, -1} };

double Gy[3][3] = { {-1, 0, 1 },						
				    {-2, 0, 2},							
				    {-1, 0, 1} };	

double Avg_3x3[3][3] = { {1, 1, 1},  // Маска 3x3 для усреднения 
						 {1, 1, 1},  
						 {1, 1, 1} };

double Avg_5x5[5][5] = { {1, 1, 1, 1, 1}, // Маска 5x5 для усреднения
						{1, 1, 1, 1, 1},
						{1, 1, 1, 1, 1},
						{1, 1, 1, 1, 1},
						{1, 1, 1, 1, 1} };

void SetOpenFileParams(HWND hwnd);	// Функция для конфигурации окна открытия файла
void CreateMainMenu(HWND hwnd);		// Функция для конфигурации меню главного окна
void Subtract_1ch();	// Функция вычитания из исходного изображения размытого изображения
