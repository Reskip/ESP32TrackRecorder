
#include <stdio.h>

#ifndef UTILS
#define UTILS

double haversine_distance(double lat1, double lon1, double lat2, double lon2);
void init_gpx(FILE *fp);
void close_gpx(FILE *fp);

#endif