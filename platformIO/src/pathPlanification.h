double calculate_azimuth(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon);
void define_no_go_zone(double wind_vane, double *min_angle, double *max_angle);
int is_in_no_go_zone(double azimuth, double min_angle, double max_angle);
double calculate_direction(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon, double horizontal_tilt, double vertical_tilt, double compass, double wind_vane);