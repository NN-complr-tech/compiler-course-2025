double fmaDouble(double a, double b, double c) {
    double res1 = a * b + c;
    double res2 = a + b * c;
    return res1;
}

float fmaFloat(float a, float b, float c) {
    float res1 = a * b + c;
    float res2 = a + b * c;
    return res1 + res2 * b;
}

// file needed for me to generate other .ll's
