#ifndef TESTS_H
#define TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#if DRIVER_TEST
void Driver_Test(void);
#elif ADAPTATION_TEST
void Adaptation_Test(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TESTS_H */