long long int calculate (int input) {

	int n = 0;

	if (input == 0) return 0;

	if (input == 31) return pow(2,n);
	else n++;
	if (input == 32) return pow(2,n);
	else n++;
	if (input == 33) return pow(2,n);
	else n++;
	if (input == 34) return pow(2,n);
	else n++;
	if (input == 41) return pow(2,n);
	else n++;
	if (input == 42) return pow(2,n);
	else n++;
	if (input == 43) return pow(2,n);
	else n++;
	if (input == 44) return pow(2,n);
	else n++;
	if (input == 51) return pow(2,n);
	else n++;
	if (input == 52) return pow(2,n);
	else n++;
	if (input == 53) return pow(2,n);
	else n++;
	if (input == 54) return pow(2,n);
	else n++;
	if (input == 61) return pow(2,n);
	else n++;
	if (input == 62) return pow(2,n);
	else n++;
	if (input == 63) return pow(2,n);
	else n++;
	if (input == 64) return pow(2,n);
	else n++;
	if (input == 71) return pow(2,n);
	else n++;
	if (input == 72) return pow(2,n);
	else n++;
	if (input == 73) return pow(2,n);
	else n++;
	if (input == 74) return pow(2,n);
	else n++;
	if (input == 81) return pow(2,n);
	else n++;
	if (input == 82) return pow(2,n);
	else n++;
	if (input == 83) return pow(2,n);
	else n++;
	if (input == 84) return pow(2,n);
	else n++;
	if (input == 91) return pow(2,n);
	else n++;
	if (input == 92) return pow(2,n);
	else n++;
	if (input == 93) return pow(2,n);
	else n++;
	if (input == 94) return pow(2,n);
	else n++;
	if (input == 101) return pow(2,n);
	else n++;
	if (input == 102) return pow(2,n);
	else n++;
	if (input == 103) return pow(2,n);
	else n++;
	if (input == 104) return pow(2,n);
	else n++;
	if (input == 111) return pow(2,n);
	else n++;
	if (input == 112) return pow(2,n);
	else n++;
	if (input == 113) return pow(2,n);
	else n++;
	if (input == 114) return pow(2,n);
	else n++;
	if (input == 121) return pow(2,n);
	else n++;
	if (input == 122) return pow(2,n);
	else n++;
	if (input == 123) return pow(2,n);
	else n++;
	if (input == 124) return pow(2,n);
	else n++;
	if (input == 131) return pow(2,n);
	else n++;
	if (input == 132) return pow(2,n);
	else n++;
	if (input == 133) return pow(2,n);
	else n++;
	if (input == 134) return pow(2,n);
	else n++;
	if (input == 141) return pow(2,n);
	else n++;
	if (input == 142) return pow(2,n);
	else n++;
	if (input == 143) return pow(2,n);
	else n++;
	if (input == 144) return pow(2,n);
	else n++;
	if (input == 151) return pow(2,n);
	else n++;
	if (input == 152) return pow(2,n);
	else n++;
	if (input == 153) return pow(2,n);
	else n++;
	if (input == 154) return pow(2,n);

}


int main () {

	int a, b, c, d, e, f, g, h, i, j, k, l, m;
	long long int number;
	int ok = 1;

	while (ok != 0) {
	printf("O valor das cartas pode ser 3,4,5,6,7,8,9,T,J,Q,K,A,2.\n");
	printf("O valor do naipe pode ser D,C,H,S.\n");
	printf("Exemplo para 4 de paus: 4C.\n");
	printf("Insira as cartas que quer na sua mao, separadas por virgulas:\n");
	scanf("%d", &a);
	scanf("%d", &b);
	scanf("%d", &c);
	scanf("%d", &d);
	scanf("%d", &e);
	scanf("%d", &f);
	scanf("%d", &g);
	scanf("%d", &h);
	scanf("%d", &i);
	scanf("%d", &j);
	scanf("%d", &k);
	scanf("%d", &l);
	scanf("%d", &m);
	number = calculate(a) + calculate(b) + calculate(c) + calculate(d) + calculate(e) + calculate(f) +
	calculate(g) + calculate(g) + calculate(i) + calculate(j) + calculate(k) + calculate(l) + calculate(m);   
	printf("A valor para QUERY e: %lld\n", number);
	}

	return 0;
}

