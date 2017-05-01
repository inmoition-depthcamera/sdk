//Cross-platform console controller class written by Alex Gittemeier and Miles Fogle
//Handles input, output, text coloring, and waiting/time controls
//Targets Windows and POSIX systems
//

#ifndef CONSOLECONTROLLER_H_INCLUDED
#define CONSOLECONTROLLER_H_INCLUDED

//General includes
#include <ctime>   //temporal methods
#include <string>  //input and output
#include <sstream> //output

#ifdef _WIN32
//Windows-specific includes
#include <Windows.h>
#include <conio.h>
#include <iostream>

//Color flags to simulate a POSIX-like color implementation
enum Colors {
    COLOR_BLACK,  COLOR_RED,     COLOR_GREEN, COLOR_BLUE,
    COLOR_YELLOW, COLOR_MAGENTA, COLOR_CYAN,  COLOR_WHITE
};

//special (non-ascii) key codes
//obviously this is not fully implemented
enum SpecialKeyCodes {
    ARROW_LEFT  = 0x00000100,
    ARROW_RIGHT = 0x00000101,
    ARROW_DOWN  = 0x00000102,
    ARROW_UP    = 0x00000103
};

#else

#include <curses.h>

#endif // _WIN32


class ConsoleController {
    public:

        // Define types
        typedef unsigned char COLOR_ID;

        // Setup and teardown
        ConsoleController();
        ~ConsoleController();
        void initColor(COLOR_ID colorId, int fg, int bg, bool fBold, bool bBold);

        // Accessors
		COORD getWindowSize();
		COORD getCurPos();
        void getWindowSize(int *x, int *y);
        void getCurPos(int *x, int *y);

		// Cursor Show/Hide
		void ShowCursor(bool show_hide);

        // Actions
        void cls();
        void moveCursor(int x, int y);
        void moveCursor(COORD pos);
        void color(COLOR_ID);

        //fix for the commonly distributed GCC bug with std::to_string()
        template <typename TYPE>
        std::string toString(TYPE t) {
            std::stringstream ss;
            ss << t;
            return ss.str();
        }

        // Output
        void output(std::string s);

        template <typename TYPE>
        void output(TYPE t) {
#ifdef _WIN32 //slight performance improvement for Windows
            std::cout << t; //avoids expensive stringstream construction
#else
            output(toString(t));
#endif // _WIN32
        }

        template <typename TYPE>
        void output(COLOR_ID c, TYPE t) {
            color(c);
            output(t);
        }

        template <typename TYPE>
        void output(int x, int y, TYPE t) {
            moveCursor(x, y);
            output(t);
        }

        template <typename TYPE>
        void output(COORD pos, TYPE t) {
            moveCursor(pos);
            output(t);
        }

        template <typename TYPE>
        void output(int x, int y, COLOR_ID c, TYPE t) {
            moveCursor(x, y);
            color(c);
            output(t);
        }

        template <typename TYPE>
        void output(COORD pos, COLOR_ID c, TYPE t) {
            moveCursor(pos);
            color(c);
            output(t);
        }

        // Input
        int getKey();
        int waitForKey();
        int waitForNewKey();
        void waitForKey(int match);
        int echoKey();
        int waitForChar();

        std::string waitForInput();
        std::string waitForInput(char delimiter);
        std::string waitForInput(std::string delimiter);

        // Type-specific input
        template <typename TYPE>
        TYPE waitForInput(std::string delimiters) {
            std::string str(waitForInput(delimiters));
            std::stringstream ss;
            TYPE result;

            ss << str;
            ss >> result;
            return result;
        }

        template <typename TYPE>
        TYPE waitForInput(char delimiter) {
            return waitForInput<TYPE>(std::string(1, delimiter));
        }

        template <typename TYPE>
        TYPE waitForInput() {
            return waitForInput<TYPE>("\n");
        }

        // Temporal methods (consider moving to a different file)
        void sleepMs(long ms);
        void throttle(long ms);

        // Utility methods
        void clearKey();
        void pause();

        // Operator overloads
        template<typename TYPE>
        ConsoleController& operator<< (TYPE t) {
            output(t);
            return *this;
        }

        template<typename TYPE>
        ConsoleController& operator>> (TYPE& t) {
            t = waitForInput<TYPE>();
            return *this;
        }


    private:
        // Define types
        struct COLOR {
            short foreground, background;
            bool foreBold, backBold;
        };

        // Fields
        clock_t throttleLast;
        bool throttleInitialized;
        static int classInstances;

        // Disallow copying and assigning over the object (do not implement these methods)
        ConsoleController(const ConsoleController &);
        ConsoleController & operator= (const ConsoleController &);

        //color values made static for compatibility with POSIX implementation
#ifdef _WIN32
        // Windows specific fields
        static HANDLE hStdout;
        static COLOR colors[256];
#else
        // POSIX specific fields
        static bool foreBoldFlags[256];
#endif
};

//declare a static instance available everywhere, similar to std::cout
static ConsoleController con;

#endif // CONSOLECONTROLLER_H_INCLUDED
