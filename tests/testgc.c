// implementare recursiva pentru factorial

// stack frame: n[-2] addr_ret[-1] oldFP[0]
/*
ENTER 0
FPADDR -2
LOAD.i
PUSH.i 3
LESS.i ;(n<3)
JF
NOP // daca este fals nu se sare la vreo adresa, ci se trece la instructiunea urmatoare
FPADDR -2
LOAD.i
FPADDR -2
LOAD.i
PUSH.i 1
SUB.i ;(n-1)
CALL
ENTER 0
...
*/

int fact(int n){
	if(n<3)return n;
	return n*fact(n-1);
	}

void main(){
	// stack frame: 4.9[-2] ret[-1] oldFP[0]
	put_i(4.9);		// se afiseaza 4

	// stack frame fact: n[-2] ret[-1] oldFP[0]	
	// stack frame puti: fact(3)[-2] ret[-1] oldFP[0]	
	put_i(fact(3));	// se afiseaza 6

	// implementare nerecursiva pentru factorial
	int r;
	r=1;
	int i;
	i=2;
	while(i<5){
		r=r*i;
		i=i+1;
		}
	put_i(r);		// se afiseaza 24
	}

/*
factorial nerecursiv
stack frame main: addr_ret[-1] oldFP[0] r[1] i[2]
// int r; r=1;
FPADDR 1
PUSH.i 1
STORE.i
DROP

// int i; i=2;
FPADDR 2
PUSH.i 2
STORE.i
DROP

// while(i<5)
while_addr:
FPADDR 2
LOAD.i
PUSH.i 5
LESS.i ;(i<5)
JF ; false_addr

// r = r * i;
FPADDR 1 // luam adresa lui r pentru STORE
FPADDR 1 // luam adresa lui r pentru LOAD
LOAD.i // load r
FPADDR 2 // luam adresa lui i pentru LOAD
LOAD.i // load i
MUL.i // i * r -> se pune in varful stivei
STORE.i // store la rezultatul inmultirii (din varful stivei) in r (urmatoarea valoare pe stiva, pusa cu primul FPADDR 1)
DROP

// i = i + 1;
FPADDR 2 // luam adresa lui i pentru STORE
FPADDR 2 // luam adresa lui i pentru LOAD
LOAD.i // load in i
PUSH.i 1 // punem 1 pe stiva
ADD.i // i + 1 -> se pune in varful stivei
STORE.i // store la rezultatul adunarii (din varful stivei) in i (urmatoarea valoare pe stiva, pusa cu primul FPADDR 2)
DROP

JMP while_addr

false_addr:
CALL_EXT

*/
