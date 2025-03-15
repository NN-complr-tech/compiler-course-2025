// RUN: clang -Xclang -load -Xclang ./libmyClangPlugin_mamaeva_olga_FIIT3_ClangAST.so -Xclang -plugin -Xclang myClangPlugin -c %s

double sum(int a, float b) {
    return a + b;
}

int mul(float a, float b) {
    return a + sum(a, b);
}
