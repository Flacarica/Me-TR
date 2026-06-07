/*

            $$\      $$\                   $$$$$$$$\  $$$$$$$\
            $$$\    $$$ |                  \__$$ \__| $$  __$$\
            $$$$\  $$$$ |  $$$$$$\            $$ |    $$ |  $$ |
            $$\$$\$$ $$ | $$  __$$\  $$$$$$\  $$ |    $$$$$$$  |
            $$ \$$$  $$ | $$$$$$$$ | \_____\| $$ |    $$  __$$<
            $$ |\$  /$$ | $$|_____\|          $$ |    $$ |  $$ |  
            $$ | \_/ $$ | \$$$$$$$\           $$ |    $$ |  $$ |
            \_\|     \_\|  \______\|          \_\|    \_\|  \_\|


            52 px x 8 px
*/

#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>

using namespace std;

char logo[8][64] = {
    "$$\\      $$\\                    $$$$$$$$\\  $$$$$$$\\  ",
    "$$$\\    $$$ |                   \\__$$ \\__| $$  __$$\\ ",
    "$$$$\\  $$$$ |  $$$$$$\\             $$ |    $$ |  $$ |",
    "$$\\$$\\$$ $$ | $$  __$$\\  $$$$$$\\   $$ |    $$$$$$$  |",
    "$$ \\$$$  $$ | $$$$$$$$ | \\_____\\|  $$ |    $$  __$$< ",
    "$$ |\\$  /$$ | $$|_____\\|           $$ |    $$ |  $$ |",
    "$$ | \\_/ $$ | \\$$$$$$$\\            $$ |    $$ |  $$ |",
    "\\_\\|     \\_\\|  \\______\\|           \\_\\|    \\_\\|  \\_\\| "
};

void gotoxy(int x, int y) {
	// Fie coord de tip COORD
    COORD coord;
    
	// Setam coordonatele
    coord.X = x;
    coord.Y = y;
    
	// Mutam cursorul la coord
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
	// Obtinem informatiile despre cursor
    CONSOLE_CURSOR_INFO cursorInfo;

	// Setam vizibilitatea cursorului la false
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

	// Ascundem cursorul
    cursorInfo.bVisible = false;

	// Aplicam modificarile
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void SetConsoleSize(int width, int height) {
	// Obtinem handle-ul pentru consola
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Cream un COORD pentru a specifica dimensiunea buffer-ului
    COORD bufferSize = { (SHORT)width, (SHORT)height };

	// Setam dimensiunea buffer-ului
    SetConsoleScreenBufferSize(hConsole, bufferSize);

	// Cream un dreptunghi pentru a specifica dimensiunea ferestrei
    SMALL_RECT rect = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };

    // Setam dimensiunea ferestrei
    SetConsoleWindowInfo(hConsole, TRUE, &rect);
}

void WriteCharAt(int x, int y, wchar_t ch) {
	// Mutam cursorul la pozitia specificata
    gotoxy(x, y);

	// Obtinem handle-ul pentru consola de iesire
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// Convertim caracterul la formatul necesar pentru WriteConsoleW
    DWORD written = 0;

	// Scriem caracterul la pozitia specificata
    WriteConsoleW(hStdOut, &ch, 1, &written, NULL);
}

void PrintLogo(int a, int b) {
    // Parcurge matricea si printeaza logo-ul
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 53; y++) {
            gotoxy(y + a, x + b);
            cout << logo[x][y];
        }
    }
}

void PrintDetailTable(int a, int b) {
	// Muchia de sus
	WriteCharAt(a, b, L'╔');
    for (int i = 1; i < 60; i++) {
        WriteCharAt(a + i, b, L'═');
    }
	WriteCharAt(a + 60, b, L'╗');
    
	// Muchiile laterale
    for (int i = 1; i < 20; i++) {
        WriteCharAt(a, b + i, L'║');
        WriteCharAt(a + 60, b + i, L'║');
	}
    
	// Muchia de jos
    WriteCharAt(a, b + 20, L'╚');
    for (int i = 1; i < 60; i++) {
        WriteCharAt(a + i, b + 20, L'═');
	}
	WriteCharAt(a + 60, b + 20, L'╝');
}

void PrintOptionsTable(int a, int b) {
    // Muchia de sus
    WriteCharAt(a, b, L'╔');
    for (int i = 1; i < 60; i++) {
        WriteCharAt(a + i, b, L'═');
    }
    WriteCharAt(a + 60, b, L'╗');
    
	// Muchiile laterale
    for (int i = 1; i < 32; i++) {
        WriteCharAt(a, b + i, L'║');
        WriteCharAt(a + 60, b + i, L'║');
    }
    
	// Muchia de jos
    WriteCharAt(a, b + 32, L'╚');
    for (int i = 1; i < 60; i++) {
        WriteCharAt(a + i, b + 32, L'═');
    }
    WriteCharAt(a + 60, b + 32, L'╝');
}

void PrintOption(int a, int b, bool highlight, int index) {
	// Lista cu optiuni
    char option[9][17] = { 
        {"1. Temperature"},
        {"2. Pressure"},
        {"3. UV Index"},
        {"4. Air Humidity"},
        {"5. Soil Humidity"},
        {"6. Air Speed"},
        {"7. Air Direction"},
		{"8. Rainfall"},
        {"9. LoRa"}
    };

	// Printam optiunea la pozitia specificata, evidentiind-o daca este necesar
    gotoxy(a, b);
    if (highlight) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_RED | BACKGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << option[index - 1];
    } else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        cout << option[index - 1];
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

void UpdateDetails(int index) {
    // Lista cu detalii
    char details[9][50] = {
        {"Temperature: 25°C"},
        {"Pressure: 1013 hPa"},
        {"UV Index: 5"},
        {"Air Humidity: 60%"},
        {"Soil Humidity: 40%"},
        {"Air Speed: 15 km/h"},
        {"Air Direction: NE"},
        {"Rainfall: 0 mm"},
		{"LoRa: Connected" }
    };

    // Curatam
    gotoxy(4, 16);
    cout << "                    ";

    // Si printam
    gotoxy(4, 16);
    cout << details[index - 1];
}

void DrawInterface() {
    // Dimensiune si culoare
	SetConsoleSize(140, 40);    // Setam marimea 
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);     // Text portocaliu
	hideCursor();   // Ascundem cursorul
	PrintLogo(6, 4);    // Printam logo-ul

	// Tabele
	PrintDetailTable(2, 14);    // Printam tabelul pentru detalii
    gotoxy(28, 15);     
	cout << "Details:";     // Titlu pentru tabelul de detalii

	PrintOptionsTable(65, 2);   // Printam tabelul pentru optiuni
    gotoxy(92, 3);
    cout << "Options:";     // Titlu pentru tabelul de optiuni

	// Optiuni
	PrintOption(67, 5, true, 1);        // Afisam prima optiune si o evidentiem
	UpdateDetails(1);                   // Afisam detaliile pentru prima optiune
	PrintOption(67, 8, false, 2);       // Afisam a doua optiune
	PrintOption(67, 11, false, 3);      // ...
    PrintOption(67, 14, false, 4);
    PrintOption(67, 17, false, 5);
    PrintOption(67, 20, false, 6);
    PrintOption(67, 23, false, 7);
	PrintOption(67, 26, false, 8);      // ...
	PrintOption(67, 29, false, 9);      // Afisam ultima optiune
    
	//              x ,  y ,    bool  ,  int
    // PrintOption(row, col, highlight, index);
}

void RedrawLine(int& x, int& y, int& index, bool direction) {
	// Stergem linia curenta si o redesenam fara evidentiere
    gotoxy(x, y);
    for (int i = 0; i < 20; i++) cout << " ";
    PrintOption(x, y, false, index);

	// Actualizam pozitia si indexul in functie de directie
    if (direction) {
        if (index > 1) {
            y -= 3; // sus 3 randuri
            index--;
        }
    } else {
        if (index < 9) {
            y += 3; // jos 3 randuri
            index++;
        }
    }

	// Evidentiem noua linie
    PrintOption(x, y, true, index);
    UpdateDetails(index);
}

int main()
{
	DrawInterface();        // Desenam interfata
	int x = 67, y = 5;      // Pozitia primei optiuni
    gotoxy(x, y);
	int index = 1;          // Setam index la 1

    while (true) {          // Bucla principala
		if (_kbhit()) {     // Verificam daca a fost apasata o tasta
			int key = _getch();     // Citim tasta apasata
			if (key == 224) {       // Tasta speciala (sageti)
                key = _getch();
                if (key == 72) { // Sus
					RedrawLine(x, y, index, true);  // Redesenam linia pentru a evidentia optiunea curenta
                }
                else if (key == 80) { // Jos
                    RedrawLine(x, y, index, false);  // Redesenam linia pentru a evidentia optiunea curenta
                }
            }
            else if (key == 27) { // ESC pentru a iesi
                break;
            }
        }
    }
	system("cls");
    return 0;
}