// RUN: clang -Xclang -load -Xclang /home/shmud/clang-plugin/build/libmyClangPlugin_mamaeva_olga_FIIT3_ClangAST.so -Xclang -plugin -Xclang myClangPlugin -c %s

// Тестовые функции
double sum(int a, float b) {
    return a + b; // int -> float, float -> double
}

int mul(float a, float b) {
    return a + sum(a, b); // float -> int, float -> double, double -> int
}

int main() {
    // Вызов функций для проверки
    double result1 = sum(5, 3.14f);
    int result2 = mul(2.5f, 1.5f);

    return 0;
}
