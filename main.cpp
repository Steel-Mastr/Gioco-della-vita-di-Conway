#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <conio.h>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace chrono;
using namespace this_thread;

#define WIDTH 100
#define HEIGHT 90
#ifdef _WIN32
#define CLEAN system("cls");
#else
#define CLEAN system("clear");
#endif

string ripeti(int rep, string s) {
    string out;
    for (int i = 0; i < rep; i++)
        out += s;
    return out;
}

namespace simulazione {
    bool cellule[HEIGHT][WIDTH];
    int vicini[HEIGHT][WIDTH];

    // Inizializza tutte le cellule a false
    void inizializza() {
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                cellule[y][x] = false;
    }

    bool isAlive(int x, int y) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
            return false;
        return cellule[y][x];
    }

    int numeroVicini(int x, int y) {
        int count = 0;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                if (isAlive(x + dx, y + dy)) count++;
            }
        }
        return count;
    }

    bool OperazioniFondamentali(int x, int y) {
        int Vicini = vicini[y][x];
        bool stato_attuale = cellule[y][x];

        // NASCITA: cella morta con 3 vicini diventa viva
        if (!stato_attuale && Vicini == 3) return true;

        // SOPRAVVIVENZA: cella viva con 2 o 3 vicini sopravvive
        if (stato_attuale && (Vicini == 2 || Vicini == 3)) return true;

        // MORTE in tutti gli altri casi
        return false;
    }

    void updateGame() {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                vicini[y][x] = numeroVicini(x, y);
            }
        }

        bool nuove_cellule[HEIGHT][WIDTH];
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                nuove_cellule[y][x] = OperazioniFondamentali(x, y);
            }
        }

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                cellule[y][x] = nuove_cellule[y][x];
            }
        }
    }

    void renderGame() {
        CLEAN;
        string out = ripeti(WIDTH + 2, "-") + "\n";
        for (int y = HEIGHT - 1; y >= 0; y--) {
            out += "|";
            for (int x = 0; x < WIDTH; x++)
                out += (cellule[y][x] ? "#" : " ");
            out += "|\n";
        }
        out += ripeti(WIDTH + 2, "-") + "\n";
        cout << out;
    }

    enum tipi_attesa {
        COSTANTE,
        ATTESA_INPUT_UTENTE
    };

    tipi_attesa tipo_attesa;
    milliseconds attesa = milliseconds(100);

    // CORREZIONE: Aggiunta funzione per controllare input durante modalità automatica
    bool checkForExit() {
        if (_kbhit()) {  // Controlla se c'è un tasto premuto
            char c = _getch();
            if (c == 'q' || c == 'Q' || c == 27) {  // 'q', 'Q' o ESC
                return true;
            }
        }
        return false;
    }

    bool attendi() {
        if (tipo_attesa == COSTANTE) {
            // In modalità automatica, controlla se l'utente vuole uscire
            auto start = steady_clock::now();
            while (steady_clock::now() - start < attesa) {
                if (checkForExit()) {
                    return true;  // Richiesta di uscita
                }
                sleep_for(milliseconds(10));  // Piccola pausa per non sovraccaricare la CPU
            }
        }
        else {
            cout << "Premi 'w' o spazio per continuare, 'q' per uscire..." << endl;
            char c;
            do {
                c = _getch();
                if (c == 'q' || c == 'Q') {
                    return true;  // Richiesta di uscita
                }
            } while (c != 'w' && c != ' ');
        }
        return false;  // Continua la simulazione
    }

    bool Tick() {
        updateGame();
        renderGame();
        return attendi();  // Restituisce true se l'utente vuole uscire
    }

    // Controlla se tutte le cellule sono morte
    bool tutteVuote() {
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                if (cellule[y][x]) return false;
        return true;
    }

    void inserisciPattern(const vector<string>& pattern, int startX = -1, int startY = -1) {
        if (pattern.empty()) return;

        int W = 0;
        for (const string& s : pattern)
            if ((int)s.length() > W) W = s.length();
        int H = pattern.size();

        int posX = (startX == -1) ? WIDTH / 2 - W / 2 : startX;
        int posY = (startY == -1) ? HEIGHT / 2 - H / 2 : startY;

        // Correzione: ribalta il pattern verticalmente
        // Calcola la y corretta per ogni cella
        for (int i = 0; i < H; i++) {
            for (int j = 0; j < (int)pattern[i].length(); j++) {
                int x = posX + j;
                int y = posY + (H - 1 - i); // Correzione: ribalta l'indice i
                if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                    cellule[y][x] = (pattern[i][j] == '#');
                }
            }
        }
    }
}

namespace presets {
    struct Pattern {
        string nome;
        vector<string> pattern;
        string descrizione;
    };

    vector<Pattern> patterns = {
        // Pattern classici
        {"Blinker", {"###"}, "Oscillatore semplice - periodo 2"},
        {"Toad", {" ###", "### "}, "Oscillatore - periodo 2"},
        {"Beacon", {"##  ", "##  ", "  ##", "  ##"}, "Oscillatore - periodo 2"},
        {"Pulsar", {
            "  ###   ###  ",
            "             ",
            "#    # #    #",
            "#    # #    #",
            "#    # #    #",
            "  ###   ###  ",
            "             ",
            "  ###   ###  ",
            "#    # #    #",
            "#    # #    #",
            "#    # #    #",
            "             ",
            "  ###   ###  "
        }, "Grande oscillatore - periodo 3"},

        // Oscillatori complessi
        {"Pentadecathlon", {
            "  #  ",
            "  #  ",
            "  #  ",
            "### ###",
            "  #  ",
            "  #  ",
            "  #  ",
            "### ###",
            "  #  ",
            "  #  ",
            "  #  "
        }, "Oscillatore complesso - periodo 15"},
        {"Chad", {
            " ## ",
            "#  #",
            "#  #",
            " ## "
        }, "Oscillatore - periodo 3"},
        {"Kok's Galaxy", {
            "  ####  ",
            " #    # ",
            "#      #",
            "#      #",
            " #    # ",
            "  ####  "
        }, "Oscillatore - periodo 8"},

        // Astronavi
        {"Glider", {" # ", "  #", "###"}, "Astronave che si muove in diagonale"},
        {"Lightweight Spaceship", {" ####", "#   #", "    #", "#  # "}, "Astronave veloce"},
        {"Middleweight Spaceship", {
            "  #### ",
            "#     #",
            "      #",
            "#    # ",
            " ####  "
        }, "Astronave di media dimensione"},
        {"Heavyweight Spaceship", {
            " ##### ",
            "#     #",
            "      #",
            "#     #",
            " ##### ",
            "   #   ",
            "  #    "
        }, "Astronave pesante"},

        // Pattern a crescita indefinita
        {"Gosper Glider Gun", {
            "                         # ",
            "                       # # ",
            "             ##      ##            ##",
            "            #   #    ##            ##",
            " ##        #     #   ##              ",
            " ##        #   # ##    # #           ",
            "           #     #       #           ",
            "            #   #                    ",
            "             ##                      "
        }, "Cannone che spara glider - periodo 30"},
        {"Simkin Glider Gun", {
            "              # ",
            "            # # ",
            "          #     #",
            "         #       #",
            "        #    #    #",
            "        #   # #   #",
            "         #   #   #",
            "          #     #",
            "            # #",
            "              #"
        }, "Cannone glider compatto - periodo 120"},

        // Methuselah (pattern che evolvono per molte generazioni)
        {"Acorn", {
            " #      ",
            "   #    ",
            "##  ### "
        }, "Methuselah - diventa stabile dopo 5206 generazioni."},
        {"R-Pentomino", {
            "  ## ",
            " ##  ",
            "  #  "
        }, "Methuselah - si stabilizza dopo 1103 generazioni."},
        {"Diehard", {
            "      # ",
            "##      ",
            " #   ###"
        }, "Methuselah - scompare dopo 130 generazioni"},
        {"Rabbit", {
            "  ##  ",
            " ##   ",
            " #    ",
            "  #   ",
            "   ## ",
            "   ##"
        }, "Methuselah - si stabilizza dopo 1731 generazioni."},
        {"Pi Heptomino", {
            "  ## ",
            " #  #",
            " #  #",
            "  ## "
        }, "Methuselah - evolve per 100+ generazioni"},
        {"B-heptomino", {
            "  #  ",
            " # # ",
            "## ##",
            " # # ",
            "  #  "
        }, "Methuselah - evolve per 100+ generazioni"},
        {"Phoenix", {
            "## ##",
            "  #  ",
            "  #  ",
            "## ##"
        }, "Oscillatore di periodo 2"},
        {"Thunderbird", {
            " ### ",
            "  #  ",
            "  #  ",
            "  #  "
        }, "Methuselah che diventa un oscillatore di periodo 3"},
        {"Caterer", {
            "  #  ",
            " # # ",
            "#   #",
            " # # ",
            "  #  "
        }, "Pattern che cresce producendo detriti"},

        // Pattern speciali
        {"Eater 1", {
            " ## ",
            "#  #",
            " #  ",
            "  # "
        }, "Pattern 'mangiatore' che può assorbire altre strutture"},
        {"Queen Bee Shuttle", {
            "  ##  ",
            " #  # ",
            "#    #",
            "#    #",
            " #  # ",
            "  ##  ",
            "   #  ",
            "  #   ",
            " #    ",
            "#     ",
            "#     ",
            " #    ",
            "  #   ",
            "   #  "
        }, "Oscillatore gigante - periodo 30"},
    };

    void mostraPresets() {
        CLEAN;
        cout << "=== PRESET DISPONIBILI ===\n\n";
        cout << "0 - Pattern casuale\n\n";

        for (int i = 0; i < (int)patterns.size(); i++) {
            cout << (i + 1) << " - " << patterns[i].nome << "\n";
            cout << "    " << patterns[i].descrizione << "\n\n";
        }

        cout << "ALERT: alcuni presets potrebbero non mantenere le promese fatte, il problema non"
            " è ancora stato identificato, ma sarà risolto appena possibile, grazie per la pazienza."
            " se sapete come risolvere questi problemi un consiglio è più che gradito :)"
            "\n\n";
        cout << "Scegli un numero (0-" << patterns.size() << "): ";
    }

    void applicaPreset(int scelta) {
        simulazione::inizializza();

        if (scelta == 0) {
            // Pattern casuale
            srand(time(nullptr));
            int index = rand() % patterns.size();
            cout << "Pattern casuale scelto: " << patterns[index].nome << endl;
            simulazione::inserisciPattern(patterns[index].pattern);
        }
        else if (scelta > 0 && scelta <= (int)patterns.size()) {
            cout << "Pattern scelto: " << patterns[scelta-1].nome << endl;
            simulazione::inserisciPattern(patterns[scelta-1].pattern);
        }
        else {
            cout << "Scelta non valida!" << endl;
            return;
        }

        cout << "Premi un tasto per continuare...";
        _getch();
    }
}

namespace schermate {
    void creazionePresetByCoordinate() {
        while (true) {
            simulazione::renderGame();
            cout << "\nInserire coordinate X (0-" << (WIDTH-1) << ", -1 per uscire): ";
            int x;
            cin >> x;
            if (x == -1) break;

            cout << "Inserire coordinate Y (0-" << (HEIGHT-1) << "): ";
            int y;
            cin >> y;

            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                simulazione::cellule[y][x] = !simulazione::cellule[y][x];
                cout << "Cella (" << x << "," << y << ") "
                     << (simulazione::cellule[y][x] ? "attivata" : "disattivata") << endl;
            } else {
                cout << "Coordinate non valide!" << endl;
            }

            cout << "Premi un tasto per continuare...";
            _getch();
        }
    }

    void creazionePresetBrute() {
        CLEAN;
        cout << "=== INSERIMENTO SCHEMA MANUALE ===\n\n";
        cout << "Istruzioni:\n";
        cout << "- Usa '#' per le cellule vive\n";
        cout << "- Usa spazio ' ' per le cellule morte\n";
        cout << "- Inserisci una riga alla volta\n";
        cout << "- Termina con una riga contenente solo '-'\n\n";

        vector<string> schema;
        string buffer;
        cin.ignore(); // Pulisce il buffer di input

        int lineNumber = 1;
        while (true) {
            cout << "Riga " << lineNumber << ": ";
            getline(cin, buffer);

            if (buffer == "-") break;

            schema.push_back(buffer);
            lineNumber++;
        }

        if (schema.empty()) {
            cout << "Nessuno schema inserito!" << endl;
            cout << "Premi un tasto per continuare...";
            _getch();
            return;
        }

        // Applica lo schema
        simulazione::inserisciPattern(schema);

        cout << "\nSchema inserito con successo!" << endl;
        cout << "Pattern centrato nella griglia." << endl;
        cout << "Premi un tasto per continuare...";
        _getch();
    }

    void schermataTipoInput() {
        while (true) {
            CLEAN;
            cout << "=== CONWAY'S GAME OF LIFE ===\n\n";
            cout << "Come desideri inserire lo schema iniziale?\n\n";
            cout << " 0 - Scegli da preset predefiniti\n";
            cout << " 1 - Inserisci schema manualmente\n";
            cout << " 2 - Una cellula alla volta (coordinate)\n";
            cout << " q - Esci\n\n";
            cout << "Scelta: ";

            char scelta = _getch();
            cout << scelta << endl;

            simulazione::inizializza();

            switch (scelta) {
                case '0': {
                    presets::mostraPresets();
                    int presetChoice;
                    cin >> presetChoice;
                    presets::applicaPreset(presetChoice);
                    return;
                }
                case '1':
                    creazionePresetBrute();
                    return;
                case '2':
                    creazionePresetByCoordinate();
                    return;
                case 'q':
                case 'Q':
                    exit(0);
                default:
                    cout << "Scelta non valida!" << endl;
                    cout << "Premi un tasto per riprovare...";
                    _getch();
                    continue;
            }
        }
    }
}

int main() {
    while (true) {
        // Schermata di selezione tipo input
        schermate::schermataTipoInput();

        // Configurazione modalità esecuzione
        CLEAN;
        cout << "=== CONFIGURAZIONE ESECUZIONE ===\n\n";
        cout << "Modalità di esecuzione:\n";
        cout << " -1 -> Manuale (premi un tasto per ogni step)\n";
        cout << " 1-100 -> Automatica (N decimi di secondo tra i cicli)\n";
        cout << " Nota: In modalità automatica, premi 'q' o ESC per interrompere\n";
        cout << "       In modalità manuale, premi 'q' per tornare al menu\n\n";
        cout << "Scelta: ";

        int input;
        cin >> input;

        if (input == -1) {
            simulazione::tipo_attesa = simulazione::ATTESA_INPUT_UTENTE;
        } else if (input > 0 && input <= 100) {
            simulazione::tipo_attesa = simulazione::COSTANTE;
            simulazione::attesa = milliseconds(input * 100);
        } else {
            cout << "Valore non valido, uso modalità manuale." << endl;
            simulazione::tipo_attesa = simulazione::ATTESA_INPUT_UTENTE;
        }

        // Esecuzione simulazione
        CLEAN;
        int generazione = 0;
        while (true) {
            cout << "Generazione: " << generazione++ << endl;
            if (simulazione::tipo_attesa == simulazione::COSTANTE) {
                cout << "Premi 'q' o ESC per tornare al menu principale" << endl;
            }

            bool userWantsExit = simulazione::Tick();

            if (userWantsExit) {
                cout << "\nSimulazione interrotta dall'utente." << endl;
                cout << "Premi un tasto per tornare al menu principale...";
                _getch();
                break;
            }

            if (simulazione::tutteVuote()) {
                cout << "\nTutte le cellule sono morte! Simulazione terminata." << endl;
                cout << "Premi un tasto per tornare al menu principale...";
                _getch();
                break;
            }
        }
    }

    return 0;
}