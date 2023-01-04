#include <windows.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>


#define FILE_MENU_NEW 1
#define FILE_MENU_OPEN 2
#define FILE_MENU_EXIT 3
#define N_ENTERED 4
#define MATRIX_ENTERED 5
#define MAXSIZE 9

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

void AddMenus(HWND);
void AddControls(HWND);
void AddMatrixControls(HWND);
bool gaujorInv();
void displayMatrix(HWND, long int, bool);

float matrix[MAXSIZE][MAXSIZE * 2];
HWND hN; // edit handlers
std::vector<HWND> hMatrix;
std::vector<HWND> hInv;
HMENU hMenu;
int n = 0;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {

	WNDCLASSW wc = { 0 };

	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInst;
	wc.lpszClassName = L"myWindowClass";
	wc.lpfnWndProc = WindowProcedure;

	if (!RegisterClassW(&wc)) {
		return -1;
	}

	CreateWindowW(L"myWindowClass", L"Gauss Jordan Inverse App", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 1500, 1500, NULL, NULL, NULL, NULL);
	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, NULL, NULL)) { // start edit from here
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;

}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) { // msg constantly passed
	case WM_COMMAND: // some control pressed

		switch (wp)
		{
		case FILE_MENU_EXIT:
			DestroyWindow(hWnd);
			break;
		case FILE_MENU_NEW:
			MessageBeep(MB_ICONINFORMATION);
			break;
		case N_ENTERED:
			// get entered matrix dimension

			// reset all windows
			for (int i = 0; i < hMatrix.size(); i++) {
				DestroyWindow(hMatrix[i]);
			}
			for (int i = 0; i < hInv.size(); i++) {
				DestroyWindow(hInv[i]);
			}
			hMatrix.clear();
			hInv.clear();

			wchar_t num[2];
			GetWindowTextW(hN, num, 2);
			n = wcstol(num, 0, 10);
			AddMatrixControls(hWnd);
			break;
		case MATRIX_ENTERED:
			// get entered matrix (invalid entry become 0)

			// zero out the matrix and reset all windows
			for (int i = 0; i < MAXSIZE; i++) {
				for (int j = 0; j < MAXSIZE; j++) {
					matrix[i][j] = 0;
				}
			}

			for (int i = 0; i < hInv.size(); i++) {
				DestroyWindow(hInv[i]);
			}
			hInv.clear();
			
			wchar_t entry[100];
			for (int i = 0; i < n * n; i++) {
				GetWindowTextW(hMatrix[i], entry, 100);
				matrix[i / n][i % n] = wcstof(entry, NULL);
			}
			auto start = std::chrono::high_resolution_clock::now();
			bool singular = gaujorInv();
			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
			displayMatrix(hWnd, duration.count(), singular);

		}
		break;
	case WM_CREATE: // window created
		AddMenus(hWnd); // add menus
		AddControls(hWnd); //add controls
		break;
	case WM_DESTROY: // window destroyed
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}
}


// Add windows menu
void AddMenus(HWND hWnd) {
	hMenu = CreateMenu();
	HMENU hFileMenu = CreateMenu();
	HMENU hSubMenu = CreateMenu();


	// under file
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_NEW, L"New");
	AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFileMenu, MF_STRING, FILE_MENU_EXIT, L"Exit");

	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
	AppendMenu(hMenu, MF_STRING, NULL, L"Help");

	SetMenu(hWnd, hMenu);
}

// Initial input box
void AddControls(HWND hWnd) {
	// text box
	CreateWindowW(L"static", L"Enter n for n by n matrix (1 - 9): ", WS_VISIBLE | WS_CHILD | SS_CENTER, 600, 100, 300, 25, hWnd, NULL, NULL, NULL);
	// input text
	hN = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER , 600, 130, 300, 25, hWnd, NULL, NULL, NULL);
	// button
	CreateWindowW(L"Button", L"Enter", WS_VISIBLE | WS_CHILD, 600, 160, 300, 50, hWnd, (HMENU)N_ENTERED, NULL, NULL);
}

// add input matrix
void AddMatrixControls(HWND hWnd) {
	HWND hM;
	if (n == 0) {
		hM = CreateWindowW(L"static", L"Invalid n please try again.", WS_VISIBLE | WS_CHILD, 100, 350, 200, 50, hWnd, NULL, NULL, NULL);
		hMatrix.push_back(hM);
	} else {
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				hM = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 100 + j * 60, 400 + i * 40, 50, 25, hWnd, NULL, NULL, NULL);
				hMatrix.push_back(hM);
			}
		}
		hM = CreateWindowW(L"static", L"Enter the Matrix: ", WS_VISIBLE | WS_CHILD, 100, 350, 200, 50, hWnd, NULL, NULL, NULL);
		hMatrix.push_back(hM);
		hM = CreateWindowW(L"Button", L"Enter", WS_VISIBLE | WS_CHILD, (70 + n * 60) / 2, 440 + 40 * n, 100, 50, hWnd, (HMENU)MATRIX_ENTERED, NULL, NULL);
		hMatrix.push_back(hM);
	}

 }

// display inverse matrix
void displayMatrix(HWND hWnd, long int dur, bool sing) {
	HWND win;
	if (sing) {
		win = CreateWindowW(L"static", L"ERROR: Matrix is singular. ", WS_VISIBLE | WS_CHILD, 800, 350, 200, 50, hWnd, NULL, NULL, NULL);
		hInv.push_back(win);
	}
	else {
		win = CreateWindowW(L"static", L"The Inverse Matrix: ", WS_VISIBLE | WS_CHILD, 800, 350, 200, 50, hWnd, NULL, NULL, NULL);
		hInv.push_back(win);
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				float value = std::round(matrix[i][j + n] * 100.0) / 100.0;
				std::wstringstream wss;
				wss << value;
				win = CreateWindowW(L"static", wss.str().c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER, 800 + j * 60, 400 + i * 40, 50, 25, hWnd, NULL, NULL, NULL);
				hInv.push_back(win);
			}
		}
		std::wstringstream wss;
		wss << L"This took: ";
		wss << dur;
		wss << "ms";
		win = CreateWindowW(L"static", wss.str().c_str(), WS_VISIBLE | WS_CHILD, 800, 440 + 40 * n, 500, 50, hWnd, (HMENU)MATRIX_ENTERED, NULL, NULL);
		hInv.push_back(win);
	}
	
}


bool gaujorInv() {
	// augment In matrix
	for (int i = 0; i < n; i++) {
		matrix[i][i + n] = 1;
	}

	// Gauss Jordan Elimination
	for (int i = 0; i < n; i++) {
		if (matrix[i][i] == 0) {
			int skip = 1;
			while ((i + skip) < n && matrix[i + skip][i] == 0) {
				skip++;
			}
			if ((i + skip == n)) {
				break;
			}
			for (int k = 0; k < 2 * n; k++) {
				float temp = matrix[i][k];
				matrix[i][k] = matrix[i + skip][k];
				matrix[i + skip][k] = temp;
			}
		}

		for (int j = 0; j < n; j++) {
			if (i != j) {
				float rate = matrix[j][i] / matrix[i][i];
				for (int a = 0; a < 2 * n; a++) {
					matrix[j][a] = matrix[j][a] - matrix[i][a] * rate;
				}
			}
		}
	}
	for (int i = 0; i < n; i++) {
		float factor = matrix[i][i];
		for (int j = 0; j < 2 * n; j++) {
			matrix[i][j] = matrix[i][j] / factor;
		}
	}

	// Check if the matrix is singular
	if (matrix[n-1][n-1] == 1) {
		return false;
	} else {
		return true;
	}
}