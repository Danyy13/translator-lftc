struct numeStructMaiMic {
    int mic;
};

struct numeStruct {
    int i;
    char c;
    double d;
    
    struct numeStructMaiMic maiMic;
};

void fn() {

}

void fnParam(int x, double y, char c, char string[100]) {

}

int main() {
    // stmCompound -> varDef
    struct numeStruct myStruct;

    int a;
    int b;
    int x;
    char c;
    double d;
    char string[10];

    // stmCompound -> stm

    // stmCompound -> stm -> stmCompound    
    {
        int b;
    }

    // stmCompound -> stm -> if
    if(x != 0) { // expr -> ... -> exprEq
        x = 1; // expr -> exprAssign
    } else {
        x = -1; // expr -> exprAssign
    }
    
    // stmCompound -> stm -> while
    while(x == 0) { // expr -> ... -> exprEq
        x = 1; // expr -> exprAssign
    }

    x = -x; // exprUnary
    x = !x; // exprUnary
    
    if(x < 1 || x > 10) {
        puts(x);
    } else if(x >= 2 && x <= 9) {
        puts(d);
    }

    if(a < b || a <= b) {
        a = a + b;
        b = b * a;
    } else if(a > b || a >= b) {
        a = a - b;
        b = b / a;
    }

    a = (double)d; 
    // cast nu merge pentru ca intra pe exprPrimary inainte de exprCast,
    // apoi consuma LPAR dar nu da niciodata "eroare" => iteratorul nu se
    // mai intoarce niciodata inainte de LPAR

    myStruct.d = 100.13;

    // --- error cases ---
    // duble d; // Missing or invalid type name. Did you mean 'double'?


    return 0;
}

// --- error cases ---
// int ceva(int a, int b,) {} // Missing parameter after ','