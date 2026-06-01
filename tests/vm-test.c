// f(2.0);
// void f(int n){		// stack frame: n[-2] ret[-1] oldFP[0] i[1]
// 	int i=0;
// 	while(i<n) {
// 		put_i(i);
// 		i=i+1;
// 	}
// }

// f(2.0);
// void f(double n) {
// 	double i = 0.0;
// 	while(i < n) {
// 		put_d(i);
// 		i=i+0.5;
// 	}
// }