/*
 * ============================================================
 *   AZERBAYCAN XƏSTƏXANA İDARƏETMƏ SİSTEMİ
 *   Hospital Management System — C++17
 * ============================================================
 *  Tərtibat:
 *      g++ -std=c++17 -o hospital hospital_system.cpp
 *  İcra:
 *      ./hospital
 * ============================================================
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>

 // ─────────────────────────────────────────
 //  HANSI RƏNG KODLARI
 // ─────────────────────────────────────────
#define RST   "\033[0m"
#define BOLD  "\033[1m"
#define RED   "\033[31m"
#define GRN   "\033[32m"
#define YLW   "\033[33m"
#define BLU   "\033[34m"
#define MAG   "\033[35m"
#define CYN   "\033[36m"
#define WHT   "\033[37m"
#define BGRN  "\033[42m"
#define BBLU  "\033[44m"
#define BRED  "\033[41m"
#define BYEL  "\033[43m"

// ─────────────────────────────────────────
//  STRUKTURLAR
// ─────────────────────────────────────────

struct WorkDay {
    std::string day;
    std::string startTime;
    std::string endTime;
};

struct Doctor {
    int         id;
    std::string name;
    std::string surname;
    std::string specialty;
    int         departmentId;
    std::string phone;
    int         experience;
    std::vector<WorkDay> schedule;
};

struct Department {
    int         id;
    std::string name;
    std::string description;
    std::string floor;
    std::string phone;
};

enum class Priority { NORMAL = 0, PRIORITY = 1 };

struct Appointment {
    int         id = 0;
    std::string name;
    std::string surname;
    std::string phone;
    std::string birthDate;
    std::string finCode;
    bool        isVeteranOrMartyr = false;
    std::string vetDocNumber;
    int         doctorId = 0;
    std::string day;
    std::string time;
    Priority    priority = Priority::NORMAL;
    int         queueNumber = 0;
};

// ─────────────────────────────────────────
//  YARDIMÇI FUNKSIYALAR
// ─────────────────────────────────────────

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printLine(char ch = '-', int len = 65) {
    std::cout << CYN;
    for (int i = 0; i < len; ++i) std::cout << ch;
    std::cout << RST << "\n";
}

void printDoubleLine(int len = 65) {
    std::cout << CYN;
    for (int i = 0; i < len; ++i) std::cout << '=';
    std::cout << RST << "\n";
}

void printHeader(const std::string& title) {
    printDoubleLine();
    int total = 65;
    int pad = (total - (int)title.size()) / 2;
    std::cout << BBLU << WHT << BOLD;
    std::cout << std::string(pad, ' ') << title;

    std::cout << RST << "\n";
    printDoubleLine();
}

void printSuccess(const std::string& msg) {
    std::cout << BGRN << BOLD << " ✔  " << msg << " " << RST << "\n";
}

void printError(const std::string& msg) {
    std::cout << BRED << BOLD << " ✘  " << msg << " " << RST << "\n";
}

void printInfo(const std::string& msg) {
    std::cout << BLU << BOLD << "  ℹ  " << RST << WHT << msg << RST << "\n";
}

void pauseScreen() {
    std::cout << YLW << "\n  [ENTER] davam etmək üçün basın..." << RST;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

std::string inputStr(const std::string& prompt) {
    std::cout << GRN << "  ➤ " << WHT << prompt << RST;
    std::string val;
    std::getline(std::cin, val);
    return val;
}

int inputInt(const std::string& prompt, int minV, int maxV) {
    int val;
    while (true) {
        std::cout << GRN << "  ➤ " << WHT << prompt << RST;
        if (std::cin >> val && val >= minV && val <= maxV) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        printError("Xəta! " + std::to_string(minV) + "-" + std::to_string(maxV) + " arasında daxil edin.");
    }
}

bool isValidFIN(const std::string& fin) {
    if (fin.size() != 7) return false;
    for (char c : fin) {
        bool isUpperLetter = (c >= 'A' && c <= 'Z');
        bool isDigit = (c >= '0' && c <= '9');
        if (!isUpperLetter && !isDigit) return false;
    }
    return true;
}

enum class PhoneError { OK, TOO_SHORT, TOO_LONG };

PhoneError checkPhone(const std::string& ph) {
    std::string digits;
    for (char c : ph) if (std::isdigit(c)) digits += c;
    if ((int)digits.size() < 10) return PhoneError::TOO_SHORT;
    if ((int)digits.size() > 10) return PhoneError::TOO_LONG;
    return PhoneError::OK;
}

// ─────────────────────────────────────────
//  XƏSTƏXANA SİSTEMİ
// ─────────────────────────────────────────

class HospitalSystem {
private:
    std::vector<Department>  departments;
    std::vector<Doctor>      doctors;
    std::vector<Appointment> appointments;
    int nextId = 1;

    // Hər doktorun növbə sayğacı (gün+doktor açarı)
    std::map<std::string, int> queueCounter; // key: "doctorId_day"

    // ─── Başlanğıc məlumatlar ───────────────
    void initDepartments() {
        departments = {
            {1, "Terapiya",          "Ümumi müayinə və müalicə",           "1-ci mərtəbə", "012-123-01"},
            {2, "Cərrahiyyə",        "Cərrahi əməliyyatlar",               "2-ci mərtəbə", "012-123-02"},
            {3, "Kardioloji",        "Ürək-damar xəstəlikləri",            "2-ci mərtəbə", "012-123-03"},
            {4, "Nevroloji",         "Sinir sistemi xəstəlikləri",         "3-cü mərtəbə", "012-123-04"},
            {5, "Pediatriya",        "Uşaq xəstəlikləri",                  "1-ci mərtəbə", "012-123-05"},
            {6, "Göz xəstəlikləri", "Oftalmologiya",                      "3-cü mərtəbə", "012-123-06"},
            {7, "Ortopediya",        "Sümük-oynaq xəstəlikləri",           "4-cü mərtəbə", "012-123-07"},
            {8, "Dərmatologiya",     "Dəri xəstəlikləri",                  "1-ci mərtəbə", "012-123-08"},
        };
    }

    void initDoctors() {
        doctors = {
            // Terapiya (dept 1)
            {1, "Anar",    "Hüseynov",  "Terapevt",          1, "050-111-1001", 15,
             {{"Bazar ertəsi","09:00","17:00"},{"Çərşənbə","09:00","17:00"},{"Cümə","09:00","14:00"}}},
            {2, "Nigar",   "Əliyeva",   "Terapevt",          1, "050-111-1002", 10,
             {{"Çərşənbə axşamı","09:00","17:00"},{"Cümə axşamı","09:00","17:00"}}},

             // Cərrahiyyə (dept 2)
             {3, "Kamran",  "Məmmədov",  "Baş cərrah",        2, "050-111-1003", 22,
              {{"Bazar ertəsi","08:00","16:00"},{"Çərşənbə","08:00","16:00"},{"Cümə","08:00","13:00"}}},
             {4, "Leyla",   "Quliyeva",  "Cərrah",            2, "050-111-1004", 8,
              {{"Çərşənbə axşamı","09:00","17:00"},{"Cümə axşamı","09:00","17:00"},{"Şənbə","09:00","14:00"}}},

              // Kardioloji (dept 3)
              {5, "Elçin",   "Babayev",   "Kardioloq",         3, "050-111-1005", 18,
               {{"Bazar ertəsi","09:00","17:00"},{"Çərşənbə","09:00","17:00"},{"Cümə axşamı","09:00","17:00"}}},
              {6, "Sevinc",  "İsmayılova","Kardioloq",         3, "050-111-1006", 12,
               {{"Çərşənbə axşamı","09:00","17:00"},{"Cümə","09:00","14:00"},{"Şənbə","09:00","14:00"}}},

               // Nevroloji (dept 4)
               {7, "Rəşad",   "Nəsirov",   "Nevroloq",          4, "050-111-1007", 14,
                {{"Bazar ertəsi","09:00","17:00"},{"Çərşənbə axşamı","09:00","17:00"},{"Cümə","09:00","14:00"}}},

                // Pediatriya (dept 5)
                {8, "Günay",   "Rəhimova",  "Pediatr",           5, "050-111-1008", 9,
                 {{"Bazar ertəsi","09:00","17:00"},{"Çərşənbə","09:00","17:00"},{"Cümə axşamı","09:00","17:00"}}},

                 // Göz xəstəlikləri (dept 6)
                 {9, "Fərid",   "Əsgərov",   "Oftalmoloq",        6, "050-111-1009", 16,
                  {{"Çərşənbə axşamı","09:00","17:00"},{"Cümə","09:00","14:00"},{"Şənbə","09:00","14:00"}}},

                  // Ortopediya (dept 7)
                  {10,"Tural",   "Həsənov",   "Ortoped",           7, "050-111-1010", 11,
                   {{"Bazar ertəsi","09:00","17:00"},{"Çərşənbə","09:00","17:00"},{"Cümə axşamı","09:00","17:00"}}},

                   // Dərmatologiya (dept 8)
                   {11,"Aytən",   "Musayeva",  "Dermatoloq",        8, "050-111-1011", 7,
                    {{"Çərşənbə axşamı","09:00","17:00"},{"Cümə","09:00","14:00"},{"Şənbə","09:00","14:00"}}},
        };
    }

    // ─── Köməkçi axtarış ───────────────────
    Department* findDept(int id) {
        for (auto& d : departments) if (d.id == id) return &d;
        return nullptr;
    }

    Doctor* findDoctor(int id) {
        for (auto& d : doctors) if (d.id == id) return &d;
        return nullptr;
    }

    std::string queueKey(int docId, const std::string& day) {
        return std::to_string(docId) + "_" + day;
    }

    // Növbə nömrəsi alır və prioritetə görə düzür
    int assignQueueNumber(int docId, const std::string& day) {
        std::string key = queueKey(docId, day);
        queueCounter[key]++;
        return queueCounter[key];
    }

    // Həmin doktor + gün üçün bütün qeydiyyatları növbəyə görə çap et
    std::vector<Appointment*> getSortedQueue(int docId, const std::string& day) {
        std::vector<Appointment*> result;
        for (auto& a : appointments)
            if (a.doctorId == docId && a.day == day)
                result.push_back(&a);

        // Prioritetlilər əvvəl, sonra növbə nömrəsinə görə
        std::stable_sort(result.begin(), result.end(), [](Appointment* a, Appointment* b) {
            if (a->priority != b->priority)
                return (int)a->priority > (int)b->priority;
            return a->queueNumber < b->queueNumber;
            });
        return result;
    }

    // ─────────────────────────────────────────
    //  MENYULAR
    // ─────────────────────────────────────────

    // ── Şöbələr ────────────────────────────
    void showDepartments() {
        clearScreen();
        printHeader("  ŞÖBƏLƏR  ");
        std::cout << "\n";

        std::cout << BOLD << CYN
            << std::left
            << std::setw(5) << "ID"
            << std::setw(22) << "Şöbə adı"
            << std::setw(20) << "Mərtəbə"
            << "Telefon"
            << RST << "\n";
        printLine();

        for (auto& d : departments) {
            // 1-ci sətir: ID | Ad | Mərtəbə | Telefon
            std::cout << std::left
                << std::setw(5) << d.id
                << std::setw(22) << d.name
                << std::setw(20) << d.floor
                << d.phone << "\n";
            // 2-ci sətir: Açıqlama (girintili)
            std::cout << WHT << "     ↳ " << d.description << RST << "\n\n";
        }
        pauseScreen();
    }

    // ── Həkimlər ───────────────────────────
    void showDoctors() {
        clearScreen();
        printHeader("  HƏKİMLƏR  ");
        std::cout << "\n";
        for (auto& dept : departments) {
            std::cout << MAG << BOLD << "  ► " << dept.name << RST << "\n";
            printLine('-', 65);
            bool any = false;
            for (auto& doc : doctors) {
                if (doc.departmentId != dept.id) continue;
                any = true;
                std::cout << BLU << BOLD << "  ID: " << doc.id << RST
                    << "  " << BOLD << doc.name << " " << doc.surname << RST
                    << "  |  " << YLW << doc.specialty << RST
                    << "  |  Təcrübə: " << doc.experience << " il"
                    << "  |  Tel: " << doc.phone << "\n";

                std::cout << GRN << "     İş qrafiki:\n" << RST;
                for (auto& s : doc.schedule) {
                    std::cout << "       • " << s.day << "\n";
                    std::cout << "           " << s.startTime << " – " << s.endTime << "\n";
                }
                std::cout << "\n";
            }
            if (!any) std::cout << "     (Həkim yoxdur)\n\n";
        }
        pauseScreen();
    }

    bool isValidBirthDate(const std::string& date) {
        // Uzunluq dəqiq 10 olmalıdır: dd.mm.yyyy
        if (date.size() != 10) return false;
        // 2-ci və 5-ci mövqedə nöqtə olmalıdır
        if (date[2] != '.' || date[5] != '.') return false;
        // Qalan mövqelər rəqəm olmalıdır
        for (int i = 0; i < 10; ++i) {
            if (i == 2 || i == 5) continue;
            if (!std::isdigit(date[i])) return false;
        }
        int day = std::stoi(date.substr(0, 2));
        int mon = std::stoi(date.substr(3, 2));
        int year = std::stoi(date.substr(6, 4));
        if (day < 1 || day > 31) return false;
        if (mon < 1 || mon > 12) return false;
        if (year < 1900 || year > 2025) return false;
        return true;
    }

    // ── Həkim axtarışı (şöbəyə görə) ───────
    void searchDoctorByDept() {
        clearScreen();
        printHeader("  ŞÖBƏYƏ GÖRƏ HƏKİM AXTAR  ");
        std::cout << "\n";
        for (auto& d : departments)
            std::cout << "  [" << d.id << "] " << d.name << "\n";
        std::cout << "\n";
        int id = inputInt("Şöbə ID seçin: ", 1, (int)departments.size());
        Department* dept = findDept(id);
        if (!dept) { printError("Şöbə tapılmadı!"); pauseScreen(); return; }

        clearScreen();
        printHeader("  " + dept->name + " şöbəsinin həkimləri  ");
        std::cout << "\n";
        bool any = false;
        for (auto& doc : doctors) {
            if (doc.departmentId != id) continue;
            any = true;
            std::cout << BOLD << BLU << "  Dr. " << doc.name << " " << doc.surname << RST
                << "  (ID: " << doc.id << ")\n";
            std::cout << "  İxtisas   : " << doc.specialty << "\n";
            std::cout << "  Təcrübə   : " << doc.experience << " il\n";
            std::cout << "  Telefon   : " << doc.phone << "\n";
            std::cout << GRN << "  İş qrafiki:\n" << RST;
            for (auto& s : doc.schedule) {
                std::cout << "    • " << s.day << "\n";
                std::cout << "          " << s.startTime << " – " << s.endTime << "\n";
            }
            std::cout << "\n";
        }
        if (!any) printInfo("Bu şöbədə həkim qeydə alınmayıb.");
        pauseScreen();
    }

    // ── Qeydiyyat ──────────────────────────
    void registerAppointment() {
        clearScreen();
        printHeader("  YENİ QEYDİYYAT  ");
        std::cout << "\n";

        Appointment ap;
        ap.id = nextId++;

        // ── İstifadəçi məlumatları ──────────
        std::cout << BOLD << CYN << "  [ Şəxsi Məlumatlar ]\n" << RST;
        printLine('-', 65);

        ap.name = inputStr("Ad           : ");
        ap.surname = inputStr("Soyad        : ");

        while (true) {
            ap.phone = inputStr("Telefon (10 reqem): ");
            PhoneError pe = checkPhone(ap.phone);
            if (pe == PhoneError::OK) break;
            if (pe == PhoneError::TOO_LONG)
                printError("Nomre duzgun qeyd olunmayib! 10 reqemden cox daxil edilib.");
            else
                printError("Nomre duzgun qeyd olunmayib! 10 reqemden az daxil edilib.");
        }

        

        while (true) {
            ap.birthDate = inputStr("Dogum tarixi (dd.mm.yyyy): ");
            if (isValidBirthDate(ap.birthDate)) break;
            printError("Tarix duzgun deyil! Format: dd.mm.yyyy (mes. 05.03.1990)");
        }

        while (true) {
            ap.finCode = inputStr("FIN kod (7 simvol, yalniz A-Z ve 0-9): ");
            if (isValidFIN(ap.finCode)) break;
            if (ap.finCode.size() != 7)
                printError("FIN kod dəqiq 7 simvoldan ibarət olmalıdır!");
            else
                printError("FIN kodda yalnız böyük hərf (A-Z) və rəqəm (0-9) istifadə edə bilərsiniz!");
           
        }

        // ── Veteran / Şəhid ailəsi ──────────
        std::cout << "\n" << BOLD << CYN << "  [ Güzəşt Statusu ]\n" << RST;
        printLine('-', 65);
        std::cout << "  Şəhid ailəsindən və ya Qazisinizsə [1], deyilsinizsə [0] daxil edin: ";
        int vetChoice;
        std::cin >> vetChoice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        ap.isVeteranOrMartyr = (vetChoice == 1);

        if (ap.isVeteranOrMartyr) {
            while (true) {
                ap.vetDocNumber = inputStr("Vesiqe/Sened nomresi (7 reqem): ");
                bool valid = true;
                if (ap.vetDocNumber.size() != 7) {
                    valid = false;
                }
                else {
                    for (char c : ap.vetDocNumber) {
                        if (!std::isdigit(c)) { valid = false; break; }
                    }
                }
                if (valid) break;
                if (ap.vetDocNumber.size() != 7)
                    printError("Vesiqe nomresi deqiq 7 reqemden ibaret olmalidir!");
                else
                    printError("Vesiqe nomresinde yalniz reqem (0-9) istifade ede bilersiniz!");
            }
            ap.priority = Priority::PRIORITY;
            std::cout << BYEL << BOLD << "  ★  Prioritet növbə tətbiq ediləcək!" << RST << "\n";
        }
        else {
            ap.vetDocNumber = "";
            ap.priority = Priority::NORMAL;
        }

        // ── Həkim seçimi ────────────────────
        std::cout << "\n" << BOLD << CYN << "  [ Həkim Seçimi ]\n" << RST;
        printLine('-', 65);
        std::cout << "\n  Şöbələr:\n";
        for (auto& d : departments)
            std::cout << "  [" << d.id << "] " << d.name << "\n";
        std::cout << "\n";
        int deptId = inputInt("Şöbə seçin (ID): ", 1, (int)departments.size());

        std::cout << "\n  Həkimlər:\n";
        bool anyDoc = false;
        for (auto& doc : doctors) {
            if (doc.departmentId != deptId) continue;
            anyDoc = true;
            std::cout << "  [" << doc.id << "] Dr. " << doc.name << " " << doc.surname
                << "  —  " << doc.specialty << "\n";
        }
        if (!anyDoc) {
            printError("Bu şöbədə həkim yoxdur!");
            nextId--;
            pauseScreen();
            return;
        }

        int docId = inputInt("Həkim ID seçin: ", 1, (int)doctors.size());
        Doctor* doc = findDoctor(docId);
        if (!doc || doc->departmentId != deptId) {
            printError("Seçilmiş həkim bu şöbəyə aid deyil!");
            nextId--;
            pauseScreen();
            return;
        }
        ap.doctorId = docId;

        // ── Gün seçimi ──────────────────────
        std::cout << "\n  Dr. " << doc->name << " " << doc->surname << " iş günləri:\n";
        for (int i = 0; i < (int)doc->schedule.size(); ++i) {
            std::cout << "  [" << i + 1 << "] " << doc->schedule[i].day
                << "  (" << doc->schedule[i].startTime << " – "
                << doc->schedule[i].endTime << ")\n";
        }
        int dayIdx = inputInt("Gün seçin: ", 1, (int)doc->schedule.size()) - 1;
        ap.day = doc->schedule[dayIdx].day;

        // ── Saat seçimi ─────────────────────
        std::cout << "\n  İş saatları: "
            << doc->schedule[dayIdx].startTime << " – "
            << doc->schedule[dayIdx].endTime << "\n";
        ap.time = inputStr("İstədiyiniz saat (məs. 10:30): ");

        // ── Növbə nömrəsi ───────────────────
        ap.queueNumber = assignQueueNumber(ap.doctorId, ap.day);
        appointments.push_back(ap);

        // ── Nəticə ─────────────────────────
        clearScreen();
        printHeader("  QEYDİYYAT TAMAMLANDI  ");
        std::cout << "\n";
        printSuccess("Qeydiyyatınız uğurla həyata keçirildi!");
        std::cout << "\n";
        const int CARD_W = 51;
        auto padRight = [](const std::string& s, int width) -> std::string {
            int len = 0;
            for (unsigned char c : s)
                if ((c & 0xC0) != 0x80) ++len;
            int spaces = width - len;
            return s + std::string(spaces > 0 ? spaces : 0, ' ');
            };
        auto row = [&](const std::string& label, const std::string& val) {
            std::string cell = "  " + padRight(label, 14) + ": " + val;
            int cellLen = 0;
            for (unsigned char c : cell)
                if ((c & 0xC0) != 0x80) ++cellLen;
            int spaces = CARD_W - cellLen;
            std::cout << BOLD << "  |" << RST
                << cell
                << std::string(spaces > 0 ? spaces : 1, ' ')
                << BOLD << "|\n" << RST;
            };

        std::cout << BOLD << "  +---------------------------------------------------+\n" << RST;
        std::cout << BOLD << "  |         QEYDİYYAT KARTI                           |\n" << RST;
        std::cout << BOLD << "  +---------------------------------------------------+\n" << RST;
        row("Qeydiyyat ID", std::to_string(ap.id));
        row("Ad Soyad", ap.name + " " + ap.surname);
        row("FIN kod", ap.finCode);
        row("Hekim", "Dr. " + doc->name + " " + doc->surname);
        row("Sobe", findDept(doc->departmentId)->name);
        row("Gun", ap.day);
        row("Saat", ap.time);
        if (ap.isVeteranOrMartyr)
            row("Status", "* Prioritet novbe");
        std::cout << BOLD << "  +---------------------------------------------------+\n" << RST;
        std::cout << "\n  " << YLW << "Növbə nömrəniz: " << BOLD << ap.queueNumber << RST << "\n";
        if (ap.isVeteranOrMartyr)
            std::cout << "  " << BYEL << BOLD << " ★  Siz prioritet sıraya əlavə edildiniz! " << RST << "\n";

        pauseScreen();
    }

    // ── Növbəyə bax ────────────────────────
    void viewQueue() {
        clearScreen();
        printHeader("  NÖVBƏ SİYAHISI  ");
        std::cout << "\n";

        // Həkim seç
        std::cout << "  Həkimlər:\n";
        for (auto& doc : doctors)
            std::cout << "  [" << doc.id << "] Dr. " << doc.name << " " << doc.surname
            << "  —  " << findDept(doc.departmentId)->name << "\n";
        std::cout << "\n";
        int docId = inputInt("Həkim ID: ", 1, (int)doctors.size());
        Doctor* doc = findDoctor(docId);
        if (!doc) { printError("Həkim tapılmadı!"); pauseScreen(); return; }

        std::cout << "\n  Gün seçin:\n";
        for (int i = 0; i < (int)doc->schedule.size(); ++i)
            std::cout << "  [" << i + 1 << "] " << doc->schedule[i].day << "\n";
        int dayIdx = inputInt("Gün: ", 1, (int)doc->schedule.size()) - 1;
        std::string day = doc->schedule[dayIdx].day;

        auto queue = getSortedQueue(docId, day);

        clearScreen();
        printHeader("  NÖVBƏ SİYAHISI  ");
        std::cout << "\n  Dr. " << BOLD << doc->name << " " << doc->surname << RST
            << "  |  " << day << "\n\n";

        if (queue.empty()) {
            printInfo("Bu gün üçün qeydiyyat yoxdur.");
            pauseScreen();
            return;
        }

        std::cout << BOLD << CYN
            << std::left
            << std::setw(6) << "Sıra"
            << std::setw(5) << "ID"
            << std::setw(22) << "Ad Soyad"
            << std::setw(10) << "Saat"
            << std::setw(10) << "Status"
            << "FİN"
            << RST << "\n";
        printLine('-', 65);

        int displayOrder = 1;
        for (auto* ap : queue) {
            bool isPrio = (ap->priority == Priority::PRIORITY);
            if (isPrio) std::cout << BYEL << BOLD;
            std::cout << std::left
                << std::setw(6) << displayOrder
                << std::setw(5) << ap->id
                << std::setw(22) << (ap->name + " " + ap->surname)
                << std::setw(10) << ap->time
                << std::setw(10) << (isPrio ? "★ Prior." : "Normal")
                << ap->finCode;
            if (isPrio) std::cout << RST;
            std::cout << "\n";
            displayOrder++;
        }
        pauseScreen();
    }

    // ── Qeydiyyat axtarışı ─────────────────
    void searchAppointment() {
        clearScreen();
        printHeader("  QEYDİYYAT AXTAR  ");
        std::cout << "\n";
        std::cout << "  [1] FİN kodla axtar\n";
        std::cout << "  [2] Qeydiyyat ID ilə axtar\n\n";
        int ch = inputInt("Seçin: ", 1, 2);
    

        Appointment* found = nullptr;

        if (ch == 1) {
            std::string fin = inputStr("FİN kod: ");
            for (auto& a : appointments)
                if (a.finCode == fin) { found = &a; break; }
        }
        else {
            int id = inputInt("Qeydiyyat ID: ", 1, 999999);
            for (auto& a : appointments)
                if (a.id == id) { found = &a; break; }
        }

        if (!found) {
            printError("Qeydiyyat tapılmadı!");
            pauseScreen();
            return;
        }

        clearScreen();
        printHeader("  QEYDİYYAT MƏLUMATLARI  ");
        std::cout << "\n";
        Doctor* doc = findDoctor(found->doctorId);
        Department* dept = doc ? findDept(doc->departmentId) : nullptr;

        auto row = [](const std::string& label, const std::string& val) {
            std::cout << BOLD << GRN << "  " << std::left << std::setw(18) << label
                << RST << ": " << WHT << val << RST << "\n";
            };
        row("Qeydiyyat ID", std::to_string(found->id));
        row("Ad", found->name);
        row("Soyad", found->surname);
        row("Telefon", found->phone);
        row("Doğum tarixi", found->birthDate);
        row("FİN kod", found->finCode);
        row("Həkim", doc ? ("Dr. " + doc->name + " " + doc->surname) : "?");
        row("Şöbə", dept ? dept->name : "?");
        row("Gün", found->day);
        row("Saat", found->time);
        row("Növbə nömrəsi", std::to_string(found->queueNumber));
        if (found->isVeteranOrMartyr) {
            row("Status", "★ Prioritet (Şəhid ailəsi/Qazi)");
            row("Vəsiqə nömrəsi", found->vetDocNumber);
        }
        else {
            row("Status", "Normal");
        }
        pauseScreen();
    }

    // ── Bütün qeydiyyatlar ─────────────────
    void showAllAppointments() {
        clearScreen();
        printHeader("  BÜTÜN QEYDİYYATLAR  ");
        std::cout << "\n";

        if (appointments.empty()) {
            printInfo("Heç bir qeydiyyat yoxdur.");
            pauseScreen();
            return;
        }

        std::cout << BOLD << CYN
            << std::left
            << std::setw(5) << "ID"
            << std::setw(22) << "Ad Soyad"
            << std::setw(10) << "FİN"
            << std::setw(20) << "Həkim"
            << std::setw(12) << "Gün"
            << std::setw(8) << "Saat"
            << "Status"
            << RST << "\n";
        printLine('-', 65);

        for (auto& a : appointments) {
            Doctor* doc = findDoctor(a.doctorId);
            bool isPrio = (a.priority == Priority::PRIORITY);
            if (isPrio) std::cout << BYEL << BOLD;
            std::cout << std::left
                << std::setw(5) << a.id
                << std::setw(22) << (a.name + " " + a.surname)
                << std::setw(10) << a.finCode
                << std::setw(20) << (doc ? doc->surname : "?")
                << std::setw(12) << a.day.substr(0, 10)
                << std::setw(8) << a.time
                << (isPrio ? "★ Prior." : "Normal");
            if (isPrio) std::cout << RST;
            std::cout << "\n";
        }
        std::cout << "\n  Cəmi: " << appointments.size() << " qeydiyyat\n";
        pauseScreen();
    }

    // ─────────────────────────────────────────
    //  ANA MENYU
    // ─────────────────────────────────────────
    void mainMenu() {
        while (true) {
            clearScreen();
            printHeader("  AZERBAYCAN XƏSTƏXANA İDARƏETMƏ SİSTEMİ  ");
            std::cout << "\n";
            std::cout << BLU << "  ┌──────────────────────────────────────┐\n" << RST;
            std::cout << BLU << "  │  " << RST << BOLD << "1." << RST << "  Şöbələri göstər                  " << BLU << "│\n" << RST;
            std::cout << BLU << "  │  " << RST << BOLD << "2." << RST << "  Həkimləri göstər                 " << BLU << "│\n" << RST;
            std::cout << BLU << "  │  " << RST << BOLD << "3." << RST << "  Şöbəyə görə həkim axtar          " << BLU << "│\n" << RST;
            std::cout << BLU << "  │  " << RST << BOLD << "4." << RST << "  Yeni qeydiyyat                   " << BLU << "│\n" << RST;
            std::cout << BLU << "  │  " << RST << BOLD << "5." << RST << "  Növbəyə bax                      " << BLU << "│\n" << RST;
            std::cout << BLU << "  │  " << RST << BOLD << "6." << RST << "  Qeydiyyat axtar                  " << BLU << "│\n" << RST;
            std::cout << BLU << "  │  " << RST << BOLD << "7." << RST << "  Bütün qeydiyyatlar               " << BLU << "│\n" << RST;
            std::cout << BLU << "  │  " << RST << RED << "0." << RST << "  Çıxış                            " << BLU << "│\n" << RST;
            std::cout << BLU << "  └──────────────────────────────────────┘\n" << RST;
            std::cout << "\n";

            int choice = inputInt("Seçiminizi daxil edin (0-7): ", 0, 7);
            switch (choice) {
            case 1: showDepartments();       break;
            case 2: showDoctors();           break;
            case 3: searchDoctorByDept();    break;
            case 4: registerAppointment();   break;
            case 5: viewQueue();             break;
            case 6: searchAppointment();     break;
            case 7: showAllAppointments();   break;
            case 0:
                clearScreen();
                std::cout << GRN << BOLD
                    << "\n  Sistemdən çıxdınız. Sağlıqla!\n\n"
                    << RST;
                return;
            }
        }
    }

public:
    HospitalSystem() {
        initDepartments();
        initDoctors();
    }

    void run() {
        mainMenu();
    }
};

// ─────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────
int main() {
    // UTF-8 dəstəyi üçün (Windows)
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    HospitalSystem hospital;
    hospital.run();
    return 0;
}