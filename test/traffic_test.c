/* Desktop check of the traffic (issue 45): start every course as a time attack with the
 * traffic on, drive it with the arcade's own pilot, and print how many traffic cars the port
 * sees, how many of them face against the player (the opposite carriageway), and the six
 * opposite-carriageway slots of yt::COnComerManager.
 *
 * cc -O1 -std=gnu99 -w -ffp-contract=off -DRC_FPT=double -Isrc -Isrc/phys
 *    test/traffic_test.c test/traffic_net_stub.c src/car.c src/phys/rc_runtime.c
 *    src/phys/phys_rc.c -lm    (WMMT_PILOT=1 to drive it)
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/car.h"

static float yaw(const float *M) { /* car.c's convention: c->yaw = atan2f(M[8], M[10]) */
  return atan2f(M[8], M[10]) * 57.29578f;
}

int main(int argc, char **argv) {
  char err[256];
  if (car_init(argc > 1 ? argv[1] : ".", err, sizeof err)) {
    puts(err);
    return 1;
  }
  int frames = argc > 2 ? atoi(argv[2]) : 900;
  car_set_ta_traffic(1);
  car_state c;
  for (int course = 0; course < CAR_COURSES; course++) {
    if (car_start(&c, course, 0, NULL)) {
      printf("course %d failed\n", course);
      return 1;
    }
    /* settle through the countdown */
    car_input in = {0};
    for (int i = 0; i < 240; i++) car_update(&c, &in);
    int n_same = 0, n_opp = 0, n_seen = 0, max_on = 0, n_oc = 0, max_oc = 0, oc_against = 0, oc_models[4] = {0};
    float first_opp_yaw = 0.0f;
    for (int i = 0; i < frames; i++) {
      in.throttle = 1.0f;
      car_update(&c, &in);
      int on = 0;
      for (int k = 0; k < c.n_traffic && k < CAR_TRAFFIC; k++) {
        const car_traffic *t = &c.traffic[k];
        n_seen++;
        on++;
        float dy = yaw(t->body) - c.yaw * 57.29578f;
        while (dy > 180.0f) dy -= 360.0f;
        while (dy < -180.0f) dy += 360.0f;
        if (fabsf(dy) > 90.0f) {
          n_opp++;
          if (!first_opp_yaw) first_opp_yaw = dy;
        } else {
          n_same++;
        }
      }
      if (on > max_on) max_on = on;
      if (c.n_oncomer > max_oc) max_oc = c.n_oncomer;
      for (int k = 0; k < c.n_oncomer && k < CAR_ONCOMER; k++) {
        const car_oncomer *o = &c.oncomer[k];
        n_oc++;
        if (o->model >= 0 && o->model < 4) oc_models[o->model]++;
        float dy = yaw(o->body) - c.yaw * 57.29578f;
        while (dy > 180.0f) dy -= 360.0f;
        while (dy < -180.0f) dy += 360.0f;
        if (fabsf(dy) > 90.0f) oc_against++;
      }
    }
    printf("course %2d: traffic %5d (same %5d against %5d, max %2d)  ONCOMER %5d (against %5d, max %d, models %d/%d/%d/%d)",
           course, n_seen, n_same, n_opp, max_on, n_oc, oc_against, max_oc,
           oc_models[0], oc_models[1], oc_models[2], oc_models[3]);
    if (n_opp) printf("  (first against rel-yaw %.0f deg)", first_opp_yaw);
    printf("\n");
  }
  return 0;
}
