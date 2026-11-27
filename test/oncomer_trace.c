/* The port's side of the OnComer comparison (issue 45): the same drive on course 0, printing
 * every 10th frame the six opposite-carriageway slots' body matrices as hex, one field a slot
 * (a '-' when that slot has no car), in the same format as ref_oncomer_trace.py, so the two
 * can be diffed.
 *
 * cc -O1 -std=gnu99 -w -ffp-contract=off -DRC_FPT=double -Isrc -Isrc/phys
 *    test/oncomer_trace.c test/traffic_net_stub.c src/car.c src/phys/rc_runtime.c
 *    src/phys/phys_rc.c -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/car.h"

int main(int argc, char **argv) {
  char err[256];
  if (car_init(argc > 1 ? argv[1] : ".", err, sizeof err)) {
    puts(err);
    return 1;
  }
  car_set_ta_traffic(1);
  car_state c;
  if (car_start(&c, 0, 0, NULL)) {
    puts("start failed");
    return 1;
  }
  car_input in = {0};
  for (int i = 0; i < 240; i++) car_update(&c, &in);
  for (int i = 0; i < 600; i++) {
    in.throttle = 1.0f;
    car_update(&c, &in);
    if (i % 10 == 0) {
      char hex[CAR_ONCOMER][129];
      int have[CAR_ONCOMER] = {0};
      for (int k = 0; k < c.n_oncomer && k < CAR_ONCOMER; k++) {
        const car_oncomer *o = &c.oncomer[k];
        const unsigned char *b = (const unsigned char *)o->body;
        if (o->slot < 0 || o->slot >= CAR_ONCOMER) continue;
        have[o->slot] = 1;
        for (int j = 0; j < 64; j++) sprintf(hex[o->slot] + 2 * j, "%02x", b[j]);
      }
      printf("f%03d", i);
      for (int k = 0; k < CAR_ONCOMER; k++) printf(" %s", have[k] ? hex[k] : "-");
      printf("\n");
    }
  }
  return 0;
}
