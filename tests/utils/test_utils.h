/**
 * test_utils.h
 * Common utilities for VOF method testing
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <time.h>

// Color codes for terminal output
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Test status tracking
typedef struct {
  int total_tests;
  int passed_tests;
  int failed_tests;
  double total_time;
} TestSuite;

TestSuite test_suite = {0, 0, 0, 0.0};

// Initialize test suite
void test_init(const char* suite_name) {
  printf(ANSI_COLOR_CYAN);
  printf("=================================================\n");
  printf("Test Suite: %s\n", suite_name);
  printf("=================================================\n");
  printf(ANSI_COLOR_RESET);
  test_suite.total_tests = 0;
  test_suite.passed_tests = 0;
  test_suite.failed_tests = 0;
  test_suite.total_time = 0.0;
}

// Start a test
clock_t test_start(const char* test_name) {
  printf("\n" ANSI_COLOR_YELLOW "Running: %s" ANSI_COLOR_RESET "\n", test_name);
  test_suite.total_tests++;
  return clock();
}

// End a test
void test_end(const char* test_name, clock_t start_time, int passed) {
  clock_t end_time = clock();
  double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  test_suite.total_time += elapsed;

  if (passed) {
    test_suite.passed_tests++;
    printf(ANSI_COLOR_GREEN "✓ PASSED" ANSI_COLOR_RESET " (%s, %.3f s)\n",
           test_name, elapsed);
  } else {
    test_suite.failed_tests++;
    printf(ANSI_COLOR_RED "✗ FAILED" ANSI_COLOR_RESET " (%s, %.3f s)\n",
           test_name, elapsed);
  }
}

// Finalize test suite
int test_finalize() {
  printf("\n");
  printf(ANSI_COLOR_CYAN);
  printf("=================================================\n");
  printf("Test Summary\n");
  printf("=================================================\n");
  printf(ANSI_COLOR_RESET);

  printf("Total tests:  %d\n", test_suite.total_tests);
  printf(ANSI_COLOR_GREEN "Passed:       %d\n" ANSI_COLOR_RESET,
         test_suite.passed_tests);

  if (test_suite.failed_tests > 0) {
    printf(ANSI_COLOR_RED "Failed:       %d\n" ANSI_COLOR_RESET,
           test_suite.failed_tests);
  }

  printf("Total time:   %.3f s\n", test_suite.total_time);

  printf("\n");

  if (test_suite.failed_tests == 0) {
    printf(ANSI_COLOR_GREEN "All tests PASSED ✓\n" ANSI_COLOR_RESET);
    return 0;
  } else {
    printf(ANSI_COLOR_RED "Some tests FAILED ✗\n" ANSI_COLOR_RESET);
    return 1;
  }
}

// Assertion with message
#define TEST_ASSERT(condition, message) \
  do { \
    if (!(condition)) { \
      printf(ANSI_COLOR_RED "  Assertion failed: %s\n" ANSI_COLOR_RESET, \
             message); \
      printf("  Location: %s:%d\n", __FILE__, __LINE__); \
      return 0; \
    } \
  } while (0)

// Numerical comparison
#define TEST_APPROX_EQ(a, b, tol, message) \
  do { \
    double _a = (a); \
    double _b = (b); \
    double _tol = (tol); \
    if (fabs(_a - _b) > _tol) { \
      printf(ANSI_COLOR_RED "  Numerical test failed: %s\n" ANSI_COLOR_RESET, \
             message); \
      printf("  Expected: %g, Got: %g, Tolerance: %g\n", _b, _a, _tol); \
      printf("  Error: %g\n", fabs(_a - _b)); \
      printf("  Location: %s:%d\n", __FILE__, __LINE__); \
      return 0; \
    } \
  } while (0)

// Vector comparison
typedef struct {
  double x;
  double y;
  double z;
} Vec3;

int vec_approx_eq(Vec3 a, Vec3 b, double tol) {
  return (fabs(a.x - b.x) < tol &&
          fabs(a.y - b.y) < tol &&
          fabs(a.z - b.z) < tol);
}

// Statistics structure
typedef struct {
  double min;
  double max;
  double mean;
  double std_dev;
  int count;
} Statistics;

// Compute statistics from array
Statistics compute_stats(double* data, int n) {
  Statistics stats;
  stats.count = n;

  if (n == 0) {
    stats.min = stats.max = stats.mean = stats.std_dev = 0.0;
    return stats;
  }

  stats.min = data[0];
  stats.max = data[0];
  double sum = 0.0;

  for (int i = 0; i < n; i++) {
    if (data[i] < stats.min) stats.min = data[i];
    if (data[i] > stats.max) stats.max = data[i];
    sum += data[i];
  }

  stats.mean = sum / n;

  // Compute standard deviation
  double var_sum = 0.0;
  for (int i = 0; i < n; i++) {
    double diff = data[i] - stats.mean;
    var_sum += diff * diff;
  }

  stats.std_dev = sqrt(var_sum / n);

  return stats;
}

// Print statistics
void print_stats(const char* name, Statistics stats) {
  printf("  %s:\n", name);
  printf("    Count:   %d\n", stats.count);
  printf("    Min:     %g\n", stats.min);
  printf("    Max:     %g\n", stats.max);
  printf("    Mean:    %g\n", stats.mean);
  printf("    Std Dev: %g\n", stats.std_dev);
}

// L1, L2, Linf norms
double l1_norm(double* data, int n) {
  double sum = 0.0;
  for (int i = 0; i < n; i++)
    sum += fabs(data[i]);
  return sum / n;
}

double l2_norm(double* data, int n) {
  double sum = 0.0;
  for (int i = 0; i < n; i++)
    sum += data[i] * data[i];
  return sqrt(sum / n);
}

double linf_norm(double* data, int n) {
  double max_val = 0.0;
  for (int i = 0; i < n; i++) {
    if (fabs(data[i]) > max_val)
      max_val = fabs(data[i]);
  }
  return max_val;
}

// Convergence rate calculation
// error[i] = C * h[i]^p
// log(error[i]) = log(C) + p * log(h[i])
double convergence_rate(double* h, double* error, int n) {
  if (n < 2) return 0.0;

  // Linear regression on log-log data
  double sum_log_h = 0.0;
  double sum_log_e = 0.0;
  double sum_log_h_sq = 0.0;
  double sum_log_h_log_e = 0.0;

  for (int i = 0; i < n; i++) {
    double log_h = log(h[i]);
    double log_e = log(error[i]);

    sum_log_h += log_h;
    sum_log_e += log_e;
    sum_log_h_sq += log_h * log_h;
    sum_log_h_log_e += log_h * log_e;
  }

  double p = (n * sum_log_h_log_e - sum_log_h * sum_log_e) /
             (n * sum_log_h_sq - sum_log_h * sum_log_h);

  return p;
}

// Progress bar
void print_progress(int current, int total, int bar_width) {
  double progress = (double)current / total;
  int filled = (int)(progress * bar_width);

  printf("\r[");
  for (int i = 0; i < bar_width; i++) {
    if (i < filled)
      printf("=");
    else if (i == filled)
      printf(">");
    else
      printf(" ");
  }
  printf("] %d/%d (%.1f%%)", current, total, progress * 100.0);
  fflush(stdout);

  if (current == total)
    printf("\n");
}

// File I/O helpers
int write_array_to_file(const char* filename, double* data, int n) {
  FILE* f = fopen(filename, "w");
  if (!f) return -1;

  for (int i = 0; i < n; i++)
    fprintf(f, "%g\n", data[i]);

  fclose(f);
  return 0;
}

int read_array_from_file(const char* filename, double* data, int max_n) {
  FILE* f = fopen(filename, "r");
  if (!f) return -1;

  int count = 0;
  while (count < max_n && fscanf(f, "%lf", &data[count]) == 1)
    count++;

  fclose(f);
  return count;
}

// Reference data comparison
typedef struct {
  double* x;
  double* y;
  int n;
} ReferenceData;

ReferenceData load_reference_data(const char* filename) {
  ReferenceData ref;
  ref.x = NULL;
  ref.y = NULL;
  ref.n = 0;

  FILE* f = fopen(filename, "r");
  if (!f) {
    printf(ANSI_COLOR_YELLOW "Warning: Could not open reference file: %s\n"
           ANSI_COLOR_RESET, filename);
    return ref;
  }

  // Count lines
  int count = 0;
  double dummy1, dummy2;
  while (fscanf(f, "%lf %lf", &dummy1, &dummy2) == 2)
    count++;

  if (count == 0) {
    fclose(f);
    return ref;
  }

  // Allocate and read
  ref.x = (double*)malloc(count * sizeof(double));
  ref.y = (double*)malloc(count * sizeof(double));
  ref.n = count;

  rewind(f);
  for (int i = 0; i < count; i++)
    fscanf(f, "%lf %lf", &ref.x[i], &ref.y[i]);

  fclose(f);

  printf("  Loaded %d reference data points from %s\n", count, filename);

  return ref;
}

void free_reference_data(ReferenceData* ref) {
  if (ref->x) free(ref->x);
  if (ref->y) free(ref->y);
  ref->x = ref->y = NULL;
  ref->n = 0;
}

#endif // TEST_UTILS_H
