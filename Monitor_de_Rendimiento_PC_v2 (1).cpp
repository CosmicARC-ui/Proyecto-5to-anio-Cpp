// ============================================================================
//  MONITOR DE RENDIMIENTO PC  -  Simulacion en consola (Windows)
//  Proyecto final - "
//  Guardar este archivo en codificacion UTF-8.
// ============================================================================
#define _CRT_SECURE_NO_WARNINGS

#include <conio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "shell32.lib")
#endif

using namespace std;

// ----------------------------------------------------------------------------
//  Constantes
// ----------------------------------------------------------------------------
const int MAX_REGISTROS = 50;     // maximo de programas guardados por sesion
const int COL_VALOR = 26;         // columna de los valores (panel izquierdo)
const int COL_PANEL = 45;         // columna donde empieza el panel derecho
const int COL_VALOR_PANEL = 72;   // columna de los valores (panel derecho)

// Filas de la pantalla del monitor (etiquetas y valores usan las mismas)
const int Y_CPU = 7,   Y_GPU = 8,   Y_RAM = 9,   Y_FPS = 10;
const int Y_TGPU = 14, Y_TCPU = 15, Y_TSSD = 16;
const int Y_CGPU = 20, Y_CCPU = 21, Y_CTOT = 22;
const int Y_FCPU = 26, Y_FGPU = 27;
const int Y_SSDC = 31, Y_SSDJ = 32, Y_LEC = 33, Y_ESC = 34;
const int Y_TIEMPO = 38;

// ----------------------------------------------------------------------------
//  Datos de la sesion
// ----------------------------------------------------------------------------
struct Registro
{
    string nombre;
    bool esJuego;
    int duracion;          // segundos
    float cpu, gpu, ram;   // promedios
    float fps;
    float tempCpu, tempGpu;
    float consCpu, consGpu;
};

string usuario;
Registro historial[MAX_REGISTROS];
int cantidadRegistros = 0;

bool guardarInforme();   // se define mas abajo

// ----------------------------------------------------------------------------
//  Utilidades
// ----------------------------------------------------------------------------
void gotoxy(int x, int y)
{
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// Devuelve base +/- rango (con decimales)
float variar(float base, float rango)
{
    return base + (rand() % 2001 - 1000) / 1000.0f * rango;
}

float limitar(float valor, float minimo, float maximo)
{
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

string formatoTiempo(int segundos)
{
    int m = segundos / 60;
    int s = segundos % 60;
    return to_string(m) + ":" + (s < 10 ? "0" : "") + to_string(s);
}

// Dibuja un cuadro del ancho justo del texto (en cualquier salida: pantalla o archivo)
void imprimirCuadro(ostream& salida, const string& texto)
{
    string linea = "";
    for (size_t i = 0; i < texto.length() + 2; i++)
        linea += "═";

    salida << "╔" << linea << "╗" << endl;
    salida << "║ " << texto << " ║" << endl;
    salida << "╚" << linea << "╝" << endl;
}

void cuadroEn(int x, int y, const string& texto)
{
    string linea = "";
    for (size_t i = 0; i < texto.length() + 2; i++)
        linea += "═";

    gotoxy(x, y);     cout << "╔" << linea << "╗";
    gotoxy(x, y + 1); cout << "║ " << texto << " ║";
    gotoxy(x, y + 2); cout << "╚" << linea << "╝";
}

// Lee una opcion de un solo digito. Si el usuario escribe otra cosa devuelve -1
// (evita que el programa se cuelgue si se escribe una letra).
int leerOpcion(int x, int y)
{
    string linea;

    gotoxy(x, y);
    cout << "Por favor ingrese una opcion: ";
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    getline(cin, linea);

    if (linea.length() == 1 && linea[0] >= '0' && linea[0] <= '9')
        return linea[0] - '0';

    return -1;
}

void opcionIncorrecta()
{
    cout << endl << "Opcion incorrecta. Intente de nuevo." << endl;
    Sleep(1500);
}

void esperarTecla()
{
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    _getch();
}

// ----------------------------------------------------------------------------
//  Ventiladores
// ----------------------------------------------------------------------------
int calcularRPMcpu(float temperatura)
{
    if (temperatura < 50)
        return 1000 + rand() % 101;
    else if (temperatura < 60)
        return 1300 + rand() % 101;
    else if (temperatura < 70)
        return 1600 + rand() % 101;
    else if (temperatura < 80)
        return 2100 + rand() % 101;
    else
        return 2500 + rand() % 101;
}

int calcularRPMgpu(float temperatura)
{
    if (temperatura < 50)
        return 0;                       // Fan stop (ventiladores detenidos)
    else if (temperatura < 60)
        return 900 + rand() % 101;
    else if (temperatura < 70)
        return 1300 + rand() % 101;
    else if (temperatura < 80)
        return 1700 + rand() % 101;
    else
        return 2100 + rand() % 101;
}

// ----------------------------------------------------------------------------
//  Logos
// ----------------------------------------------------------------------------
void mostrarLogo()
{
    gotoxy(54, 0);
    cout << "E.E.T. Nº2 'Presbítero José María Colombo' " << endl;
    cout << R"(
	                                       ████████████████████████████████████████████████████████ 
	                                      ██████████████████████████████████████████████████████████
	                                      ██████████████████████████████████████████████████████████
	                                      ███████                 ██████████                 ███████
	                                      ████                    ██████████                    ████
	                                      ██                     ████████████                     ██
	                                                         ████████████████████                              
	                                                      ██████████████████████████      
	                                                    ████████  ██████████  ████████    
	                                                    ██████    ██████████    ██████                       
	                                                    ██████    ██████████    ██████
	                                                      █████   ██████████  ████████
	                                                              ██████████████████
	                                                              ████████████████
	                                                            ████████████████
	                                                          ████████████████
	                                                        ████████████████
	                                                      ██████████████████                      
	                                                     ████████ ██████████                         
	                                                     ███████  ██████████
	                                                    ████████  ██████████
	                                                    ██████████████████████████████
	                                                    ██████████████████████████████
	                                                    ██████████████████████████████
	                                                              ██████████
	                                                              ██████████
	                                                             ████████████
	                                                            ██████████████
	)";
}

// ----------------------------------------------------------------------------
//  Monitor (pantalla de rendimiento) - sirve para juegos y aplicaciones
// ----------------------------------------------------------------------------
void dibujarPantallaMonitor(const string& nombre, bool esJuego)
{
    system("cls");

    imprimirCuadro(cout, "- Monitor de rendimiento -");

    gotoxy(0, 4);
    cout << (esJuego ? "Juego: " : "Aplicacion: ") << nombre;

    // ---------------- Panel izquierdo ----------------
    gotoxy(0, 6);  cout << "----------Uso de componentes----------";
    gotoxy(0, Y_CPU); cout << "CPU: ";
    gotoxy(0, Y_GPU); cout << "GPU: ";
    gotoxy(0, Y_RAM); cout << "RAM: ";
    if (esJuego)
    {
        gotoxy(0, Y_FPS); cout << "Fotogramas por segundo: ";
    }

    gotoxy(0, 12); cout << "----------Temperaturas----------";
    gotoxy(0, Y_TGPU); cout << "Temperatura GPU: ";
    gotoxy(0, Y_TCPU); cout << "Temperatura CPU: ";
    gotoxy(0, Y_TSSD); cout << "Temperatura SSD: ";

    gotoxy(0, 18); cout << "-------Consumo de energia-------";
    gotoxy(0, Y_CGPU); cout << "Consumo GPU: ";
    gotoxy(0, Y_CCPU); cout << "Consumo CPU: ";
    gotoxy(0, Y_CTOT); cout << "Consumo total: ";

    gotoxy(0, 24); cout << "----------Ventiladores----------";
    gotoxy(0, Y_FCPU); cout << "CPU FAN: ";
    gotoxy(0, Y_FGPU); cout << "GPU FANS: ";

    gotoxy(0, 29); cout << "---------Almacenamiento---------";
    gotoxy(0, Y_SSDC); cout << "SSD C: ";
    gotoxy(0, Y_SSDJ); cout << "SSD A (Juegos): ";
    gotoxy(0, Y_LEC);  cout << "Lectura SSD: ";
    gotoxy(0, Y_ESC);  cout << "Escritura SSD: ";

    gotoxy(0, 36); cout << "--------Tiempo de sesion--------";
    gotoxy(0, Y_TIEMPO); cout << (esJuego ? "Tiempo jugando: " : "Tiempo de uso: ");

    // ---------------- Panel derecho ----------------
    for (int y = 5; y <= Y_TIEMPO; y++)
    {
        gotoxy(COL_PANEL, y);
        cout << " ║";
    }

    gotoxy(COL_PANEL, 5);  cout << " ║   ---------Sistema---------";
    gotoxy(COL_PANEL, 7);  cout << " ║   Resolución: 1920 x 1080";
    gotoxy(COL_PANEL, 8);  cout << " ║   Calidad grafica: " << (esJuego ? "Ultra" : "-");
    gotoxy(COL_PANEL, 9);  cout << " ║   DLSS: " << (esJuego ? "Activado" : "Desactivado");
    gotoxy(COL_PANEL, 10); cout << " ║   Ray Tracing: " << (esJuego ? "Activado" : "Desactivado");

    if (esJuego)
    {
        gotoxy(COL_PANEL, 12); cout << " ║   ---------Conexión---------";
        gotoxy(COL_PANEL, 13); cout << " ║   Ping: ";
        gotoxy(COL_PANEL, 14); cout << " ║   Pérdida de paquetes: ";
        gotoxy(COL_PANEL, 15); cout << " ║   Velocidad de red: ";
    }

    gotoxy(COL_PANEL, 17); cout << " ║   ---------Usuario---------";
    gotoxy(COL_PANEL, 19); cout << " ║   Usuario: " << usuario;
    gotoxy(COL_PANEL, 20); cout << " ║   Procesador: Intel i5 - 14400F";
    gotoxy(COL_PANEL, 21); cout << " ║   Memoria RAM: 32 GB DDR5";
    gotoxy(COL_PANEL, 22); cout << " ║   GPU: RTX 5060 Ti 16 GB";

    gotoxy(COL_PANEL, 25); cout << " ║   [ESC] Detener y volver al menu";
}

void ejecutarMonitor
(
    string nombre,
    bool esJuego,
    float cpuBase, float gpuBase, float ramBase, float fpsBase,
    float tgpuBase, float tcpuBase, float tssdBase,
    float cgpuBase, float ccpuBase,
    float ssdCBase, float ssdJBase, float lecturaBase, float escrituraBase,
    float pingBase, float plBase, float redBase
)
{
    // -------- Pantalla de carga --------
    system("cls");
    cout << "Espere..." << endl;
    Sleep(1000);

    cout << "Abriendo " << nombre << ". . ." << endl;
    Sleep(2000);

    cout << (esJuego ? "Juego abierto" : "Aplicacion abierta") << " con exito [OK]" << endl << endl;
    Sleep(1000);

    dibujarPantallaMonitor(nombre, esJuego);

    // -------- Variables de la medicion --------
    float cpu, gpu, ram, tgpu, tcpu, tssd, cgpu, ccpu, ctotal;
    float ssdC, ssdJ, plActual = 0, redActual = 0;
    int fps = 0, lectura, escritura, ping = 0, rpmCpu, rpmGpu;

    // Acumuladores para los promedios
    float aCpu = 0, aGpu = 0, aRam = 0, aFps = 0;
    float aTcpu = 0, aTgpu = 0, aCcpu = 0, aCgpu = 0;
    int mediciones = 0;

    time_t inicio = time(0);
    bool continuar = true;

    while (continuar)
    {
        // ---------- Generar valores ----------
        cpu  = limitar(variar(cpuBase, 1.0f), 0, 100);
        gpu  = limitar(variar(gpuBase, 1.0f), 0, 100);
        ram  = limitar(variar(ramBase, 0.5f), 0, 32);

        tgpu = variar(tgpuBase, 0.5f);
        tcpu = variar(tcpuBase, 0.5f);
        tssd = variar(tssdBase, 0.3f);

        cgpu   = variar(cgpuBase, 2.0f);
        ccpu   = variar(ccpuBase, 2.0f);
        ctotal = cgpu + ccpu;

        rpmCpu = calcularRPMcpu(tcpu);
        rpmGpu = calcularRPMgpu(tgpu);

        ssdC = limitar(variar(ssdCBase, 1.0f), 0, 100);
        ssdJ = limitar(variar(ssdJBase, 1.0f), 0, 100);
        lectura   = (int)variar(lecturaBase, lecturaBase * 0.05f);
        escritura = (int)variar(escrituraBase, escrituraBase * 0.05f);

        if (esJuego)
        {
            fps       = (int)variar(fpsBase, 10.0f);
            ping      = (int)variar(pingBase, 3.0f);
            if (ping < 1) ping = 1;
            plActual  = limitar(variar(plBase, 0.5f), 0, 100);
            redActual = variar(redBase, 2.0f);
        }

        int segundos = (int)difftime(time(0), inicio);

        // ---------- Mostrar valores ----------
        gotoxy(COL_VALOR, Y_CPU); cout << cpu << " %      ";
        gotoxy(COL_VALOR, Y_GPU); cout << gpu << " %      ";
        gotoxy(COL_VALOR, Y_RAM); cout << ram << " GB / 32 GB   ";

        if (esJuego)
        {
            gotoxy(COL_VALOR, Y_FPS); cout << fps << " FPS   ";
        }

        gotoxy(COL_VALOR, Y_TGPU); cout << tgpu << " °C   ";
        gotoxy(COL_VALOR, Y_TCPU); cout << tcpu << " °C   ";
        gotoxy(COL_VALOR, Y_TSSD); cout << tssd << " °C   ";

        gotoxy(COL_VALOR, Y_CGPU); cout << cgpu << " W   ";
        gotoxy(COL_VALOR, Y_CCPU); cout << ccpu << " W   ";
        gotoxy(COL_VALOR, Y_CTOT); cout << ctotal << " W   ";

        gotoxy(COL_VALOR, Y_FCPU); cout << rpmCpu << " RPM   ";
        gotoxy(COL_VALOR, Y_FGPU); cout << rpmGpu << " RPM   ";

        gotoxy(COL_VALOR, Y_SSDC); cout << ssdC << " %   ";
        gotoxy(COL_VALOR, Y_SSDJ); cout << ssdJ << " %   ";
        gotoxy(COL_VALOR, Y_LEC);  cout << lectura << " MB/s   ";
        gotoxy(COL_VALOR, Y_ESC);  cout << escritura << " MB/s   ";

        if (esJuego)
        {
            gotoxy(COL_VALOR_PANEL, 13); cout << ping << " ms   ";
            gotoxy(COL_VALOR_PANEL, 14); cout << plActual << " %   ";
            gotoxy(COL_VALOR_PANEL, 15); cout << redActual << " Mbps   ";
        }

        gotoxy(COL_VALOR, Y_TIEMPO); cout << formatoTiempo(segundos) << "   ";

        // ---------- Acumular para promedios ----------
        mediciones++;
        aCpu  += cpu;
        aGpu  += gpu;
        aRam  += ram;
        aTcpu += tcpu;
        aTgpu += tgpu;
        aCcpu += ccpu;
        aCgpu += cgpu;
        if (esJuego) aFps += fps;

        // ---------- Esperar 1 segundo revisando el teclado (ESC = salir) ----------
        for (int i = 0; i < 10; i++)
        {
            Sleep(100);
            while (_kbhit())
            {
                if (_getch() == 27)
                    continuar = false;
            }
        }
    }

    // -------- Guardar el registro con los promedios --------
    if (mediciones > 0)
    {
        Registro r;
        r.nombre   = nombre;
        r.esJuego  = esJuego;
        r.duracion = (int)difftime(time(0), inicio);
        r.cpu      = aCpu / mediciones;
        r.gpu      = aGpu / mediciones;
        r.ram      = aRam / mediciones;
        r.fps      = esJuego ? aFps / mediciones : 0;
        r.tempCpu  = aTcpu / mediciones;
        r.tempGpu  = aTgpu / mediciones;
        r.consCpu  = aCcpu / mediciones;
        r.consGpu  = aCgpu / mediciones;

        // Si el historial esta lleno se descarta el registro mas viejo
        if (cantidadRegistros == MAX_REGISTROS)
        {
            for (int i = 1; i < MAX_REGISTROS; i++)
                historial[i - 1] = historial[i];
            cantidadRegistros--;
        }

        historial[cantidadRegistros] = r;
        cantidadRegistros++;

        guardarInforme();   // guardado automatico de la sesion
    }

    gotoxy(COL_PANEL, 25);
    cout << " ║   Monitor detenido. Presione una tecla...   ";
    esperarTecla();
    system("cls");
}

// ----------------------------------------------------------------------------
//  Juegos y aplicaciones (cada uno recibe sus valores base)
// ----------------------------------------------------------------------------
void ejecutarJuegos
(
    string nombreJuego,
    float cpuBase, float gpuBase, float ramBase, float fpsBase,
    float tgpuBase, float tcpuBase, float tssdBase,
    float cgpuBase, float ccpuBase,
    float ssdCBase, float ssdJBase, float lecturaBase, float escrituraBase,
    float pingBase, float plBase, float redBase
)
{
    ejecutarMonitor(nombreJuego, true,
                    cpuBase, gpuBase, ramBase, fpsBase,
                    tgpuBase, tcpuBase, tssdBase,
                    cgpuBase, ccpuBase,
                    ssdCBase, ssdJBase, lecturaBase, escrituraBase,
                    pingBase, plBase, redBase);
}

void ejecutarAplicacion
(
    string nombreAplicacion,
    float cpuBase, float gpuBase, float ramBase,
    float tgpuBase, float tcpuBase, float tssdBase,
    float cgpuBase, float ccpuBase,
    float ssdCBase, float ssdJBase, float lecturaBase, float escrituraBase
)
{
    ejecutarMonitor(nombreAplicacion, false,
                    cpuBase, gpuBase, ramBase, 0,
                    tgpuBase, tcpuBase, tssdBase,
                    cgpuBase, ccpuBase,
                    ssdCBase, ssdJBase, lecturaBase, escrituraBase,
                    0, 0, 0);
}

// ----------------------------------------------------------------------------
//  Informe de la sesion (archivo de texto)
// ----------------------------------------------------------------------------
bool guardarInforme()
{
    ofstream informe;
    informe.open("Informe de la Sesion.txt", ios::out);

    if (informe.fail())
        return false;

    informe << fixed << setprecision(1);

    char fecha[32];
    time_t ahora = time(0);
    strftime(fecha, sizeof(fecha), "%d/%m/%Y %H:%M:%S", localtime(&ahora));

    int tiempoTotal = 0;
    for (int i = 0; i < cantidadRegistros; i++)
        tiempoTotal += historial[i].duracion;

    imprimirCuadro(informe, "- Monitor de rendimiento -");
    imprimirCuadro(informe, "- Informe de la sesion -");
    informe << endl;
    informe << "Usuario: " << usuario << endl;
    informe << "Lenguaje: C++" << endl;
    informe << "Fecha: " << fecha << endl;
    informe << "Programas monitoreados: " << cantidadRegistros << endl;
    informe << "Tiempo total monitoreado: " << formatoTiempo(tiempoTotal) << endl;
    informe << endl;

    if (cantidadRegistros == 0)
        informe << "No se abrio ningun juego ni aplicacion en esta sesion." << endl << endl;

    bool hayAlerta = false;
    string alertas = "";

    for (int i = 0; i < cantidadRegistros; i++)
    {
        Registro r = historial[i];

        informe << "=========== " << (i + 1) << ") " << r.nombre
                << (r.esJuego ? " (Juego)" : " (Aplicacion)") << " ===========" << endl;
        informe << "Duracion: " << formatoTiempo(r.duracion) << endl;
        informe << endl;
        informe << "--------- Rendimiento ---------" << endl;
        informe << "CPU Promedio: " << r.cpu << " %" << endl;
        informe << "GPU Promedio: " << r.gpu << " %" << endl;
        informe << "RAM Promedio: " << r.ram << " GB" << endl;
        if (r.esJuego)
            informe << "FPS Promedio: " << (int)r.fps << " FPS" << endl;
        informe << endl;
        informe << "--------- Temperaturas ---------" << endl;
        informe << "Temperatura media CPU: " << r.tempCpu << " °C" << endl;
        informe << "Temperatura media GPU: " << r.tempGpu << " °C" << endl;
        informe << endl;
        informe << "--------- Consumo Energetico ---------" << endl;
        informe << "Consumo medio CPU: " << r.consCpu << " W" << endl;
        informe << "Consumo medio GPU: " << r.consGpu << " W" << endl;
        informe << "Consumo TOTAL medio: " << (r.consCpu + r.consGpu) << " W" << endl;
        informe << endl;

        if (r.tempCpu >= 90 || r.tempGpu >= 85)
        {
            hayAlerta = true;
            alertas += "  - " + r.nombre + "\n";
        }
    }

    informe << "--------- Estado del sistema ---------" << endl;
    informe << endl;

    if (hayAlerta)
    {
        informe << "Estado Final: ATENCION - TEMPERATURAS ELEVADAS" << endl;
        informe << endl;
        informe << "[!] Se detectaron temperaturas altas en:" << endl;
        informe << alertas;
    }
    else
    {
        informe << "Estado Final: SISTEMA ESTABLE" << endl;
        informe << endl;
        informe << "[✓] No se detectaron problemas." << endl;
    }

    informe << "----------------------------------------------------" << endl;
    informe.close();
    return true;
}

// ----------------------------------------------------------------------------
//  Historial en pantalla
// ----------------------------------------------------------------------------
void mostrarHistorial()
{
    system("cls");
    imprimirCuadro(cout, "- Historial de la sesion -");
    cout << endl;
    cout << "Usuario: " << usuario << endl << endl;

    if (cantidadRegistros == 0)
    {
        cout << "Todavia no se abrio ningun juego ni aplicacion." << endl;
    }
    else
    {
        cout << fixed << setprecision(1);
        cout << left << setw(4) << "N" << setw(32) << "Programa" << setw(14) << "Tipo"
             << setw(10) << "Tiempo" << setw(9) << "CPU %" << setw(9) << "GPU %"
             << setw(10) << "RAM GB" << setw(8) << "FPS" << endl;
        cout << string(96, '-') << endl;

        for (int i = 0; i < cantidadRegistros; i++)
        {
            Registro r = historial[i];
            cout << left << setw(4) << (i + 1) << setw(32) << r.nombre
                 << setw(14) << (r.esJuego ? "Juego" : "Aplicacion")
                 << setw(10) << formatoTiempo(r.duracion)
                 << setw(9) << r.cpu << setw(9) << r.gpu << setw(10) << r.ram;

            if (r.esJuego)
                cout << setw(8) << (int)r.fps;
            else
                cout << setw(8) << "-";

            cout << endl;
        }
    }

    cout << endl << "Presione cualquier tecla para volver al menu..." << endl;
    esperarTecla();
    system("cls");
}

// ----------------------------------------------------------------------------
//  Menus de juegos y aplicaciones
// ----------------------------------------------------------------------------
void menuJuegos()
{
  
    cout << "[JUEGOS DISPONIBLES]" << endl;

    imprimirCuadro(cout, "- Forza Horizon 6 - GTA 6 - Ananta - Neverness to Everness - Warzone - Arknights Endfield - Metro Exodus -");

    cout << "[1] FH 6" << endl;
    cout << "[2] GTA 6" << endl;
    cout << "[3] Ananta" << endl;
    cout << "[4] NTE" << endl;
    cout << "[5] WZ" << endl;
    cout << "[6] ArkEnd" << endl;
    cout << "[7] MEx" << endl << endl;

    int opcion = leerOpcion(0, 17);
    system("cls");

    // Orden de los datos:
    //  nombre, CPU, GPU, RAM, FPS, Tgpu, Tcpu, Tssd, Cgpu, Ccpu, SSDc, SSDj, Lectura, Escritura, Ping, PL, Red
    switch (opcion)
    {
        case 1:
            ejecutarJuegos("Forza Horizon 6",           45, 75, 12, 146,  55, 68, 34, 150, 55, 52, 38, 3600, 2100, 32, 2, 48);
            break;
        case 2:
            ejecutarJuegos("GTA VI",                    65, 95, 16,  98,  75, 70, 42, 210, 57, 52, 80, 4700,   80, 34, 2, 50);
            break;
        case 3:
            ejecutarJuegos("Ananta",                    55, 82, 12, 153,  72, 64, 35, 170, 41, 52, 65, 3100,  115, 32, 2, 44);
            break;
        case 4:
            ejecutarJuegos("Neverness To Everness",     60, 85, 13, 186,  74, 66, 36, 180, 43, 52, 70, 3300,   65, 29, 2, 46);
            break;
        case 5:
            ejecutarJuegos("Warzone",                   75, 92, 15, 167,  78, 72, 44, 200, 53, 52, 90, 5200,  160, 32, 2, 47);
            break;
        case 6:
            ejecutarJuegos("Arknights Endfield",        50, 70, 10, 189,  67, 60, 34, 145, 46, 52, 55, 2900,  180, 11, 2, 55);
            break;
        case 7:
            ejecutarJuegos("Metro Exodus Enhanced Edition", 58, 88, 11, 157, 76, 65, 37, 190, 45, 52, 75, 3900, 120, 2, 2, 52);
            break;
        default:
            opcionIncorrecta();
    }
}

void menuAplicaciones()
{
    gotoxy(54, 0);
    cout << "E.E.T. Nº2 'Presbítero José María Colombo' " << endl << endl;
    cout << "[APLICACIONES DISPONIBLES]" << endl;

    imprimirCuadro(cout, "- Brave - Photoshop - Premiere Pro - Adobe After Effects - Blender -");

    cout << "[1] Brave" << endl;
    cout << "[2] Photoshop" << endl;
    cout << "[3] Premiere Pro" << endl;
    cout << "[4] Adobe After Effects" << endl;
    cout << "[5] Blender" << endl << endl;

    int opcion = leerOpcion(0, 12);
    system("cls");

    // Orden de los datos:
    //  nombre, CPU, GPU, RAM, Tgpu, Tcpu, Tssd, Cgpu, Ccpu, SSDc, SSDj, Lectura, Escritura
    switch (opcion)
    {
        case 1:
            ejecutarAplicacion("Brave",         20, 15,  2, 45, 30, 35,  50,  45, 30, 38,  150,  100);
            break;
        case 2:
            ejecutarAplicacion("Photoshop",     45, 40,  5, 55, 58, 45,  76,  75, 50, 48,  400,  300);
            break;
        case 3:
            ejecutarAplicacion("Premiere Pro",  70, 76,  6, 68, 66, 45, 120, 110, 55, 58,  900,  680);
            break;
        case 4:
            ejecutarAplicacion("After Effects", 90, 58,  8, 70, 78, 55, 140, 120, 70, 75,  900,  700);
            break;
        case 5:
            ejecutarAplicacion("Blender",       99, 78, 13, 82, 97, 65, 180, 150, 68, 76, 1450, 1200);
            break;
        default:
            opcionIncorrecta();
    }
}

void menuProgramas()
{
    mostrarLogo();

    cuadroEn(66, 30, "- MENU  GPU -");

    gotoxy(63, 33); cout << "[1] Juegos";
    gotoxy(63, 34); cout << "[2] Aplicaciones de Edicion";
    gotoxy(63, 35); cout << "[3] Volver al menú principal";

    int subopcion = leerOpcion(63, 36);
    system("cls");

    switch (subopcion)
    {
        case 1:
            menuJuegos();
            break;
        case 2:
            menuAplicaciones();
            break;
        case 3:
            break;
        default:
            opcionIncorrecta();
    }
}

// ----------------------------------------------------------------------------
//  Ingreso de usuario y contraseña (simulado)
// ----------------------------------------------------------------------------
string ingresarUsuario(int x, int y)
{
    string nombre;
    int tecla;

    gotoxy(x, y);     cout << "┌──────────────────────┐";
    gotoxy(x, y + 1); cout << "│ Usuario              │";
    gotoxy(x, y + 2); cout << "└──────────────────────┘";

    gotoxy(x + 2, y + 1);

    while (true)
    {
        tecla = _getch();

        // Teclas especiales (flechas, F1...): traen dos codigos, se ignoran
        if (tecla == 0 || tecla == 224)
        {
            _getch();
            continue;
        }

        if (tecla == 13)                      // ENTER
        {
            if (!nombre.empty())
                break;
            continue;
        }

        if (tecla == 8)                       // BACKSPACE
        {
            if (!nombre.empty())
            {
                nombre.pop_back();

                gotoxy(x + 2, y + 1);
                cout << "                    ";
                gotoxy(x + 2, y + 1);
                cout << nombre;
            }
        }
        else if (tecla >= 32 && tecla < 127)  // caracteres imprimibles
        {
            if (nombre.length() < 20)         // maximo 20 caracteres
            {
                if (nombre.empty())           // borrar el texto "Usuario"
                {
                    gotoxy(x + 2, y + 1);
                    cout << "                    ";
                    gotoxy(x + 2, y + 1);
                }

                nombre += (char)tecla;
                cout << (char)tecla;
            }
        }
    }

    return nombre;
}

void ingresarContrasena(int x, int y)
{
    gotoxy(x, y);     cout << "┌──────────────────────┐";
    gotoxy(x, y + 1); cout << "│ Contraseña           │";
    gotoxy(x, y + 2); cout << "└──────────────────────┘";

    Sleep(1000);

    gotoxy(x + 2, y + 1);
    cout << "                    ";
    gotoxy(x + 2, y + 1);

    for (int i = 0; i < 12; i++)
    {
        cout << "*";
        Sleep(250);
    }
}

// ----------------------------------------------------------------------------
//  Ultimos datos registrados (se muestran al cerrar el monitor)
// ----------------------------------------------------------------------------
void mostrarUltimosDatos(bool informeGuardado)
{
    system("cls");
    imprimirCuadro(cout, "- Ultimos datos registrados -");
    cout << endl;
    cout << "Usuario: " << usuario << endl;

    if (cantidadRegistros == 0)
    {
        cout << endl << "No se abrio ningun juego ni aplicacion en esta sesion." << endl;
    }
    else
    {
        Registro r = historial[cantidadRegistros - 1];

        cout << fixed << setprecision(1);
        cout << "Ultimo programa abierto: " << r.nombre
             << (r.esJuego ? " (Juego)" : " (Aplicacion)") << endl;
        cout << "Tiempo de uso: " << formatoTiempo(r.duracion) << endl;
        cout << endl;
        cout << "--------- Rendimiento (promedio) ---------" << endl;
        cout << "CPU: " << r.cpu << " %" << endl;
        cout << "GPU: " << r.gpu << " %" << endl;
        cout << "RAM: " << r.ram << " GB" << endl;
        if (r.esJuego)
            cout << "FPS: " << (int)r.fps << " FPS" << endl;
        cout << endl;
        cout << "--------- Temperaturas (promedio) ---------" << endl;
        cout << "CPU: " << r.tempCpu << " °C" << endl;
        cout << "GPU: " << r.tempGpu << " °C" << endl;
        cout << endl;
        cout << "--------- Consumo (promedio) ---------" << endl;
        cout << "CPU: " << r.consCpu << " W" << endl;
        cout << "GPU: " << r.consGpu << " W" << endl;
        cout << "Total: " << (r.consCpu + r.consGpu) << " W" << endl;
        cout << endl;
        cout << "Programas monitoreados en la sesion: " << cantidadRegistros << endl;
    }

    cout << endl;
    if (informeGuardado)
        cout << "Informe guardado en \"Informe de la Sesion.txt\"" << endl;
    else
        cout << "Error al guardar el informe..." << endl;

    cout << endl << "Presione cualquier tecla para cerrar el monitor..." << endl;
    esperarTecla();
}

// ----------------------------------------------------------------------------
//  Quienes somos (con codigo QR)
// ----------------------------------------------------------------------------
// Abre la imagen "QR.png" con el visor de imagenes de Windows.
// La imagen tiene que estar en la misma carpeta que el programa.
void abrirQR()
{
    ifstream archivo("QR.png");

    if (archivo.fail())
    {
        cout << endl << "No se encontro el archivo \"QR.png\"." << endl;
        cout << "Copielo en la misma carpeta que el programa." << endl;
        Sleep(3000);
    }
    else
    {
        archivo.close();
        ShellExecuteA(NULL, "open", "QR.png", NULL, NULL, SW_SHOWNORMAL);
    }
}

void mostrarQuienesSomos()
{
    int opcion = 0;

    while (opcion != 2)
    {
        system("cls");
        imprimirCuadro(cout, "- Quienes somos -");
        cout << endl;
        cout << "Proyecto: Monitor de Rendimiento PC" << endl;
        cout << "Version: 2.1" << endl;
        cout << "Autor: -" << endl;
        cout << "Institucion: -" << endl;
        cout << "Curso: 5° 1° T.C Ciclo Superior" << endl;
        cout << "Tecnologias: C++ · HTML · CSS · JavaScript" << endl;
        cout << endl;
        cout << "[1] Ver codigo QR" << endl;
        cout << "[2] Volver al menu principal" << endl << endl;

        opcion = leerOpcion(0, 14);

        if (opcion == 1)
            abrirQR();
        else if (opcion != 2)
            opcionIncorrecta();
    }

    system("cls");
}

// ----------------------------------------------------------------------------
//  Programa principal
// ----------------------------------------------------------------------------
int main()
{
    srand((unsigned)time(0));
    ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);

    // Habilitar caracteres especiales (logos y cuadros)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);

    cout << fixed << setprecision(1);   // 1 decimal en los valores medidos

    // ----------------- Advertencia inicial -----------------
    imprimirCuadro(cout, "- ATENCION -");

    cout << "Esta simulacion de |Monitor de rendimiento de pc| está basada en una maquina con las siguientes especificaciones: " << endl << endl;

    cout << "Procesador: Intel Core I5 14400F" << endl;
    cout << "RAM: Patriot Viper Venom DDR5 32GB 6000MHz CL30" << endl;
    cout << "Tarjeta Grafica: ASUS Prime GeForce RTX 5060 Ti 16GB Overclock Edition" << endl << endl;

    cout << "Presione cualquier tecla para continuar..." << endl;
    esperarTecla();
    system("cls");

    // ----------------- Pantalla de ingreso -----------------
    mostrarLogo();

    cuadroEn(59, 30, "- MONITOR DE RENDIMIENTO PC -");

    usuario = ingresarUsuario(63, 33);
    Sleep(400);
    ingresarContrasena(63, 36);

    Sleep(900);
    gotoxy(88, 37);
    cout << "Guardando . . .";
    Sleep(1000);
    gotoxy(88, 37);
    cout << "Guardado con éxito";
    Sleep(1000);
    system("cls");

    // ----------------- Menu principal -----------------
    int opcion = 0;

    while (opcion != 4)
    {
        mostrarLogo();

        cuadroEn(63, 30, "- MENÚ DE OPCIONES -");

        gotoxy(63, 33); cout << "[1] Menú de Juegos y aplicaciones";
        gotoxy(63, 34); cout << "[2] Historial de la sesión";
        gotoxy(63, 35); cout << "[3] Quiénes somos";
        gotoxy(63, 36); cout << "[4] Salir y Cerrar Monitor";

        opcion = leerOpcion(63, 37);
        system("cls");

        switch (opcion)
        {
            case 1:
                menuProgramas();
                break;

            case 2:
                mostrarHistorial();
                break;

            case 3:
                mostrarQuienesSomos();
                break;

            case 4:
                break;

            default:
                opcionIncorrecta();
                system("cls");
        }
    }

    // ----------------- Cierre: ultimos datos + informe -----------------
    bool guardado = guardarInforme();
    mostrarUltimosDatos(guardado);

    system("cls");
    cout << "Hasta pronto, " << usuario << "!" << endl;
    Sleep(1500);

    return 0;
}
