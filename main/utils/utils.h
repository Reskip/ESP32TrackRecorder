
#include <stdio.h>

#ifndef UTILS
#define UTILS

#define UTIL_TAG "Util"

struct Context;

double haversine_distance(double lat1, double lon1, double lat2, double lon2);
double calculate_course(double last_lat, double last_lon, double lat, double lon);
void init_gpx(FILE *fp);
void new_track_seg(FILE *fp);
void close_gpx(FILE *fp);
int get_battery_level(int battery);
void monitorSystemResources(Context &context);
#endif