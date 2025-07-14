
#include <stdio.h>

#ifndef UTILS
#define UTILS

double haversine_distance(double lat1, double lon1, double lat2, double lon2);
void init_gpx(FILE *fp);
void new_track_seg(FILE *fp);
void close_gpx(FILE *fp);
int get_battery_level(int battery);

#endif