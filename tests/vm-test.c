void f(int n){		// stack frame: n[-2] ret[-1] oldFP[0] i[1]
	int i;
    i=0;
	while(i<n) {
		put_i(i);
		i=i+1;
	}
}
