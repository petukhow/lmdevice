#pragma once
#include "Key.h"
#include "WString.h"
#include "esp32-hal.h"
#include <Keypad.h>
#include <Arduino.h>
#include <cstring>

constexpr byte ROWS = 4;
constexpr byte COLS = 4;

constexpr char keys[ROWS][COLS] = {
    {'1', '2','3','\n'},   // .,!?1 ABC2 DEF3  ENTER
    {'4','5','6','\x01'},  // GHI4 JKL5 MNO6   RIGHT
    {'7','8','9','\x02'},  // PQRS7 TUV8 WXYZ9 LEFT
    {'0','\b',' ',' '}     // SPACE0 BACKSPACE (last 2 are empty yet)
};

byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const char* keymap(int digit) {
    switch (digit) {
        case '1': return ".,!?";
        case '2': return "abc";
        case '3': return "def";
        case '4': return "ghi";
        case '5': return "jkl";
        case '6': return "mno";
        case '7': return "pqrs";
        case '8': return "tuv";
        case '9': return "wxyz";
        case '0': return " ";
        default:  return "";
    }
}

class T9Input {
public:
    String buffer = "";
    int cursorPos = 0;

    void update() {
        keypad.getKeys();
        unsigned long now = millis();

        if (_pendingDigit != 0 && (now - _lastPressTime > TAP_TIMEOUT_MS)) {
            _commitPendingButton();
        }

        for (int i = 0; i < LIST_MAX; i++) {
            if (keypad.key[i].stateChanged && keypad.key[i].kstate == PRESSED) {
                char key = keypad.key[i].kchar;
                _handleKey(key, now);
            }
        }
        
    }

    bool wasEnterPressed() {
        if (_isEnterPressed) {
            _isEnterPressed = false;
            return true;
        }
        return false;
    }

    void clear() {
        _pendingDigit = 0;
        _tapCount = 0;
        cursorPos = 0;
        buffer = "";
    }

private:
    static const unsigned long TAP_TIMEOUT_MS = 2000;
    char _pendingDigit = 0;
    unsigned int _tapCount = 0;
    unsigned int _lastPressTime = 0;
    bool _isEnterPressed = false;

    void _commitPendingButton() {
        if (_pendingDigit == 0) return;
        const char* options = keymap(_pendingDigit);

        int len = strlen(options);
        if (len > 0) {
            char c = options[_tapCount % len];
            buffer = buffer.substring(0, cursorPos) + String(c) + buffer.substring(cursorPos);
            cursorPos++;
        }
        
        _pendingDigit = 0;
        _tapCount = 0;
    }

    void _handleKey(char key, unsigned long now) {
        if (!key) return;

        if (key == '\n') {
            _commitPendingButton();
            _isEnterPressed = true;
            return;
        }

        if (key == '\b') {
            _commitPendingButton();
            if (cursorPos > 0) {
                buffer.remove(cursorPos-1, 1);
                cursorPos--;
            }
            return;
        }

        if (key == '\x01') {
            _commitPendingButton();
            if (cursorPos < buffer.length()) cursorPos++;
            return;
        }

        if (key == '\x02') {
            _commitPendingButton();
            if (cursorPos > 0) cursorPos--;
            return;
        }

        if (key == _pendingDigit && (now - _lastPressTime <= TAP_TIMEOUT_MS)) {
            _tapCount++;
        } else {
            _commitPendingButton();
            _pendingDigit = key;
            _tapCount = 0;
        }

        _lastPressTime = now;
    }
};