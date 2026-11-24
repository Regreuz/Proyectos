#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cctype>

typedef enum {
    OP_REGISTRO,
    OP_INMEDIATO,
    OP_ETIQUETA,
    OP_MEMORIA
} TipoOperando;

typedef struct {
    TipoOperando tipo;
    std::string texto;
    int tamanio;
} Operando;

typedef struct {
    std::string mnemonico;
    std::vector<Operando> operandos;
    size_t numeroLinea;
} Instruccion;

//Estructura de salida 
typedef struct {
    std::vector<uint8_t> bytes;
    size_t numeroLinea;
    std::string mnemonico;
} InstruccionCodificada;

//Tablas de registros de para registros
static int indiceRegistro32(const std::string& reg_in) {
    std::string reg = reg_in;
    for (char& c : reg) c = (char)std::toupper((unsigned char)c);

    if (reg == "EAX") return 0;
    if (reg == "ECX") return 1;
    if (reg == "EDX") return 2;
    if (reg == "EBX") return 3;
    if (reg == "ESP") return 4;
    if (reg == "EBP") return 5;
    if (reg == "ESI") return 6;
    if (reg == "EDI") return 7;

    return -1;   // no es registro de 32 bits
}

//Byte MOdR/M, mod = 11, reg, r/m
static uint8_t construirModRM_RegReg(int reg, int rm) {
    return (uint8_t)(0xC0 | ((reg & 0x7) << 3) | (rm & 0x7));
}

//Codificar con el mnemonico

//MOV reg, reg
//Opcodes: 88 /r (r/m8, r8), 89 /r (r/m32, r32), 8A /r, 8B /r
static bool codificarMOV_RegReg(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    int dst = indiceRegistro32(instr.operandos[0].texto);
    int src = indiceRegistro32(instr.operandos[1].texto);

    if (dst < 0 || src < 0) {
        error = "MOV reg,reg: solo se soportan registros de 32 bits";
        return false;
    }

    out.bytes.push_back(0x89);                              // opcode
    out.bytes.push_back(construirModRM_RegReg(src, dst));   // reg=fuente, rm=destino
    return true;
}

// Para ADD reg, reg
// Opcodes: 00 /r (r/m8, r8), 01 /r (r/m32, r32), 02 /r, 03 /r
static bool codificarADD_RegReg(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    int dst = indiceRegistro32(instr.operandos[0].texto);
    int src = indiceRegistro32(instr.operandos[1].texto);

    if (dst < 0 || src < 0) {
        error = "ADD reg,reg: solo se soportan registros de 32 bits";
        return false;
    }

    out.bytes.push_back(0x01);
    out.bytes.push_back(construirModRM_RegReg(src, dst));
    return true;
}

// SUB reg, reg
// Opcodes: 28 /r (r/m8, r8), 29 /r (r/m32, r32), 2A /r, 2B /r
static bool codificarSUB_RegReg(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    int dst = indiceRegistro32(instr.operandos[0].texto);
    int src = indiceRegistro32(instr.operandos[1].texto);

    if (dst < 0 || src < 0) {
        error = "SUB reg,reg: solo se soportan registros de 32 bits";
        return false;
    }

    out.bytes.push_back(0x29);
    out.bytes.push_back(construirModRM_RegReg(src, dst));
    return true;
}

// CMP reg, reg
// Opcodes: 38 /r (r/m8, r8), 39 /r (r/m32, r32), 3A /r, 3B /r
static bool codificarCMP_RegReg(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    int dst = indiceRegistro32(instr.operandos[0].texto);
    int src = indiceRegistro32(instr.operandos[1].texto);

    if (dst < 0 || src < 0) {
        error = "CMP reg,reg: solo se soportan registros de 32 bits";
        return false;
    }

    out.bytes.push_back(0x39);
    out.bytes.push_back(construirModRM_RegReg(src, dst));
    return true;
}

// MOV reg, inmediato (32 bits)
// Opcode: B8+rd id  (MOV r32, imm32)
static bool codificarMOV_RegInm(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    int dst = indiceRegistro32(instr.operandos[0].texto);
    if (dst < 0) {
        error = "MOV reg,imm: destino debe ser registro de 32 bits";
        return false;
    }

    // Parsear el inmediato (decimal o hexadecimal)
    uint32_t imm = 0;
    try {
        imm = (uint32_t)std::stoul(instr.operandos[1].texto, nullptr, 0);
    } catch (...) {
        error = "MOV reg,imm: inmediato no numérico: " + instr.operandos[1].texto;
        return false;
    }

    out.bytes.push_back((uint8_t)(0xB8 + dst));   // B8+rd
    // imm32 little-endian
    out.bytes.push_back((uint8_t)(imm & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 8) & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 16) & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 24) & 0xFF));
    return true;
}

// ADD reg, inmediato (32 bits)
// Opcode: 81 /0 id  (ADD r/m32, imm32)
static bool codificarADD_RegInm(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    int dst = indiceRegistro32(instr.operandos[0].texto);
    if (dst < 0) {
        error = "ADD reg,imm: destino debe ser registro de 32 bits";
        return false;
    }

    uint32_t imm = 0;
    try {
        imm = (uint32_t)std::stoul(instr.operandos[1].texto, nullptr, 0);
    } catch (...) {
        error = "ADD reg,imm: inmediato no numérico: " + instr.operandos[1].texto;
        return false;
    }

    out.bytes.push_back(0x81);
    out.bytes.push_back(construirModRM_RegReg(0, dst));   // reg=0 (/0 para ADD)
    out.bytes.push_back((uint8_t)(imm & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 8) & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 16) & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 24) & 0xFF));
    return true;
}

// SUB reg, inmediato (32 bits)
// Opcode: 81 /5 id  (SUB r/m32, imm32)
static bool codificarSUB_RegInm(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    int dst = indiceRegistro32(instr.operandos[0].texto);
    if (dst < 0) {
        error = "SUB reg,imm: destino debe ser registro de 32 bits";
        return false;
    }

    uint32_t imm = 0;
    try {
        imm = (uint32_t)std::stoul(instr.operandos[1].texto, nullptr, 0);
    } catch (...) {
        error = "SUB reg,imm: inmediato no numérico: " + instr.operandos[1].texto;
        return false;
    }

    out.bytes.push_back(0x81);
    out.bytes.push_back(construirModRM_RegReg(5, dst));   // reg=5 (/5 para SUB)
    out.bytes.push_back((uint8_t)(imm & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 8) & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 16) & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 24) & 0xFF));
    return true;
}

// CMP reg, inmediato (32 bits)
// Opcode: 81 /7 id  (CMP r/m32, imm32)
static bool codificarCMP_RegInm(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    int dst = indiceRegistro32(instr.operandos[0].texto);
    if (dst < 0) {
        error = "CMP reg,imm: primer operando debe ser registro de 32 bits";
        return false;
    }

    uint32_t imm = 0;
    try {
        imm = (uint32_t)std::stoul(instr.operandos[1].texto, nullptr, 0);
    } catch (...) {
        error = "CMP reg,imm: inmediato no numérico: " + instr.operandos[1].texto;
        return false;
    }

    out.bytes.push_back(0x81);
    out.bytes.push_back(construirModRM_RegReg(7, dst));   // reg=7 (/7 para CMP)
    out.bytes.push_back((uint8_t)(imm & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 8) & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 16) & 0xFF));
    out.bytes.push_back((uint8_t)((imm >> 24) & 0xFF));
    return true;
}

// JMP etiqueta (relativo, 32 bits)
// Opcode: E9 cd  (JMP rel32)
// Todavia pendiente, preguntar
static bool codificarJMP_Etiqueta(const Instruccion& instr, InstruccionCodificada& out, std::string& error) {
    if (instr.operandos[0].tipo != OP_ETIQUETA) {
        error = "JMP: solo se soportan saltos a etiqueta (rel32)";
        return false;
    }

    out.bytes.push_back(0xE9);
    out.bytes.push_back(0x00);   // Todavia pendiente
    out.bytes.push_back(0x00);
    out.bytes.push_back(0x00);
    out.bytes.push_back(0x00);
    return true;
}

//Codificacion de la instruccion
bool codificarInstruccion(
    const Instruccion& instr,
    InstruccionCodificada& out,
    std::string& error)
{
    out.bytes.clear();
    out.numeroLinea = instr.numeroLinea;
    out.mnemonico   = instr.mnemonico;

    if (instr.operandos.empty()) {
        error = "Instrucción sin operandos";
        return false;
    }

    //JMP, primero porque pues es el unico de una instruccion
    if (instr.mnemonico == "JMP") {
        if (instr.operandos[0].tipo == OP_ETIQUETA) {
            return codificarJMP_Etiqueta(instr, out, error);
        }
        error = "JMP: combinación no soportada (solo etiqueta)";
        return false;
    }

    //Las demas operaciones que necesitan de dos operandos
    if (instr.operandos.size() != 2) {
        error = "Se esperaban 2 operandos para " + instr.mnemonico;
        return false;
    }

    const Operando& op1 = instr.operandos[0];
    const Operando& op2 = instr.operandos[1];

    //MOV
    if (instr.mnemonico == "MOV") {
        if (op1.tipo == OP_REGISTRO && op2.tipo == OP_REGISTRO)
            return codificarMOV_RegReg(instr, out, error);
        if (op1.tipo == OP_REGISTRO && op2.tipo == OP_INMEDIATO)
            return codificarMOV_RegInm(instr, out, error);

        error = "MOV: combinación no soportada (" +
                std::string(op1.tipo == OP_MEMORIA ? "memoria" : "reg") + ", " +
                std::string(op2.tipo == OP_MEMORIA ? "memoria" : "inm") + ")";
        return false;
    }

    //ADD
    if (instr.mnemonico == "ADD") {
        if (op1.tipo == OP_REGISTRO && op2.tipo == OP_REGISTRO)
            return codificarADD_RegReg(instr, out, error);
        if (op1.tipo == OP_REGISTRO && op2.tipo == OP_INMEDIATO)
            return codificarADD_RegInm(instr, out, error);

        error = "ADD: combinación no soportada";
        return false;
    }

    //SUB
    if (instr.mnemonico == "SUB") {
        if (op1.tipo == OP_REGISTRO && op2.tipo == OP_REGISTRO)
            return codificarSUB_RegReg(instr, out, error);
        if (op1.tipo == OP_REGISTRO && op2.tipo == OP_INMEDIATO)
            return codificarSUB_RegInm(instr, out, error);

        error = "SUB: combinación no soportada";
        return false;
    }

    //CMP
    if (instr.mnemonico == "CMP") {
        if (op1.tipo == OP_REGISTRO && op2.tipo == OP_REGISTRO)
            return codificarCMP_RegReg(instr, out, error);
        if (op1.tipo == OP_REGISTRO && op2.tipo == OP_INMEDIATO)
            return codificarCMP_RegInm(instr, out, error);

        error = "CMP: combinación no soportada";
        return false;
    }

    error = "Mnemónico no soportado: " + instr.mnemonico;
    return false;
}

//Funcion de control, proximamente retirar
void imprimirInstruccionCodificada(const InstruccionCodificada& ic) {
    std::cout << "[codificar] línea " << ic.numeroLinea
              << " (" << ic.mnemonico << "): ";
    for (uint8_t b : ic.bytes) {
        std::printf("%02X ", b);
    }
    std::cout << std::endl;
}

//Codificando en tandas o lotes
bool codificarPrograma(
    const std::vector<Instruccion>& instrucciones,
    std::vector<InstruccionCodificada>& salida,
    std::vector<std::string>& errores)
{
    bool todoOk = true;

    for (const auto& instr : instrucciones) {
        InstruccionCodificada ic;
        std::string error;

        if (codificarInstruccion(instr, ic, error)) {
            salida.push_back(ic);
        } else {
            errores.push_back("línea " + std::to_string(instr.numeroLinea) +
                              ": " + error);
            todoOk = false;
        }
    }
    return todoOk;
}