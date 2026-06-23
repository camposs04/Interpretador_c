int main() {
    {
        int x, y, z;
        x = 10;
        y = 20;

        if (x + y) {
            z = x * y;
            int resultado;
            resultado = z - 5;
            printf("z = %d\n", z);
            printf("resultado = %d\n", resultado);
        } else {
            z = 0;
            printf("z = %d\n", z);
        }
    }

    {
        int a = 10, b = 20;

        if (a == b) {
            a = 0;
        } else {
            if (a != b) {
                a = 1;
            }
        }
        printf("a = %d\n", a);
    }

    {
        int x = 5, y = 10;
        if (x + 2 >= y * 3) {
            x = x + 1;
        }
        printf("x = %d\n", x);
    }

    {
        int x = 1, y = 2;

        if (x > 0) {
            if (y > 0) {
                x = 100;
            } else {
                x = 200;
            }
        }
        printf("x = %d\n", x);
    }

    {
        int a = 1, b, c = 10, d;

        if (a < c) {
            b = a + 5;
            d = b * 2;
        } else {
            b = 0;
            d = 0;
        }
        printf("b = %d, d = %d\n", b, d);
    }

    {
        int logico = 1;

        if (logico) {
            logico = 0;
        }
        printf("logico = %d\n", logico);
    }

    return 0;
}