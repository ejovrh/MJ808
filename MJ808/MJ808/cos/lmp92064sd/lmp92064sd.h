#ifndef LMP92064SD_H_
#define LMP92064SD_H_

typedef struct lmp92064sd_t												// lmp92064sd_t struct describing the port expander as a whole
{

} lmp92064sd_t __attribute__((aligned(8)));

lmp92064sd_t *lmp92064sd_ctor();										// lmp92064sd_t object constructor - does function pointer & hardware initialization

#endif /* LMP92064SD_H_ */