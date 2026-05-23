int a = 10;
int b = 3;
float f = 2.5;

int soma = a + b;
int diff = a - b;
int prod = a * b;
int quoc = a / b;
int resto = a % b;

float fsom = f + 1.5;
float fprod = f * 4.0;

bool eq  = a == 10;
bool neq = a != b;
bool lt  = b < a;
bool gt  = a > b;
bool le  = b <= 3;
bool ge  = a >= 10;

bool logand = eq && lt;
bool logor  = neq || ge;
bool lognot = !logor;

printf(soma);
printf(diff);
printf(prod);
printf(quoc);
printf(resto);
printf(fsom);
printf(fprod);
printf(logand);
printf(lognot);

if (a > b) {
    int x = a - b;
    printf(x);
} else {
    int x = 0;
    printf(x);
}

if (lognot) {
    printf(b);
} else {
    printf(a);
}

int i = 0;
int acc = 0;
while (i < 5) {
    acc += i;
    i++;
}
printf(acc);

int fat = 1;
int n = 5;
int j = 1;
while (j <= n) {
    fat = fat * j;
    j++;
}
printf(fat);

for (int k = 0; k < 4; k++) {
    f += 1.0;
}
printf(f);

int x = 100;
x -= 25;
x *= 2;
x /= 5;
x %= 7;
printf(x);

int p = 0;
int q = 1;
int r = 0;
int cnt = 0;
while (cnt < 8) {
    r = p + q;
    p = q;
    q = r;
    cnt++;
}
printf(q);