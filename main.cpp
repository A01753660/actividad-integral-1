// Actividad Integral 1 - Conceptos Basicos y Algoritmos Fundamentales
// Equipo 1
// Alan Farid Hernández Sanmartín - A01753660
// Alejandro Sánchez Calderón A01754913

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// Archivo de bitacora que corresponde a nmuesto equipo
const string INPUT_FILE = "equipo1.csv";

// Archivo donde se guardaran los eventos ya que se ordenen
const string OUTPUT_FILE = "bitacora_ordenada.csv";



// Numero de campos que conforman un evento de la bitacora
const int FIELD_COUNT = 8;

// Struct representing an IP address
struct ip {
    int o1;
    int o2;
    int o3;
    int o4;
};

// Struct representing a log event (connection between two nodes)
// ts: Date-time of the event as ctime's tm (check docs: https://cplusplus.com/reference/ctime/tm/)
// ip_o: IP of the origin node
// port_o: Port of the origin node
// domain_o: Domain of the origin node
// ip_d: IP of the destination node
// port_d: Port of the destination node
// domain_d: Domain of the destination node
struct event {
    struct tm ts;
    struct ip ip_o;
    string port_o;
    string domain_o;
    struct ip ip_d;
    string port_d;
    string domain_d;
};

// Escribe una direccion ip en un flujo de salida con el formato de la bitacora
// Una ip cuyo primer octeto es 0 representa un valor que no se puede aplicar y se imprime
// como el caracter '-'.
// Parámetros:
//   os: flujo de salida donde se escribirá la direccion.
//   address: direccion ip a escribir.
// Retorno:
//   El mismo flujo de salida, para permitir encadenar operaciones.
ostream& operator<<(ostream &os, const ip &address) {
    if (address.o1 == 0) {
        os << "-";
    } else {
        os << address.o1 << "." << address.o2 << "." << address.o3 << "."
           << address.o4;
    }
    return os;
}

// Escribe un evento en un flujo de salida usando el mismo formato de la
// bitacora, para que el archivo generado sea compatible con el
// archivo de entrada.
// Parametros:
//   os: flujo de salida donde se escribira el evento.
//   e: evento a escribir.
// Retorno:
//   El mismo flujo de salida, para permitir encadenar operaciones.
ostream& operator<<(ostream &os, const event &e) {
    // Se usa %H:%M:%S en lugar de %T porque este ultimo es un especificador de
    // C99 que la biblioteca de C de Windows no reconoce. Ante un especificador
    // invalido strftime falla por completo y deja la cadena vacia, por lo que
    // el evento se imprimiria sin fecha ni hora
    char dateOutput[20] = "";
    strftime(dateOutput, 20, "%d-%m-%Y,%H:%M:%S", &e.ts);
    os << dateOutput << "," << e.ip_o << "," << e.port_o << ","
       << e.domain_o << "," << e.ip_d << "," << e.port_d << ","
       << e.domain_d;
    return os;
}

// Determina si una fecha y hora es anterior a otra.
// Compara los campos del mas significativo al menos significativo: anio, mes,
// dia, hora, minuto y segundo. En cuanto encuentra un campo distinto, ese campo
// decide el orden; solo cuando hay empate se revisa el siguiente campo.
// Se comparan los campos en lugar de convertir a time_t con mktime porque
// mktime depende de la zona horaria y del horario de verano configurados en el
// sistema, lo que podria alterar el orden de eventos cercanos a un cambio de
// horario, y porque implica calculos de calendario que se ejecutarian dos veces
// en cada comparacion.
// Parametros:
//   first: primera fecha y hora a comparar.
//   second: segunda fecha y hora a comparar.
// Retorno:
//   true si first ocurre antes que second, false en cualquier otro caso.
bool isEarlier(const tm &first, const tm &second) {
    if (first.tm_year != second.tm_year) {
        return first.tm_year < second.tm_year;
    }
    if (first.tm_mon != second.tm_mon) {
        return first.tm_mon < second.tm_mon;
    }
    if (first.tm_mday != second.tm_mday) {
        return first.tm_mday < second.tm_mday;
    }
    if (first.tm_hour != second.tm_hour) {
        return first.tm_hour < second.tm_hour;
    }
    if (first.tm_min != second.tm_min) {
        return first.tm_min < second.tm_min;
    }
    return first.tm_sec < second.tm_sec;
}

// Determina el orden relativo entre dos eventos. Es el criterio que utilizara
// std::sort para ordenar la bitacora.
// Parametros:
//   first: primer evento a comparar.
//   second: segundo evento a comparar.
// Retorno:
//   true si first ocurrio antes que second, false en cualquier otro caso.
bool operator<(const event &first, const event &second) {
    return isEarlier(first.ts, second.ts);
}

// Convierte una cadena con formato "a.b.c.d" en una estructura de ip.
// Parametros:
//   text: cadena con la direccion ip, o "-" cuando el valor no aplica.
// Retorno:
//   La direccion ip con sus cuatro octetos. Cuando text es "-" o esta vacia
//   regresa los cuatro octetos en cero, valor que el operador << imprime de
//   vuelta como '-'.
ip parseIp(const string &text) {
    ip result = {0, 0, 0, 0};

    if (text == "-" || text.empty()) {
        return result;
    }

    istringstream stream(text);
    char dot = ' ';
    stream >> result.o1 >> dot >> result.o2 >> dot >> result.o3 >> dot
           >> result.o4;
    return result;
}

// Convierte las cadenas de fecha y hora de la bitacora en una estructura tm.
//Parametros:
//date: cadena con la fecha en formato "dd-mm-aaaa".
//time: cadena con la hora en formato "hh:mm:ss".
//Retorno:
//La fecha y hora correspondientes como estructura tm.
tm parseTimestamp(const string &date, const string &time) {
    tm timestamp = {};
    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    char separator = ' ';

    istringstream dateStream(date);
    dateStream >> day >> separator >> month >> separator >> year;

    istringstream timeStream(time);
    timeStream >> hour >> separator >> minute >> separator >> second;

    timestamp.tm_year = year - 1900;
    timestamp.tm_mon = month - 1;
    timestamp.tm_mday = day;
    timestamp.tm_hour = hour;
    timestamp.tm_min = minute;
    timestamp.tm_sec = second;
    timestamp.tm_isdst = -1;

    return timestamp;
}

// Convierte una linea de la bitacora en un evento. El formato esperado es
// fecha,hora,ip origen,puerto origen,dominio origen,ip destino,puerto destino,
// dominio destino.
// Parametros:
// line: linea de texto a procesar.
// e: evento donde se guardaran los valores obtenidos.
// Retorno:
//  true si la linea conteia los ocho campos y pudo procesarse, false si el
// formato era invalido.
bool parseEvent(const string &line, event &e) {
    istringstream stream(line);
    string fields[FIELD_COUNT] = {};

    for (int i = 0; i < FIELD_COUNT; i++) {
        if (!getline(stream, fields[i], ',')) {
            return false;
        }
    }

    
    if (!fields[7].empty() && fields[7].back() == '\r') {
        fields[7].pop_back();
    }

    e.ts = parseTimestamp(fields[0], fields[1]);
    e.ip_o = parseIp(fields[2]);
    e.port_o = fields[3];
    e.domain_o = fields[4];
    e.ip_d = parseIp(fields[5]);
    e.port_d = fields[6];
    e.domain_d = fields[7];

    return true;
}



// Lee la bitacora completa y carga cada uno de sus eventos en un vector.
// Parametros:
//  filename: nombre del archivo de bitacora a leer.
// Retorno:
// Vector con los eventos leidos. Queda vacio si el archivo no pudo abrirse.
vector<event> readLog(const string &filename) {
    vector<event> events = {};
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: no se pudo abrir el archivo '" << filename
                  << "'\n";
        return events;
    }

    string line = "";
    while (getline(file, line)) {
        if (!line.empty()) {
            event e = {};
            if (parseEvent(line, e)) {
                events.push_back(e);
            } else {
                cerr << "Advertencia: linea con formato invalido "
                          << "ignorada: " << line << "\n";
            }
        }
    }

    file.close();
    return events;
}

// Guarda un vector de eventos en un archivo, respetando el formato de la
// bitacora original para que el resultado pueda volver a leerse con readLog en
// las siguientes actividades integrales.
// Parametros:
//   filename: nombre del archivo a generar.
//   events: eventos a escribir, en el orden en que seran guardados.
// Retorno:
//   true si el archivo pudo crearse y escribirse, false en caso contrario.
bool writeLog(const string &filename, const vector<event> &events) {
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: no se pudo crear el archivo '" << filename
                  << "'\n";
        return false;
    }

    for (unsigned int i = 0; i < events.size(); i++) {
        file << events[i] << "\n";
    }

    file.close();
    return true;
}

// Construye una fecha dejando la hora en 00:00:00, gracias a esto una fecha de
// inicio incluye todos los eventos de ese dia y una fecha de fin los excluye.
// Parameros:
//  day: dia del mes.
//  month: mes del anio, de 1 a 12.
// year: anio con cuatro digitos.
// Retorno:
//   La fecha construida como estructura tm.
tm makeDate(int day, int month, int year) {
    tm date = {};

    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    date.tm_hour = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    date.tm_isdst = -1;

    return date;
}

// Solicita al usuario una fecha por consola y valida que sus valores esten en
// rangos razonables.
// Parametros:
// label: mensaje que se mostrara antes de pedir los datos.
// date: fecha donde se guardara el valor capturado.
// Retorno:
//  true si la fecha capturada si es valida, false si la entrada fue incorrecta.
bool readDate(const string &label, tm &date) {
    int day = 0;
    int month = 0;
    int year = 0;

    cout << label << "\n";
    cout << "  Dia(1-31):";
    cin >> day;
    cout << "  Mes(1-12): ";
    cin >> month;
    cout << "  Anio:  ";
    cin >> year;

    if (!cin || day < 1 || day > 31 || month < 1 || month > 12 || year < 1900) {
        return false;
    }

    date = makeDate(day, month, year);
    return true;
}

//Predicado que evalua si un evento ocurrio en una fecha de referencia o
// despues de ella. Guarda la fecha de referencia como atributo, por lo que el
// mismo predicado sirve para localizar tanto el inicio del periodo buscado como
// su fin.
struct OnOrAfter {
    tm reference;

    // Construyeel predicado a partir de la fecha de referencia.
    // Parametros:
    //   referenceDate: fecha contra la cual se compararan los eventos.
    explicit OnOrAfter(const tm &referenceDate) {
        reference = referenceDate;
    }

    // Evalua un evento contra la fecha de referencia.
    // Parametros:
    //   e: evento a evaluar.
    // Retorno:
    //   true si el evento ocurrio en la fecha de referencia o despues de ella.
    bool operator()(const event &e) const {
        return !isEarlier(e.ts, reference);
    }
};

// Programa principal. Lee la bitacora, la ordena por fecha y hora, guarda el
// resultado en un archivo y despliega los eventos del periodo que indique el
// usuario.
// Retorno:
//  0 si la ejecucion fue exitosa, 1 si ocurrio un error de lectura o de
// captura de datos.
int main() {
    cout << "Leyendo bitacora '" << INPUT_FILE << "'...\n";
    vector<event> events = readLog(INPUT_FILE);

    if (events.empty()) {
        cout << "No se registraron eventos. Verifica el nombre del archivo.\n";
        return 1;
    }
    cout << "Eventos leidos: " << events.size() << "\n";

    // std::sort utiliza el operador < sobrecargado para determinar el orden
    cout << "Ordenando eventos por fecha y hora...\n";
    sort(events.begin(), events.end());

    if (writeLog(OUTPUT_FILE, events)) {
        cout << "Bitacora ordenada guardada en '" << OUTPUT_FILE << "'\n";
    }

    cout << "\nRango de la bitacora:\n";
    cout << "  Primer evento: " << events.front() << "\n";
    cout << "  Ultimo evento: " << events.back() << "\n";

    tm startDate = {};
    if (!readDate("\nIngresa la fecha de INICIO de la busqueda (inclusiva).",
                  startDate)) {
        cout << "Entrada invalida.\n";
        return 1;
    }

    // std::find_if regresa un iterador al primer evento que cumple el
    // predicado. Como la fecha de inicio es inclusiva, ese evento abre el
    // periodo buscado
    vector<event>::iterator start = find_if(events.begin(), events.end(),
                                                 OnOrAfter(startDate));

    if (start == events.end()) {
        cout << "\nNo hay eventos registrados a partir de esa fecha.\n";
        return 0;
    }

    vector<event>::iterator end = events.end();
    tm endDate = {};
    bool hasEnd = false;
    char answer = 'n';

    cout << "\nDeseas acotar la busqueda con una fecha de fin? (s/n): ";
    cin >> answer;

    if (answer == 's' || answer == 'S') {
        if (!readDate("\nIngresa la fecha de FIN de la busqueda (exclusiva).",
                      endDate)) {
            cout << "Entrada invalida.\n";
            return 1;
        }

        // El primer evento en la fecha de fin ya no pertenece al periodo, por
        // lo que sirve directamente como limite superior abierto. La busqueda
        // arranca en start porque el fin nunca puede ser anterior al inicio
        end = find_if(start, events.end(), OnOrAfter(endDate));
        hasEnd = true;

        if (start == end) {
            cout << "\nNo hay eventos registrados en ese periodo.\n";
            return 0;
        }
    }

    char startLabel[20] = "";
    strftime(startLabel, 20, "%d-%m-%Y", &startDate);
    cout << "\nEventos a partir del " << startLabel;

    if (hasEnd) {
        char endLabel[20] = "";
        strftime(endLabel, 20, "%d-%m-%Y", &endDate);
        cout << "y antes del " << endLabel;
    }

    cout << " (" << (end - start) << " en total):\n";

    for (vector<event>::iterator cursor = start; cursor != end; cursor++) {
        cout << *cursor << "\n";
    }

    return 0;
}