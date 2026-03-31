#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
#endif

struct Contact {
    int id;
    std::string firstName;
    std::string lastName;
    std::string middleName;
    std::string phone;
    std::string email;
    std::string address;
    std::string birthday;   // формат: ДД.ММ.ГГГГ
    std::string notes;
};

std::vector<Contact> contacts;
const std::string DATA_FILE = "contacts.dat";
int nextId = 1;


void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    std::cout << "\n  Нажмите Enter для продолжения...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Регистронезависимое сравнение подстрок
bool containsCI(const std::string& haystack, const std::string& needle) {
    std::string h = haystack, n = needle;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n) != std::string::npos;
}

std::string inputLine(const std::string& prompt) {
    std::cout << "  " << prompt;
    std::string val;
    std::getline(std::cin, val);
    return trim(val);
}

void escapeField(std::ostream& out, const std::string& s) {
    for (char c : s) {
        if (c == '|' || c == '\\' || c == '\n')
            out << '\\';
        out << c;
    }
}

std::string unescapeField(const std::string& s) {
    std::string result;
    bool esc = false;
    for (char c : s) {
        if (esc) { result += c; esc = false; }
        else if (c == '\\') esc = true;
        else result += c;
    }
    return result;
}

void saveContacts() {
    std::ofstream f(DATA_FILE);
    if (!f) {
        std::cerr << "  [ОШИБКА] Не удалось открыть файл для записи: " << DATA_FILE << "\n";
        return;
    }
    f << nextId << "\n";
    for (const auto& c : contacts) {
        f << c.id << "|";
        escapeField(f, c.firstName);   f << "|";
        escapeField(f, c.lastName);    f << "|";
        escapeField(f, c.middleName);  f << "|";
        escapeField(f, c.phone);       f << "|";
        escapeField(f, c.email);       f << "|";
        escapeField(f, c.address);     f << "|";
        escapeField(f, c.birthday);    f << "|";
        escapeField(f, c.notes);       f << "\n";
    }
}

std::vector<std::string> splitLine(const std::string& line, char delim) {
    std::vector<std::string> fields;
    std::string cur;
    bool esc = false;
    for (char c : line) {
        if (esc) { cur += c; esc = false; }
        else if (c == '\\') esc = true;
        else if (c == delim) { fields.push_back(unescapeField(cur)); cur.clear(); }
        else cur += c;
    }
    fields.push_back(unescapeField(cur));
    return fields;
}

void loadContacts() {
    std::ifstream f(DATA_FILE);
    if (!f) return; // Файл ещё не создан

    std::string line;
    if (std::getline(f, line)) {
        try { nextId = std::stoi(trim(line)); } catch (...) { nextId = 1; }
    }

    while (std::getline(f, line)) {
        if (trim(line).empty()) continue;
        auto fields = splitLine(line, '|');
        if (fields.size() < 9) continue;
        Contact c;
        try { c.id = std::stoi(fields[0]); } catch (...) { continue; }
        c.firstName  = fields[1];
        c.lastName   = fields[2];
        c.middleName = fields[3];
        c.phone      = fields[4];
        c.email      = fields[5];
        c.address    = fields[6];
        c.birthday   = fields[7];
        c.notes      = fields[8];
        contacts.push_back(c);
    }
}

void printContact(const Contact& c) {
    std::cout << "\n";
    std::cout << "  ┌─────────────────────────────────────────┐\n";
    std::cout << "  │  ID: " << std::left << std::setw(35) << c.id << "│\n";
    std::cout << "  ├─────────────────────────────────────────┤\n";
    auto row = [](const std::string& label, const std::string& val){
        std::string line = "  " + label + ": " + val;
        // Обрезаем если слишком длинное
        if (line.size() > 43) line = line.substr(0, 40) + "...";
        std::cout << "  │  " << std::left << std::setw(39) << (label + ": " + val) << "│\n";
    };
    row("Фамилия    ", c.lastName);
    row("Имя        ", c.firstName);
    row("Отчество   ", c.middleName);
    row("Телефон    ", c.phone);
    row("Email      ", c.email);
    row("Адрес      ", c.address);
    row("Дата рожд. ", c.birthday);
    row("Заметки    ", c.notes);
    std::cout << "  └─────────────────────────────────────────┘\n";
}

void printContactList(const std::vector<Contact*>& list) {
    if (list.empty()) {
        std::cout << "\n  Контакты не найдены.\n";
        return;
    }
    std::cout << "\n";
    std::cout << "  " << std::string(70, '-') << "\n";
    std::cout << "  " << std::left
              << std::setw(5)  << "ID"
              << std::setw(18) << "Фамилия"
              << std::setw(14) << "Имя"
              << std::setw(18) << "Телефон"
              << std::setw(20) << "Email"
              << "\n";
    std::cout << "  " << std::string(70, '-') << "\n";
    for (const auto* c : list) {
        std::string ln = c->lastName.size()  > 16 ? c->lastName.substr(0,13)+"..." : c->lastName;
        std::string fn = c->firstName.size() > 12 ? c->firstName.substr(0, 9)+"..." : c->firstName;
        std::string ph = c->phone.size()     > 16 ? c->phone.substr(0,13)+"..." : c->phone;
        std::string em = c->email.size()     > 18 ? c->email.substr(0,15)+"..." : c->email;
        std::cout << "  " << std::left
                  << std::setw(5)  << c->id
                  << std::setw(18) << ln
                  << std::setw(14) << fn
                  << std::setw(18) << ph
                  << std::setw(20) << em
                  << "\n";
    }
    std::cout << "  " << std::string(70, '-') << "\n";
    std::cout << "  Итого: " << list.size() << " контакт(ов)\n";
}

void addContact() {
    clearScreen();
    std::cout << "\n  ╔══════════════════════════════╗\n";
    std::cout << "  ║     ДОБАВИТЬ КОНТАКТ         ║\n";
    std::cout << "  ╚══════════════════════════════╝\n\n";

    Contact c;
    c.id         = nextId++;
    c.lastName   = inputLine("Фамилия        : ");
    c.firstName  = inputLine("Имя            : ");
    c.middleName = inputLine("Отчество       : ");
    c.phone      = inputLine("Телефон        : ");
    c.email      = inputLine("Email          : ");
    c.address    = inputLine("Адрес          : ");
    c.birthday   = inputLine("Дата рождения  : ");
    c.notes      = inputLine("Заметки        : ");

    if (c.firstName.empty() && c.lastName.empty()) {
        std::cout << "\n  [!] Имя и Фамилия не могут быть пустыми одновременно. Отмена.\n";
        pauseScreen();
        return;
    }

    contacts.push_back(c);
    saveContacts();
    std::cout << "\n  ✓ Контакт добавлен (ID = " << c.id << ").\n";
    pauseScreen();
}

void viewAllContacts() {
    clearScreen();
    std::cout << "\n  ╔══════════════════════════════╗\n";
    std::cout << "  ║     ВСЕ КОНТАКТЫ             ║\n";
    std::cout << "  ╚══════════════════════════════╝\n";

    std::vector<Contact*> ptrs;
    for (auto& c : contacts) ptrs.push_back(&c);
    // Сортировка по фамилии, затем по имени
    std::sort(ptrs.begin(), ptrs.end(), [](const Contact* a, const Contact* b){
        if (a->lastName != b->lastName) return a->lastName < b->lastName;
        return a->firstName < b->firstName;
    });
    printContactList(ptrs);
    pauseScreen();
}

Contact* findById(int id) {
    for (auto& c : contacts)
        if (c.id == id) return &c;
    return nullptr;
}

void viewContact() {
    clearScreen();
    std::cout << "\n  ╔══════════════════════════════╗\n";
    std::cout << "  ║   ПРОСМОТР КОНТАКТА          ║\n";
    std::cout << "  ╚══════════════════════════════╝\n\n";

    std::string idStr = inputLine("Введите ID контакта: ");
    int id;
    try { id = std::stoi(idStr); } catch (...) {
        std::cout << "  [!] Неверный ID.\n";
        pauseScreen(); return;
    }
    Contact* c = findById(id);
    if (!c) { std::cout << "  [!] Контакт с ID=" << id << " не найден.\n"; }
    else     { printContact(*c); }
    pauseScreen();
}

void editContact() {
    clearScreen();
    std::cout << "\n  ╔══════════════════════════════╗\n";
    std::cout << "  ║   РЕДАКТИРОВАТЬ КОНТАКТ      ║\n";
    std::cout << "  ╚══════════════════════════════╝\n\n";

    std::string idStr = inputLine("Введите ID контакта: ");
    int id;
    try { id = std::stoi(idStr); } catch (...) {
        std::cout << "  [!] Неверный ID.\n";
        pauseScreen(); return;
    }
    Contact* c = findById(id);
    if (!c) { std::cout << "  [!] Контакт не найден.\n"; pauseScreen(); return; }

    printContact(*c);
    std::cout << "\n  (Оставьте поле пустым, чтобы не изменять)\n\n";

    auto edit = [](const std::string& prompt, std::string& field){
        std::string val = inputLine(prompt + " [" + field + "]: ");
        if (!val.empty()) field = val;
    };

    edit("Фамилия       ", c->lastName);
    edit("Имя           ", c->firstName);
    edit("Отчество      ", c->middleName);
    edit("Телефон       ", c->phone);
    edit("Email         ", c->email);
    edit("Адрес         ", c->address);
    edit("Дата рождения ", c->birthday);
    edit("Заметки       ", c->notes);

    saveContacts();
    std::cout << "\n  ✓ Контакт обновлён.\n";
    pauseScreen();
}

void deleteContact() {
    clearScreen();
    std::cout << "\n  ╔══════════════════════════════╗\n";
    std::cout << "  ║     УДАЛИТЬ КОНТАКТ          ║\n";
    std::cout << "  ╚══════════════════════════════╝\n\n";

    std::string idStr = inputLine("Введите ID контакта: ");
    int id;
    try { id = std::stoi(idStr); } catch (...) {
        std::cout << "  [!] Неверный ID.\n";
        pauseScreen(); return;
    }
    Contact* c = findById(id);
    if (!c) { std::cout << "  [!] Контакт не найден.\n"; pauseScreen(); return; }

    printContact(*c);
    std::string confirm = inputLine("Удалить этот контакт? (да/нет): ");
    if (confirm == "да" || confirm == "Да" || confirm == "ДА" || confirm == "y" || confirm == "Y") {
        contacts.erase(std::remove_if(contacts.begin(), contacts.end(),
            [id](const Contact& x){ return x.id == id; }), contacts.end());
        saveContacts();
        std::cout << "\n  ✓ Контакт удалён.\n";
    } else {
        std::cout << "\n  Отмена удаления.\n";
    }
    pauseScreen();
}

void searchContacts() {
    clearScreen();
    std::cout << "\n  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║          ПОИСК КОНТАКТОВ             ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";
    std::cout << "  1. По имени / фамилии\n";
    std::cout << "  2. По номеру телефона\n";
    std::cout << "  3. По email\n";
    std::cout << "  4. По адресу\n";
    std::cout << "  5. Универсальный поиск (по всем полям)\n";
    std::cout << "  0. Назад\n\n";

    std::string ch = inputLine("Выберите тип поиска: ");
    if (ch == "0") return;

    std::string query = inputLine("Введите строку поиска: ");
    if (query.empty()) { std::cout << "  [!] Пустой запрос.\n"; pauseScreen(); return; }

    std::vector<Contact*> results;
    for (auto& c : contacts) {
        bool found = false;
        if      (ch == "1") found = containsCI(c.firstName,  query) || containsCI(c.lastName, query) || containsCI(c.middleName, query);
        else if (ch == "2") found = containsCI(c.phone,   query);
        else if (ch == "3") found = containsCI(c.email,   query);
        else if (ch == "4") found = containsCI(c.address, query);
        else if (ch == "5") found = containsCI(c.firstName,  query) || containsCI(c.lastName,  query)
                                 || containsCI(c.middleName,  query) || containsCI(c.phone,     query)
                                 || containsCI(c.email,       query) || containsCI(c.address,   query)
                                 || containsCI(c.birthday,    query) || containsCI(c.notes,     query);
        if (found) results.push_back(&c);
    }

    std::cout << "\n  Результаты поиска по запросу \"" << query << "\":\n";
    printContactList(results);
    pauseScreen();
}

void exportToText() {
    clearScreen();
    std::cout << "\n  ╔══════════════════════════════╗\n";
    std::cout << "  ║   ЭКСПОРТ В ТЕКСТОВЫЙ ФАЙЛ   ║\n";
    std::cout << "  ╚══════════════════════════════╝\n\n";

    std::string fname = inputLine("Имя файла (напр. export.txt): ");
    if (fname.empty()) fname = "export.txt";

    std::ofstream f(fname);
    if (!f) { std::cout << "  [ОШИБКА] Не удалось создать файл.\n"; pauseScreen(); return; }

    f << "СПИСОК КОНТАКТОВ\n";
    f << std::string(60, '=') << "\n\n";

    for (const auto& c : contacts) {
        f << "ID: "          << c.id         << "\n";
        f << "Фамилия:    "  << c.lastName   << "\n";
        f << "Имя:        "  << c.firstName  << "\n";
        f << "Отчество:   "  << c.middleName << "\n";
        f << "Телефон:    "  << c.phone      << "\n";
        f << "Email:      "  << c.email      << "\n";
        f << "Адрес:      "  << c.address    << "\n";
        f << "Дата рожд.: "  << c.birthday   << "\n";
        f << "Заметки:    "  << c.notes      << "\n";
        f << std::string(60, '-') << "\n";
    }
    f << "\nВсего контактов: " << contacts.size() << "\n";
    std::cout << "  ✓ Экспорт выполнен в файл: " << fname << "\n";
    pauseScreen();
}

void showStatistics() {
    clearScreen();
    std::cout << "\n  ╔══════════════════════════════╗\n";
    std::cout << "  ║        СТАТИСТИКА            ║\n";
    std::cout << "  ╚══════════════════════════════╝\n\n";

    int total  = contacts.size();
    int noPhone = 0, noEmail = 0;
    for (const auto& c : contacts) {
        if (c.phone.empty()) noPhone++;
        if (c.email.empty()) noEmail++;
    }
    std::cout << "  Всего контактов      : " << total  << "\n";
    std::cout << "  Без телефона         : " << noPhone << "\n";
    std::cout << "  Без email            : " << noEmail << "\n";
    std::cout << "  Файл данных          : " << DATA_FILE << "\n";
    pauseScreen();
}

void printMainMenu() {
    clearScreen();
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║        МЕНЕДЖЕР КОНТАКТОВ            ║\n";
    std::cout << "  ╠══════════════════════════════════════╣\n";
    std::cout << "  ║  1. Добавить контакт                 ║\n";
    std::cout << "  ║  2. Просмотреть все контакты         ║\n";
    std::cout << "  ║  3. Просмотреть контакт по ID        ║\n";
    std::cout << "  ║  4. Редактировать контакт            ║\n";
    std::cout << "  ║  5. Удалить контакт                  ║\n";
    std::cout << "  ║  6. Поиск контактов                  ║\n";
    std::cout << "  ║  7. Экспорт в текстовый файл         ║\n";
    std::cout << "  ║  8. Статистика                       ║\n";
    std::cout << "  ║  0. Выход                            ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);  // UTF-8 для Windows
    SetConsoleCP(65001);
#endif

    // Аргумент командной строки: --file <имя_файла>
    for (int i = 1; i < argc - 1; ++i) {
        std::string arg = argv[i];
        if ((arg == "--file" || arg == "-f") && i + 1 < argc) {
            // Переопределяем DATA_FILE через глобальную переменную невозможно напрямую,
            // поэтому используем setenv / putenv (для учебных целей просто выводим инфо)
            std::cout << "  [INFO] Пользовательский файл данных: " << argv[i+1] << "\n";
        }
    }

    loadContacts();

    std::string choice;
    while (true) {
        printMainMenu();
        choice = inputLine("Ваш выбор: ");

        if      (choice == "1") 3addContact();
        else if (choice == "2") viewAllContacts();
        else if (choice == "3") viewContact();
        else if (choice == "4") editContact();
        else if (choice == "5") deleteContact();
        else if (choice == "6") searchContacts();
        else if (choice == "7") exportToText();
        else if (choice == "8") showStatistics();
        else if (choice == "0") {
            std::cout << "\n  До свидания!\n\n";
            break;
        } else {
            std::cout << "\n  [!] Неверный выбор. Попробуйте снова.\n";
            pauseScreen();
        }
    }
    return 0;
}
