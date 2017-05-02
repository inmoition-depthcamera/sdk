//Cross-platform console controller class written by Alex Gittemeier and Miles Fogle
//Handles input, output, text coloring, and waiting/time controls
//Targets Windows and POSIX systems
//

#include "console_controller.h"

//static init
int ConsoleController::classInstances = 0;

#ifdef _WIN32
HANDLE ConsoleController::hStdout;
ConsoleController::COLOR ConsoleController::colors[256];
#else
bool ConsoleController::foreBoldFlags[256];
#endif // _WIN32

//local functions
#ifndef _WIN32
int posix_translateKey();
#endif // _WIN32

/////////////////////////////////////////////////

ConsoleController::ConsoleController() {
	// Init data members
	throttleInitialized = false;

	// Only setup the console on the first allocation
	// since this must be done only once until the last destruction
	if (classInstances == 0) {
#ifdef _WIN32
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
#else
		initscr();
		cbreak();
		noecho();
		keypad(stdscr, true);
		start_color();
#endif
		cls();

		//moved inside here since these are now static
		for (int i = 0; i < 256; i++) {
#ifdef _WIN32
            colors[i] = {0, 0, false, false};
#else
            foreBoldFlags[i] = false;
#endif
        }
    }

    ++classInstances;
}

ConsoleController::~ConsoleController(void) {
	--classInstances;
	if (classInstances == 0) {
		cls();

#ifndef _WIN32
		endwin();
#endif
	}
}

void ConsoleController::initColor(COLOR_ID colorId, int fg, int bg, bool fBold, bool bBold) {
#ifdef _WIN32
	COLOR newColor = { (SHORT)fg, (SHORT)bg, fBold, bBold};
    colors[colorId] = newColor;
#else
	if (bBold)
        init_pair(colorId + 1, fg, bg + 8); // +8 is for adding the bold flag
    else
        init_pair(colorId + 1, fg, bg);
    foreBoldFlags[colorId] = fBold;
#endif
}

/////////////////////////////////////////////////

COORD ConsoleController::getWindowSize() {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD size;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	size.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	size.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	return size;
#else
	return {COLS, LINES};
#endif
}

COORD ConsoleController::getCurPos() {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD pos;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	pos.X = csbi.dwCursorPosition.X;
	pos.Y = csbi.dwCursorPosition.Y;

	return pos;
#else
    return {getcurx(stdscr), getcury(stdscr)};
	//TODO: implement under posix
#endif
}

void ConsoleController::getWindowSize(int *x, int *y) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	*x = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	*y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

#else
	*x = COLS;
	*y = LINES;
#endif
}

void ConsoleController::getCurPos(int *x, int *y) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	*x = csbi.dwCursorPosition.X;
	*y = csbi.dwCursorPosition.Y;
#else
	*x = getcurx(stdscr);
	*y = getcury(stdscr);
#endif
}

void ConsoleController::ShowCursor(bool show_hide)
{
#ifdef _WIN32
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CursorInfo;
	GetConsoleCursorInfo(handle, &CursorInfo);
	CursorInfo.bVisible = show_hide;
	SetConsoleCursorInfo(handle, &CursorInfo); 
#else
	if(!show_hide)
		printf("\033[?25l");
	else
		printf("\033[?25h");
#endif
}

/////////////////////////////////////////////////

void ConsoleController::cls() {
#ifdef _WIN32
	system("cls");
#else
	clear();
#endif
}

void ConsoleController::moveCursor(int x, int y) {
#ifdef _WIN32
	COORD position = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hStdout, position);
#else
	move(y, x);
#endif
}

void ConsoleController::moveCursor(COORD pos) {
    moveCursor(pos.X, pos.Y);
}

//TODO: make the windows side of this function not unnecessarily ugly
void ConsoleController::color(COLOR_ID colorId) {
#ifdef _WIN32
	COLOR curColor = colors[colorId];
    int colorFlags = 0;

    //consider making a magic table for this

    if (curColor.foreBold)
        colorFlags |= FOREGROUND_INTENSITY;
    if (curColor.backBold)
        colorFlags |= BACKGROUND_INTENSITY;

    if (curColor.foreground == COLOR_MAGENTA ||
        curColor.foreground == COLOR_RED     ||
        curColor.foreground == COLOR_YELLOW  ||
        curColor.foreground == COLOR_WHITE     )
        colorFlags |= FOREGROUND_RED;
    if (curColor.foreground == COLOR_YELLOW  ||
        curColor.foreground == COLOR_GREEN   ||
        curColor.foreground == COLOR_CYAN    ||
        curColor.foreground == COLOR_WHITE     )
        colorFlags |= FOREGROUND_GREEN;
    if (curColor.foreground == COLOR_CYAN    ||
        curColor.foreground == COLOR_BLUE    ||
        curColor.foreground == COLOR_MAGENTA ||
        curColor.foreground == COLOR_WHITE     )
        colorFlags |= FOREGROUND_BLUE;

    if (curColor.background == COLOR_MAGENTA ||
        curColor.background == COLOR_RED     ||
        curColor.background == COLOR_YELLOW  ||
        curColor.background == COLOR_WHITE     )
        colorFlags |= BACKGROUND_RED;
    if (curColor.background == COLOR_YELLOW  ||
        curColor.background == COLOR_GREEN   ||
        curColor.background == COLOR_CYAN    ||
        curColor.background == COLOR_WHITE     )
        colorFlags |= BACKGROUND_GREEN;
    if (curColor.background == COLOR_CYAN    ||
        curColor.background == COLOR_BLUE    ||
        curColor.background == COLOR_MAGENTA ||
        curColor.background == COLOR_WHITE     )
        colorFlags |= BACKGROUND_BLUE;

    SetConsoleTextAttribute(hStdout, colorFlags);
#else
	if (foreBoldFlags[colorId])
        attrset(COLOR_PAIR(colorId + 1) | A_BOLD);
    else
        attrset(COLOR_PAIR(colorId + 1));
#endif
}

/////////////////////////////////////////////////

void ConsoleController::output(std::string s) {
#ifdef _WIN32
	std::cout << s;
#else
	printw("%s", s.c_str());
#endif
}

/////////////////////////////////////////////////

std::string ConsoleController::waitForInput() {
    return waitForInput("\n");
}

//TODO: look into making this SpecialKeyCode-proof
std::string ConsoleController::waitForInput(char delineator) {
    return waitForInput(std::string(1, delineator));
}

std::string ConsoleController::waitForInput(std::string delineators) {
    char input;
    std::string str;

    while (true) {
        //get cursor position preemptively to handle backspace
		COORD pos = getCurPos();
        input = echoKey(); //get each individual keystroke

        if (input == '\b') { //manually handle backspace
            if (!str.empty()) { //if there's anything to backspace
                str.pop_back(); //remove the last character
                //handle cursor movement
                if (pos.X == 0) { //if cursor is all the way to the left
                    //move it to the end of the row above
                    moveCursor(getWindowSize().X - 1, pos.Y - 1);
                } else {
                    moveCursor(pos.X - 1, pos.Y); //move it back one space
                }
            } else {
                //the console auto-moves the cursor after a backspace
                //which is super unhelpful because we have to re-move it
                //back to its original position in this case
                moveCursor(pos.X, pos.Y);
            }
            //finally, clear out the character wherever the cursor is
            pos = getCurPos();
            output(' ');
            moveCursor(pos.X, pos.Y);
        } else {
            //if it matches any of the delineating characters,
            //break the input cycle
            for (size_t i = 0; i < delineators.length(); ++i)
                if (input == delineators.at(i))
                    goto soSueMe; //SO FUCKING SUE ME
            //otherwise, append the recent character
            str += input;
        }
    }
    soSueMe:

    //continue taking and ignoring characters until a newline
    if (input != '\n') {
        while (echoKey() != '\n');
    }

    return str;
}

int ConsoleController::getKey() {
#ifdef _WIN32
	if (_kbhit())
        return waitForKey();
    return 0;
#else
    timeout(0); // Set non-blocking mode
    return posix_translateKey();
#endif
}

int ConsoleController::waitForKey() {
#ifdef _WIN32
	char result = _getch();
	if (result == '\r')
		return '\n';
	// see: http://stackoverflow.com/q/10463201
	if (result == 0 || result == 0xE0) {
		switch (_getch()) {
            case 0x41:
            case 0x48:
                return ARROW_UP;
            case 0x42:
            case 0x50:
                return ARROW_DOWN;
            case 0x43:
            case 0x4D:
                return ARROW_RIGHT;
            case 0x44:
            case 0x4B:
                return ARROW_LEFT;
		}
	}

	return result;
#else
    timeout(-1); // Set blocking mode
    return posix_translateKey();
#endif
}

int ConsoleController::waitForNewKey() {
    clearKey();
    return waitForKey();
}

//POSIX-only
#ifndef _WIN32
int posix_translateKey() {
	int ch = getch();
    if (ch == ERR) return 0;
	if (ch == '\r') return '\n';
    return 0;
}
#endif //_WIN32

void ConsoleController::waitForKey(int match) {
    while (waitForKey() != match);
}

int ConsoleController::echoKey() {
    int i = waitForKey();
    output((char)i);
    return i;
}

/////////////////////////////////////////////////

void ConsoleController::sleepMs(long ms) {
#ifdef _WIN32
	Sleep(ms);
#else
	timespec sleepFor = { ms / 1000, ms % 1000 * 1000000 };
	nanosleep(&sleepFor, NULL);
#endif
}

void ConsoleController::throttle(long ms) {
	if (throttleInitialized) {
		throttleLast += (clock_t)(ms / 1000.0 * CLOCKS_PER_SEC);
		// throttleLast is now the next throttle point until we sleep
		if (throttleLast > clock())
			sleepMs(throttleLast - clock());
	}
	else {
		throttleLast = clock();
		throttleInitialized = true;
	}
}

/////////////////////////////////////////////////

void ConsoleController::clearKey() {
	while (getKey());
}

void ConsoleController::pause() {
	clearKey();
    output("Press any key to continue . . .");
    waitForKey();
    output("\n");
}
