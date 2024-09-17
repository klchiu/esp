#ifndef __ACC_GEMM8_PROFILE_H__
#define __ACC_GEMM8_PROFILE_H__

// Structure to hold accelerator data
typedef struct
{
    char acc_name[12];
    int gemm_size;
    float speedup;
    int max_size; // the max size that the gemm accelerator can handle
} AcceleratorEntry;

// Predefined accelerator database
static const AcceleratorEntry accEntries[] = {
    // "name", matrix_size, speedup, max_size
    {"gemm8", 2, 0.97, 8},
    {"gemm8", 3, 0.83, 8},
    {"gemm8", 4, 1.69, 8},
    {"gemm8", 5, 1.89, 8},
    {"gemm8", 6, 1.89, 8},
    {"gemm8", 7, 1.89, 8},
    {"gemm8", 8, 1.89, 8},
    {"gemm16", 2, 0.97, 16},
    {"gemm16", 3, 0.82, 16},
    {"gemm16", 4, 1.70, 16},
    {"gemm16", 5, 1.90, 16},
    {"gemm16", 6, 1.90, 16},
    {"gemm16", 7, 1.90, 16},
    {"gemm16", 8, 1.89, 16},
    {"gemm16", 9, 1.90, 16},
    {"gemm16", 10, 1.89, 16},
    {"gemm32", 2, 0.97, 32},
    {"gemm32", 3, 0.82, 32},
    {"gemm32", 4, 1.69, 32},
    {"gemm32", 5, 1.89, 32},
    {"gemm32", 6, 1.90, 32},
    {"gemm32", 7, 1.89, 32},
    {"gemm32", 8, 1.89, 32},
    {"gemm32", 9, 1.90, 32},
    {"gemm32", 10, 1.89, 32},
    {"gemm32", 20, 2.98, 32},
    {"gemm32", 30, 4.68, 32},
    {"gemm64", 2, 0.97, 64},
    {"gemm64", 3, 0.82, 64},
    {"gemm64", 4, 1.70, 64},
    {"gemm64", 5, 1.89, 64},
    {"gemm64", 6, 1.90, 64},
    {"gemm64", 7, 1.89, 64},
    {"gemm64", 8, 1.89, 64},
    {"gemm64", 9, 1.89, 64},
    {"gemm64", 10, 1.88, 64},
    {"gemm64", 20, 2.95, 64},
    {"gemm64", 30, 4.67, 64},
    {"gemm64", 40, 6.25, 64},
    {"gemm64", 50, 7.89, 64},
    {"gemm64", 60, 9.39, 64},
    {"gemm128", 2, 0.97, 128},
    {"gemm128", 3, 0.82, 128},
    {"gemm128", 4, 1.68, 128},
    {"gemm128", 5, 1.90, 128},
    {"gemm128", 6, 1.90, 128},
    {"gemm128", 7, 1.90, 128},
    {"gemm128", 8, 1.89, 128},
    {"gemm128", 9, 1.90, 128},
    {"gemm128", 10, 1.88, 128},
    {"gemm128", 20, 2.95, 128},
    {"gemm128", 30, 4.66, 128},
    {"gemm128", 40, 6.25, 128},
    {"gemm128", 50, 7.87, 128},
    {"gemm128", 60, 9.31, 128},
    {"gemm128", 70, 10.92, 128},
    {"gemm128", 80, 12.58, 128},
    {"gemm128", 90, 14.11, 128},
    {"gemm128", 100, 15.63, 128}};

#define ACC_ENTRIES_COUNT (sizeof(accEntries) / sizeof(accEntries[0]))

#endif //  __ACC_GEMM8_PROFILE_H__