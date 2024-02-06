#define _CRT_SECURE_NO_WARNINGS
#include "BMP_WinApi_App.h"
#include "resource.h"
	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Регистрация класса окна
	const wchar_t CLASS_NAME[] = L"Show Bmp Class";
	WNDCLASS wc = { };
	wc.hInstance = hInstance;	// Дескриптор экземпляра, который содержит оконную процедуру для класса
	wc.lpszClassName = CLASS_NAME;	 // Строка, указывающая имя класса окна
	wc.lpfnWndProc = WindowProc; // Указатель на оконную процедуру
	wc.style = CS_HREDRAW | CS_VREDRAW;		// Стиль класса
	wc.hIcon = LoadIcon(hInst, IDI_INFORMATION);	// Дескриптор значка класса
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);	// Дескриптор курсора класса
	wc.lpszMenuName = NULL;		// Имя ресурса меню класса, которое размещается в файле ресурса
	wc.cbClsExtra = sizeof(&wc);	// Число дополнительных байт, которые размещаются вслед за структурой класса окна
	wc.cbWndExtra = 0;	// устанавливает число дополнительных байтов, которые размещаются вслед за экземпляром окна
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);	// Дескриптор кисти фона класса

	RegisterClass(&wc);		// Регистрирация класса окна для последующего использования
	// Создание окна
	hwnd_main = CreateWindowEx(
		0,               // Улучшенный стиль окна   
		CLASS_NAME,      // Имя класса окна
		L"TransViewerBMP",   // Тмя окна    
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,  // Стиль окна
		CW_USEDEFAULT,	 // Позиция окна по оси x
		CW_USEDEFAULT,   // Позиция окна по оси y
		CW_USEDEFAULT,   // Ширина окна
		CW_USEDEFAULT,   // Высота окна
		NULL,            // Дескриптор родительского окна или окна владельца
		NULL,            // Дескриптор меню
		hInstance,       // Дескриптор экземпляра
		NULL             // Указатель на произвольные данные типа
	);

	if (hwnd_main == NULL) return 0;

	ShowWindow(hwnd_main, SW_SHOWMAXIMIZED); // Задает состояние отображения исходного окна
	UpdateWindow(hwnd_main);	// Обновляет клиентскую область указанного окна, отправляя WM_PAINT сообщение в окно

	MSG msg = { };		// Содержит информацию сообщения из очереди сообщений потока
	while (GetMessage(&msg, NULL, 0, 0) > 0)	// Извлекает сообщение из очереди сообщений вызывающего потока. NULL - обрабатываются как сообщения окна, так и сообщения потока
	{
		TranslateMessage(&msg);		// Преобразует сообщения с виртуальным ключом в символьные сообщения
		DispatchMessage(&msg);		// Отправляет сообщение в оконную процедуру.
	}
	return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)	   // Обрабатывает сообщения, отправляемые в окно
{
	switch (uMsg)
	{
	case WM_DESTROY:  // Закрытие окна
		DeleteObject(hBitmap); // Освобождает все системные ресурсы (кисть, шрифт, растр и т.д.), связанные с объектом
		PostQuitMessage(0); // Указывает системе, что поток сделал запрос на то, чтобы завершить свою работу
		return 0;
	case WM_COMMAND:
		switch (wParam)
		{
		case OnLoadFile:  // Нажатие кнопки "Открыть"
			if (GetOpenFileNameA(&ofn))
			{
				delete[] buffer;
				image = fopen(filename, "rb");
				fread(FILEHEADER, sizeof(BITMAPFILEHEADER), 1, image);
				fread(INFOHEADER, sizeof(BITMAPINFOHEADER), 1, image);
				ch = 3;
				padding = 0;
				if (((INFOHEADER->biWidth * 3) % 4) != 0) padding = 4 - (INFOHEADER->biWidth * 3) % 4;
				buffer = new BYTE[(INFOHEADER->biWidth * ch + padding) * INFOHEADER->biHeight];

				for (int i = 0; i < INFOHEADER->biHeight; i++)
				{
					fread(&buffer[i * INFOHEADER->biWidth * ch], sizeof(BYTE), INFOHEADER->biWidth * ch, image);
				}
				fclose(image);
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
			}
			break;
		case SobelClick:	// Нажатие кнопки "Оператор Собеля"
			delete[] buffer_ch;
			delete[] buffer_adv;
			delete[] buffer_result;
			buffer_ch = new BYTE[INFOHEADER->biWidth * INFOHEADER->biHeight];

			for (int i = 0, j = 0; i < INFOHEADER->biWidth * INFOHEADER->biHeight; i++, j += 3)
			{
				buffer_ch[i] = buffer[j];
			}

			buffer_adv = new BYTE[(INFOHEADER->biWidth + 2) * (INFOHEADER->biHeight + 2)];
			ZeroMemory(buffer_adv, (INFOHEADER->biWidth + 2) * (INFOHEADER->biHeight + 2));
			for (int i = 0; i < INFOHEADER->biHeight; i++)			
			{
				for (int j = 0; j < INFOHEADER->biWidth; j++)
				{
					buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 1)] = buffer_ch[i * INFOHEADER->biWidth + j];
				}
			}

			buffer_result = new BYTE[INFOHEADER->biWidth * INFOHEADER->biHeight];
			for (int i = 1; i < INFOHEADER->biHeight + 1; i++)
			{
				for (int j = 1; j < INFOHEADER->biWidth + 1; j++)
				{
					double gx = Gx[0][0] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j - 1)] + Gx[0][1] * buffer_adv[(i - 1) * INFOHEADER->biWidth + j] + Gx[0][2] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j + 1)] +
						Gx[1][0] * buffer_adv[i * INFOHEADER->biWidth + (j - 1)] + Gx[1][1] * buffer_adv[i * INFOHEADER->biWidth + j] + Gx[1][2] * buffer_adv[i * INFOHEADER->biWidth + (j + 1)] +
						Gx[2][0] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j - 1)] + Gx[2][1] * buffer_adv[(i + 1) * INFOHEADER->biWidth + j] + Gx[2][2] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 1)];

					double gy = Gy[0][0] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j - 1)] + Gy[0][1] * buffer_adv[(i - 1) * INFOHEADER->biWidth + j] + Gy[0][2] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j + 1)] +
						Gy[1][0] * buffer_adv[i * INFOHEADER->biWidth + (j - 1)] + Gy[1][1] * buffer_adv[i * INFOHEADER->biWidth + j] + Gy[1][2] * buffer_adv[i * INFOHEADER->biWidth + (j + 1)] +
						Gy[2][0] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j - 1)] + Gy[2][1] * buffer_adv[(i + 1) * INFOHEADER->biWidth + j] + Gy[2][2] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 1)];
					double Gres = sqrt(gx * gx + gy * gy);
					if (Gres < 0) Gres = 0;
					if (Gres > 255) Gres = 255;
					buffer_result[(i - 1) * INFOHEADER->biWidth + (j - 1)] = Gres;
				}
			}

			for (int i = 0; i < INFOHEADER->biWidth * INFOHEADER->biHeight; i++)
			{
				buffer[3 * i] = buffer[3 * i + 1] = buffer[3 * i + 2] = buffer_result[i];
			}
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
			break;
		case Average_3x3:	// Нажатие кнопки "Усреднение 3x3" 
			delete[] buffer_ch;
			delete[] buffer_adv;
			delete[] buffer_result;
			buffer_ch = new BYTE[INFOHEADER->biWidth * INFOHEADER->biHeight];
			for (int i = 0, j = 0; i < INFOHEADER->biWidth * INFOHEADER->biHeight; i++, j += 3)
			{
				buffer_ch[i] = buffer[j];
			}

			buffer_adv = new BYTE[(INFOHEADER->biWidth + 2) * (INFOHEADER->biHeight + 2)];
			ZeroMemory(buffer_adv, (INFOHEADER->biWidth + 2) * (INFOHEADER->biHeight + 2));
			for (int i = 0; i < INFOHEADER->biHeight; i++)
			{
				for (int j = 0; j < INFOHEADER->biWidth; j++)
				{
					buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 1)] = buffer_ch[i * INFOHEADER->biWidth + j];
				}
			}
			buffer_result = new BYTE[INFOHEADER->biWidth * INFOHEADER->biHeight];
			for (int i = 1; i < INFOHEADER->biHeight + 1; i++)
			{
				for (int j = 1; j < INFOHEADER->biWidth + 1; j++)
				{
					double conv = Avg_3x3[0][0] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j - 1)] + Avg_3x3[0][1] * buffer_adv[(i - 1) * INFOHEADER->biWidth + j] + Avg_3x3[0][2] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j + 1)] +
						Avg_3x3[1][0] * buffer_adv[i * INFOHEADER->biWidth + (j - 1)] + Avg_3x3[1][1] * buffer_adv[i * INFOHEADER->biWidth + j] + Avg_3x3[1][2] * buffer_adv[i * INFOHEADER->biWidth + (j + 1)] +
						Avg_3x3[2][0] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j - 1)] + Avg_3x3[2][1] * buffer_adv[(i + 1) * INFOHEADER->biWidth + j] + Avg_3x3[2][2] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 1)];
					double res = conv / 9;
					buffer_result[(i - 1) * INFOHEADER->biWidth + (j - 1)] = res;
				}
			}
			for (int i = 0, j = 0; i < INFOHEADER->biWidth * INFOHEADER->biHeight; i++, j += 3)
			{
				buffer[3 * i] = buffer[3 * i + 1] = buffer[3 * i + 2] = buffer_result[i];
			}
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
			break;
		case Average_5x5:	// Нажатие кнопки "Усреднение 5x5"
			delete[] buffer_ch;
			delete[] buffer_adv;
			delete[] buffer_result;

			buffer_ch = new BYTE[INFOHEADER->biWidth * INFOHEADER->biHeight];
			for (int i = 0, j = 0; i < INFOHEADER->biWidth * INFOHEADER->biHeight; i++, j += 3)
			{
				buffer_ch[i] = buffer[j];
			}

			buffer_adv = new BYTE[(INFOHEADER->biWidth + 3) * (INFOHEADER->biHeight + 3)];
			ZeroMemory(buffer_adv, (INFOHEADER->biWidth + 3) * (INFOHEADER->biHeight + 3));
			for (int i = 0; i < INFOHEADER->biHeight; i++)			
			{
				for (int j = 0; j < INFOHEADER->biWidth; j++)
				{
					buffer_adv[(i + 2) * INFOHEADER->biWidth + (j + 2)] = buffer_ch[i * INFOHEADER->biWidth + j];
				}
			}

			buffer_result = new BYTE[INFOHEADER->biWidth * INFOHEADER->biHeight];
			for (int i = 2; i < INFOHEADER->biHeight + 2; i++)
			{
				for (int j = 2; j < INFOHEADER->biWidth + 2; j++)
				{
					double conv = Avg_5x5[0][0] * buffer_adv[(i - 2) * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[0][1] * buffer_adv[(i - 2) * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[0][2] * buffer_adv[(i - 2) * INFOHEADER->biWidth + j] + Avg_5x5[0][3] * buffer_adv[(i - 2) * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[0][4] * buffer_adv[(i - 2) * INFOHEADER->biWidth + (j + 2)] +
						Avg_5x5[1][0] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[1][1] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[1][2] * buffer_adv[(i - 1) * INFOHEADER->biWidth + j] + Avg_5x5[0][3] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[0][4] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j + 2)] +
						Avg_5x5[2][0] * buffer_adv[i * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[2][1] * buffer_adv[i * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[2][2] * buffer_adv[i * INFOHEADER->biWidth + j] + Avg_5x5[2][3] * buffer_adv[i * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[2][4] * buffer_adv[i * INFOHEADER->biWidth + (j + 2)] +
						Avg_5x5[3][0] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[3][1] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[3][2] * buffer_adv[(i + 1) * INFOHEADER->biWidth + j] + Avg_5x5[3][3] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[3][4] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 2)] +
						Avg_5x5[4][0] * buffer_adv[(i + 2) * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[4][1] * buffer_adv[(i + 2) * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[4][2] * buffer_adv[(i + 2) * INFOHEADER->biWidth + j] + Avg_5x5[4][3] * buffer_adv[(i + 2) * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[4][4] * buffer_adv[(i + 2) * INFOHEADER->biWidth + (j + 2)];
					double res = conv / 25;
					buffer_result[(i - 2) * INFOHEADER->biWidth + (j - 2)] = res;
				}
			}
			for (int i = 0, j = 0; i < INFOHEADER->biWidth * INFOHEADER->biHeight; i++)
			{
				buffer[3 * i] = buffer[3 * i + 1] = buffer[3 * i + 2] = buffer_result[i];
			}
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
			break;
		case Subtract:	// Нажатие кнопки "Вычитание"
			Subtract_1ch();
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
			break;
		case ExitProgamm:	// Нажатие кнопки "Выход"
			PostQuitMessage(0);
			break;
		}
	case WM_CREATE:
		CreateMainMenu(hwnd);
		SetOpenFileParams(hwnd);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;		// Cодержит сведения для приложения для рисования клиентской области окна
		HDC hdc;	 // Дескриптор графического объекта
		hdc = BeginPaint(hwnd, &ps);	// Заполнение структуры PAINTSTRUCT информацией об окрашивании

		BITMAPINFO bitmapinfo;
		bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapinfo.bmiHeader.biWidth = INFOHEADER->biWidth;
		bitmapinfo.bmiHeader.biHeight = INFOHEADER->biHeight;
		bitmapinfo.bmiHeader.biPlanes = 1;
		bitmapinfo.bmiHeader.biBitCount = INFOHEADER->biPlanes * INFOHEADER->biBitCount;
		bitmapinfo.bmiHeader.biCompression = BI_RGB;
		bitmapinfo.bmiHeader.biSizeImage = INFOHEADER->biSizeImage;

		SetDIBitsToDevice(hdc, 0, 0, INFOHEADER->biWidth, INFOHEADER->biHeight, 0, 0, 0, INFOHEADER->biHeight, buffer, &bitmapinfo, DIB_RGB_COLORS);
		EndPaint(hwnd, &ps);
	}
	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);	// гарантирует, что обрабатывается каждое сообщение.
}


void CreateMainMenu(HWND hwnd) // Функция для конфигурации меню главного окна
{
	HMENU RootMenu = CreateMenu(); // дескриптор меню
	HMENU SubMenu = CreateMenu(); // дескриптор подменю

	AppendMenuW(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"Файл");
	AppendMenuW(SubMenu, MF_STRING, OnLoadFile, L"Открыть");
	AppendMenuW(SubMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(SubMenu, MF_STRING, ExitProgamm, L"Выход");

	HMENU Instruments = CreateMenu();
	AppendMenuW(RootMenu, MF_POPUP, (UINT_PTR)Instruments, L"Обработка");
	AppendMenuW(Instruments, MF_STRING, SobelClick, L"Оператор Собеля");
	AppendMenuW(Instruments, MF_STRING, Average_3x3, L"Усреднение 3x3");
	AppendMenuW(Instruments, MF_STRING, Average_5x5, L"Усреднение 5x5");
	AppendMenuW(Instruments, MF_SEPARATOR, NULL, NULL);
	AppendMenuW(Instruments, MF_STRING, Subtract, L"Вычитание");
	SetMenu(hwnd, RootMenu);
}

void SetOpenFileParams(HWND hwnd)	// Функция для конфигурации окна открытия файла
{
	ZeroMemory(&ofn, sizeof(ofn)); // Зануление стркутуры
	ofn.lStructSize = sizeof(ofn);  // Указание размера структуры
	ofn.hwndOwner = hwnd; // Указание главного окна приложения
	ofn.lpstrFile = filename; // Указание буфера для хранения имени файла
	ofn.nMaxFile = sizeof(filename); // Указание длина буфера
	ofn.lpstrFilter = ".bmp"; // Указание типа файлов
	ofn.lpstrFileTitle = NULL; // Начальное имя файла
	ofn.nMaxFileTitle = 0; // Длина начального имени
	ofn.lpstrInitialDir = "D:\\"; // Начальный путь
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;	// Дополнительные примечания для пути
}

void Subtract_1ch()		// Функция вычитания из исходного изображения размытого изображения
{
	delete[] buffer_ch;
	delete[] buffer_adv;
	delete[] buffer_result;
	buffer_ch = new BYTE[INFOHEADER->biWidth * INFOHEADER->biHeight];
	for (int i = 0, j = 0; i < INFOHEADER->biWidth * INFOHEADER->biHeight; i++, j += 3)
	{
		buffer_ch[i] = buffer[j];
	}

	buffer_adv = new BYTE[(INFOHEADER->biWidth + 3) * (INFOHEADER->biHeight + 3)];
	ZeroMemory(buffer_adv, (INFOHEADER->biWidth + 3) * (INFOHEADER->biHeight + 3));
	for (int i = 0; i < INFOHEADER->biHeight; i++)			
	{
		for (int j = 0; j < INFOHEADER->biWidth; j++)
		{
			buffer_adv[(i + 2) * INFOHEADER->biWidth + (j + 2)] = buffer_ch[i * INFOHEADER->biWidth + j];
		}
	}
	buffer_result = new BYTE[INFOHEADER->biWidth * INFOHEADER->biHeight];
	for (int i = 2; i < INFOHEADER->biHeight + 2; i++)
	{
		for (int j = 2; j < INFOHEADER->biWidth + 2; j++)
		{
			double conv = Avg_5x5[0][0] * buffer_adv[(i - 2) * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[0][1] * buffer_adv[(i - 2) * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[0][2] * buffer_adv[(i - 2) * INFOHEADER->biWidth + j] + Avg_5x5[0][3] * buffer_adv[(i - 2) * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[0][4] * buffer_adv[(i - 2) * INFOHEADER->biWidth + (j + 2)] +
				Avg_5x5[1][0] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[1][1] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[1][2] * buffer_adv[(i - 1) * INFOHEADER->biWidth + j] + Avg_5x5[0][3] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[0][4] * buffer_adv[(i - 1) * INFOHEADER->biWidth + (j + 2)] +
				Avg_5x5[2][0] * buffer_adv[i * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[2][1] * buffer_adv[i * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[2][2] * buffer_adv[i * INFOHEADER->biWidth + j] + Avg_5x5[2][3] * buffer_adv[i * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[2][4] * buffer_adv[i * INFOHEADER->biWidth + (j + 2)] +
				Avg_5x5[3][0] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[3][1] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[3][2] * buffer_adv[(i + 1) * INFOHEADER->biWidth + j] + Avg_5x5[3][3] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[3][4] * buffer_adv[(i + 1) * INFOHEADER->biWidth + (j + 2)] +
				Avg_5x5[4][0] * buffer_adv[(i + 2) * INFOHEADER->biWidth + (j - 2)] + Avg_5x5[4][1] * buffer_adv[(i + 2) * INFOHEADER->biWidth + (j - 1)] + Avg_5x5[4][2] * buffer_adv[(i + 2) * INFOHEADER->biWidth + j] + Avg_5x5[4][3] * buffer_adv[(i + 2) * INFOHEADER->biWidth + (j + 1)] + Avg_5x5[4][4] * buffer_adv[(i + 2) * INFOHEADER->biWidth + (j + 2)];
			double res = conv / 25;
			buffer_result[(i - 2) * INFOHEADER->biWidth + (j - 2)] = res;
		}
	}
	for (int i = 0, j = 0; i < INFOHEADER->biWidth * INFOHEADER->biHeight; i++, j += 3)
	{
		for (int k = 0; k < 3; k++)
		{
			buffer[3 * i + k] = buffer_ch[i] - buffer_result[i] + 100;
			if (buffer[3 * i + k] < 0) buffer[3 * i + k] = 0;
			if (buffer[3 * i + k] > 255) buffer[3 * i + k] = 255;
		}
	}
}

