
#define short char *

// void strtolower(int size, char *src, char *dst) {
// 	for (int i = 0; i < size; ++i) {
// 		char c = src[i];
// 		if (c >= 'A' && c <= 'Z') {
// 			c += 'a' - 'A';
// 		}
// 		dst[i] = c;
// 	}
// }

int stuff(int a, long long b) {
	return a;
}

void entry() {
	// Initialise stack.
	asm("MOV ST, 0xffff");
	asm("SUB ST, [0xffff]");
	
	// Logic operator test.
	// int a = 1;
	// int b = 1;
	// int c = 1;
	
	// if (a || (b && c)) {
	// 	// functor();
	// 	asm("MOV [0xfff6], 0x41");
	// }
	
	stuff(12, 0x44cc);
	
	// Halt.
	asm("DEC PC");
}
