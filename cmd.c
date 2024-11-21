#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <conio.h>

#define MAX_INPUT 1024
#define MAX_PATH_LEN 512

// Глобальная переменная для текущего диска
char current_drive[3] = "C:";  // Начинаем с диска C:

// Функция для установки кодировки UTF-8
void set_utf8_encoding() {
    system("chcp 65001 > nul");  // Устанавливаем кодировку UTF-8 для консоли
    SetConsoleOutputCP(CP_UTF8);  // Устанавливаем кодировку UTF-8 для вывода
    SetConsoleCP(CP_UTF8);        // Устанавливаем кодировку UTF-8 для ввода
}

// Функция для установки цвета текста
void set_color(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Функция для отображения текущего пути и имени пользователя
void print_prompt() {
    char cwd[MAX_PATH_LEN];
    char username[256];
    DWORD len = sizeof(username);

    // Получаем имя пользователя
    GetUserNameA(username, &len);

    // Получаем текущую директорию
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        // Печатаем диск, путь и имя пользователя
        set_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);  // Синий для диска
        printf("%s", current_drive);
        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);  // Зеленый для пути
        printf("%s", cwd + 2);  // Пропускаем 'C:'
        set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);  // Красный для имени пользователя
        printf("@%s", username);

        // Печатаем знак доллара с обычным цветом
        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Белый цвет
        printf("$ ");
    } else {
        perror("Ошибка получения текущей директории");
    }
}

// Функция для смены текущего диска
void change_drive(char *drive) {
    if (drive != NULL && strlen(drive) == 2 && drive[1] == ':') {
        // Обновляем текущий диск
        current_drive[0] = drive[0];
        current_drive[1] = ':';
        current_drive[2] = '\0';
        
        // Переходим в корневую директорию на новом диске
        char path[MAX_PATH_LEN];
        snprintf(path, sizeof(path), "%c:\\", drive[0]);
        if (_chdir(path) == 0) {
            set_color(FOREGROUND_GREEN);
            printf("Перешли на диск %c: и в директорию %s\n", drive[0], path);
        } else {
            set_color(FOREGROUND_RED);
            printf("Не удалось перейти на диск %c:\n", drive[0]);
        }
    }
}

// Функция для реализации встроенной команды 'cd'
void change_directory(char *path) {
    if (_chdir(path) == 0) {
        set_color(FOREGROUND_GREEN);  // Зеленый для успешных сообщений
        printf("Текущая директория изменена на: %s\n", path);
    } else {
        set_color(FOREGROUND_RED);  // Красный для ошибок
        perror("Ошибка изменения директории");
    }
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  // Сбросим цвет обратно
}

// Функция для реализации встроенной команды 'ls' (по сути, это 'dir' на Windows)
void list_directory() {
    system("dir");
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  // Сбросим цвет обратно
}

// Функция для выполнения внешней команды с использованием cmd.exe /k
void run_external_command(char *command) {
    char cmd[MAX_INPUT];
    
    // Формируем строку для выполнения через cmd.exe
    char *options = strtok(NULL, " ");
    char optionBuffer[7][MAX_INPUT];
    int optionCount = 0;
    while (options != NULL && optionCount < 7) {
        strcpy(optionBuffer[optionCount], options);
        options = strtok(NULL, " ");
        optionCount++;
    }
    char optionString[MAX_INPUT] = "";
    for (int i = 0; i < optionCount; i++) {
        strcat(optionString, optionBuffer[i]);
        if (i < optionCount - 1) {
            strcat(optionString, " ");
        }
    }
    if (optionCount > 0) {
        snprintf(cmd, sizeof(cmd), "powershell -Command \"%s %s\"", command, optionString);
    } else {
        snprintf(cmd, sizeof(cmd), "powershell -Command \"%s\"", command);
    }

    // Выполняем команду через cmd.exe
    int result = system(cmd);
    if (result != 0) {
        set_color(FOREGROUND_RED);  // Красный для ошибок
        printf("Ошибка при выполнении команды: %s\n", command);
        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  // Сбросим цвет обратно
    }
}

// Функция для очистки экрана
void clear_screen() {
    system("cls");  // Это стандартная команда Windows для очистки экрана
    set_utf8_encoding();  // Восстанавливаем кодировку UTF-8 после очистки экрана
}

// Основная функция
int main() {
    printf("========== | VovanchikCMD | ==========");
    system("ver");
    printf("\n\nУстановка кодировки UTF-8...");
    set_utf8_encoding();  // Устанавливаем кодировку UTF-8
    printf("Кодировка UTF-8 установлена!\nРусский текст должен теперь отображатся нормально.\n\n");
    SetConsoleTitleA("VovanchikCMD");

    char input[MAX_INPUT];
    char *command;

    // Основной цикл командной строки
    while (1) {
        print_prompt();

        // Получаем ввод пользователя
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // Завершаем программу на EOF
        }

        // Убираем символ новой строки
        input[strcspn(input, "\n")] = 0;

        // Разбираем команду и аргументы
        command = strtok(input, " ");
        
        // Обработка встроенных команд
        if (command != NULL) {
            if (strcmp(command, "exit") == 0) {
                break;
            }
            else if (strcmp(command, "cd") == 0) {
                char *path = strtok(NULL, " ");
                if (path) {
                    change_directory(path);
                } else {
                    set_color(FOREGROUND_RED | FOREGROUND_GREEN);  // Желтый для предупреждений
                    printf("Необходимо указать путь.\n");
                    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  // Сбросим цвет обратно
                }
            }
            else if (strcmp(command, "ls") == 0) {
                list_directory();
            }
            else if (strcmp(command, "clear") == 0) {
                clear_screen();  // Очистка экрана
            }
            else if (strlen(command) == 2 && command[1] == ':') {
                change_drive(command);
            }
            else {
                // Если команда не встроенная, выполняем через cmd.exe
                run_external_command(input);
            }
        }
    }

    return 0;
}
