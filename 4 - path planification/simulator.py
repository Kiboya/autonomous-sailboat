import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button
from math import sin, cos, radians, degrees, pi, atan2
import time

# ------------- Base PathPlanner ------------- #
class BasePathPlanner:
    """
    Base class that all path planning algorithms must inherit from.
    
    Provides common functionality for sailboat navigation including azimuth calculation,
    distance measurement, no-go zone definition, and boat speed estimation from polar curves.
    """
    
    @staticmethod
    def calculate_azimuth(lat1, lon1, lat2, lon2):
        """
        Calculate the azimuth (bearing) from point 1 to point 2.
        
        Args:
            lat1, lon1: Starting position in decimal degrees
            lat2, lon2: Ending position in decimal degrees
            
        Returns:
            float: Azimuth in degrees (0-360, where 0 is North)
        """
        dLon = (lon2 - lon1) * (pi / 180.0)
        lat1_rad = lat1*(pi/180.0)
        lat2_rad = lat2*(pi/180.0)
        
        y = sin(dLon)*cos(lat2_rad)
        x = cos(lat1_rad)*sin(lat2_rad) - sin(lat1_rad)*cos(lat2_rad)*cos(dLon)
        az = atan2(y,x)*(180.0/pi)
        return (az+360.0)%360.0
    
    @staticmethod
    def calculate_distance(lat1, lon1, lat2, lon2):
        """
        Calculate the great circle distance between two points using the haversine formula.
        
        Args:
            lat1, lon1: First position in decimal degrees
            lat2, lon2: Second position in decimal degrees
            
        Returns:
            float: Distance in meters
        """
        R=6371000  # Earth radius in meters
        lat1_rad=np.radians(lat1)
        lat2_rad=np.radians(lat2)
        dLat=np.radians(lat2-lat1)
        dLon=np.radians(lon2-lon1)
        a = (np.sin(dLat/2)**2 + 
             np.cos(lat1_rad)*np.cos(lat2_rad)*np.sin(dLon/2)**2)
        c=2*atan2(np.sqrt(a),np.sqrt(1-a))
        return R*c
    
    @staticmethod
    def define_no_go_zone(wind_direction, wind_speed=10.0):
        """
        Define the no-go zone where the boat cannot sail directly into the wind.
        
        Args:
            wind_direction: True wind direction in degrees
            wind_speed: Wind speed in m/s (affects zone size)
            
        Returns:
            tuple: (min_angle, max_angle) defining the no-go zone boundaries
        """
        NO_GO_ZONE_ANGLE = 45.0  # Base no-go zone half-angle
        wind_abs = (wind_direction+360.0)%360
        adjusted_no_go = NO_GO_ZONE_ANGLE
        
        # Adjust no-go zone size based on wind conditions
        if wind_speed>15:
            adjusted_no_go = NO_GO_ZONE_ANGLE * 1.2  # Wider no-go in strong winds
        elif wind_speed<5:
            adjusted_no_go = NO_GO_ZONE_ANGLE * 0.8  # Narrower no-go in light winds
        
        min_angle = (wind_abs - adjusted_no_go +360)%360
        max_angle = (wind_abs + adjusted_no_go)%360
        return min_angle, max_angle
    
    @staticmethod
    def is_in_no_go_zone(azimuth, min_angle, max_angle):
        """
        Check if a heading falls within the no-go zone.
        
        Args:
            azimuth: Heading to check in degrees
            min_angle: Minimum angle of no-go zone
            max_angle: Maximum angle of no-go zone
            
        Returns:
            bool: True if heading is in no-go zone
        """
        if min_angle<max_angle:
            return min_angle <= azimuth <= max_angle
        else:  # No-go zone crosses North (0°)
            return azimuth >= min_angle or azimuth <= max_angle
            
    @staticmethod
    def get_boat_speed_from_polars(wind_angle, wind_speed):
        """
        Calculate boat speed based on simplified polar curves.
        
        Args:
            wind_angle: Apparent wind angle relative to boat heading
            wind_speed: Wind speed in m/s
            
        Returns:
            float: Estimated boat speed in m/s
        """
        abs_wind_angle = abs(wind_angle)
        while abs_wind_angle > 180:
            abs_wind_angle = 360 - abs_wind_angle
            
        # Define speed factors based on wind angle
        if abs_wind_angle < 35:  # Too close to wind
            return 0.0
        elif abs_wind_angle < 50:  # Close hauled
            return 0.5 * wind_speed * 0.4
        elif abs_wind_angle < 90:  # Reaching
            return 0.8 * wind_speed * 0.5
        elif abs_wind_angle < 150:  # Broad reach - fastest point of sail
            return 1.0 * wind_speed * 0.6
        else:  # Running
            return 0.7 * wind_speed * 0.5
    
    def calculate_direction(self, boat_lat, boat_lon, wpt_lat, wpt_lon,
                           compass, wind_vane, wind_speed=10.0, current_time=None):
        """
        Calculate the optimal heading - must be implemented by subclasses.
        
        Args:
            boat_lat, boat_lon: Current boat position
            wpt_lat, wpt_lon: Target waypoint position
            compass: Boat's compass heading
            wind_vane: Relative wind direction from wind vane
            wind_speed: Current wind speed
            current_time: Current simulation time
            
        Returns:
            float: Optimal heading in degrees
        """
        raise NotImplementedError("Subclasses must implement calculate_direction")
    
    def calculate_tack_path(self, boat_x, boat_y, boat_lat, boat_lon, wpt_lat, wpt_lon,
                           wind_direction, wind_speed, optimal_heading, num_segments=4):
        """
        Calculate a visualization path showing the planned route to waypoint.
        
        Args:
            boat_x, boat_y: Current boat position in local coordinates
            boat_lat, boat_lon: Current boat position in lat/lon
            wpt_lat, wpt_lon: Target waypoint position
            wind_direction: Current wind direction
            wind_speed: Current wind speed
            optimal_heading: Calculated optimal heading
            num_segments: Number of tack segments to visualize
            
        Returns:
            tuple: (tack_points_x, tack_points_y) for path visualization
        """
        # Calculate direct bearing to waypoint
        initial_azimuth = self.calculate_azimuth(boat_lat, boat_lon, wpt_lat, wpt_lon)
        min_angle, max_angle = self.define_no_go_zone(wind_direction, wind_speed)
        
        # Check if direct path is possible
        direct_path_possible = not self.is_in_no_go_zone(initial_azimuth, min_angle, max_angle)
        
        # Initialize path with current boat position
        current_pos = (boat_x, boat_y)
        tack_points_x = [current_pos[0]]
        tack_points_y = [current_pos[1]]
        
        # Earth parameters for coordinate conversion
        earth_radius = 6371000
        meters_per_degree_lat = earth_radius * np.pi / 180
        meters_per_degree_lon = meters_per_degree_lat * np.cos(np.radians(boat_lat))
        
        # Convert waypoint to local coordinates
        dx_to_waypoint = (wpt_lon - boat_lon) * meters_per_degree_lon
        dy_to_waypoint = (wpt_lat - boat_lat) * meters_per_degree_lat
        waypoint_x = boat_x + dx_to_waypoint
        waypoint_y = boat_y + dy_to_waypoint
        
        # Draw straight line if direct path is clear
        if direct_path_possible:
            tack_points_x.append(waypoint_x)
            tack_points_y.append(waypoint_y)
            return tack_points_x, tack_points_y
        
        # Calculate zigzag tacking path
        current_tack_dir = optimal_heading
        
        for i in range(num_segments):
            # Calculate segment distance (decreasing as we approach waypoint)
            segment_dist = 100 * (num_segments - i) / num_segments
            
            # Calculate next position along current tack
            heading_rad = np.radians(current_tack_dir)
            next_x = current_pos[0] + segment_dist * np.sin(heading_rad)
            next_y = current_pos[1] + segment_dist * np.cos(heading_rad)
            
            # Add point to path
            tack_points_x.append(next_x)
            tack_points_y.append(next_y)
            
            # Update position for next iteration
            current_pos = (next_x, next_y)
            
            # Switch tack for next segment
            if i < num_segments - 1:
                if abs((current_tack_dir - wind_direction + 180) % 360 - 180) < 90:  # On port tack
                    current_tack_dir = (max_angle + 10) % 360  # Switch to starboard
                else:
                    current_tack_dir = (min_angle - 10) % 360  # Switch to port
        
        # Ensure final point is the waypoint
        tack_points_x.append(waypoint_x)
        tack_points_y.append(waypoint_y)
        
        return tack_points_x, tack_points_y

# ------------- RuleBasedPathPlanner ------------- #
class RuleBasedPathPlanner(BasePathPlanner):
    """
    Traditional rule-based path planning algorithm.
    
    Uses simple VMG (Velocity Made Good) calculations and tacking logic with
    time and distance based hysteresis to prevent excessive tacking.
    """
    
    def __init__(self):
        """Initialize the rule-based path planner with default parameters."""
        self.NO_GO_ZONE_ANGLE = 45.0  # Half-angle of no-go zone in degrees
        self.THRESHOLD = 10.0  # Angular threshold for tack selection
        self.last_tack_time = 0  # Time of last tack decision
        self.last_tack_direction = None  # Last chosen tack direction
        self.MINIMUM_TACK_INTERVAL = 15.0  # Minimum time between tacks (seconds)
        self.HEADING_CHANGE_THRESHOLD = 1.5  # Minimum change to trigger heading update
        self.HEADING_SMOOTHING_FACTOR = 0.2  # Smoothing factor for heading changes
        self.last_optimal_heading = None  # Previously calculated optimal heading
        self.last_tack_position = (None, None)  # Position of last tack
        self.MINIMUM_TACK_DISTANCE = 20.0  # Minimum distance before allowing new tack
    
    def calculate_direction(self, boat_lat, boat_lon, wpt_lat, wpt_lon,
                           compass, wind_vane, wind_speed=10.0, current_time=None):
        """
        Calculate optimal heading using rule-based tacking logic.
        
        Args:
            boat_lat, boat_lon: Current boat position
            wpt_lat, wpt_lon: Target waypoint position
            compass: Boat's compass heading
            wind_vane: Relative wind direction
            wind_speed: Current wind speed
            current_time: Current simulation time
            
        Returns:
            float: Optimal heading in degrees
        """
        # Calculate bearing to waypoint and absolute wind direction
        azimuth = self.calculate_azimuth(boat_lat, boat_lon, wpt_lat, wpt_lon)
        wind_direction = (compass+wind_vane)%360
        min_angle, max_angle = self.define_no_go_zone(wind_direction, wind_speed)

        # Check if we can sail directly to waypoint
        if not self.is_in_no_go_zone(azimuth, min_angle, max_angle):
            self.last_optimal_heading = azimuth
            return self.last_optimal_heading
        
        # Calculate potential tack headings
        port_tack = (min_angle - self.THRESHOLD+360)%360
        starboard_tack = (max_angle+self.THRESHOLD)%360
        
        # Calculate boat speeds for each tack
        port_speed = self.get_boat_speed_from_polars(port_tack-wind_direction, wind_speed)
        starboard_speed = self.get_boat_speed_from_polars(starboard_tack-wind_direction, wind_speed)
        
        # Calculate VMG (Velocity Made Good) toward waypoint for each tack
        port_vmg = port_speed * np.cos(np.radians(abs(self.calculate_azimuth_difference(port_tack, azimuth))))
        starboard_vmg = starboard_speed * np.cos(np.radians(abs(self.calculate_azimuth_difference(starboard_tack, azimuth))))

        chosen_tack = None
        
        # Apply tacking constraints to prevent excessive tacking
        if self.last_tack_position[0] is not None and current_time is not None and self.last_tack_direction is not None:
            # Calculate distance since last tack
            distance = self.calculate_distance(
                boat_lat, boat_lon, 
                self.last_tack_position[0], self.last_tack_position[1]
            )
            
            # Maintain current tack if haven't traveled far enough
            if distance < self.MINIMUM_TACK_DISTANCE:
                chosen_tack = self.last_tack_direction
            # Maintain current tack if not enough time has passed
            elif current_time - self.last_tack_time < self.MINIMUM_TACK_INTERVAL:
                chosen_tack = self.last_tack_direction

        # Choose best VMG tack if constraints allow
        if chosen_tack is None and current_time is not None:
            chosen_tack = port_tack if port_vmg > starboard_vmg else starboard_tack
        elif chosen_tack is None:
            # Default behavior without time tracking
            chosen_tack = port_tack if port_vmg > starboard_vmg else starboard_tack
        
        # Update tack tracking if tack changed
        if current_time is not None and chosen_tack != self.last_tack_direction:
            self.last_tack_time = current_time
            self.last_tack_direction = chosen_tack
            self.last_tack_position = (boat_lat, boat_lon)

        # Apply heading smoothing
        if self.last_optimal_heading is None:
            self.last_optimal_heading = chosen_tack
        else:
            # Only smooth significant heading changes
            diff = abs((chosen_tack - self.last_optimal_heading + 180) % 360 - 180)
            if diff > self.HEADING_CHANGE_THRESHOLD:
                angle_change = ((chosen_tack - self.last_optimal_heading + 180) % 360 - 180)
                self.last_optimal_heading = (self.last_optimal_heading + 
                                           angle_change * self.HEADING_SMOOTHING_FACTOR) % 360
            
        return self.last_optimal_heading

    @staticmethod
    def calculate_azimuth_difference(h1, h2):
        """
        Calculate the smallest angle difference between two headings.
        
        Args:
            h1, h2: Headings in degrees
            
        Returns:
            float: Angle difference (-180 to +180 degrees)
        """
        diff = (h1 - h2 + 180) % 360 - 180
        return diff

# ------------- LaylinePathPlanner ------------- #
class LaylinePathPlanner(BasePathPlanner):
    """
    Advanced layline-based path planning algorithm.
    
    Uses sophisticated layline detection and tack confirmation logic to optimize
    upwind sailing performance. Includes dynamic VMG angle calculation and
    conservative tacking decisions to minimize losses.
    """
    
    def __init__(self):
        """Initialize the layline path planner with advanced parameters."""
        super().__init__()

        # Waypoint approach distances for different behaviors
        self.WAYPOINT_ARRIVAL_DISTANCE = 15.0  # Distance to consider "arrived"
        self.WAYPOINT_TIGHT_ARRIVAL_DISTANCE = 7.0  # Distance for tight maneuvering
        
        # Tack state tracking
        self.current_tack_is_port = None  # True: Port tack, False: Starboard tack, None: Direct
        self.last_optimal_heading = None  # Last smoothed heading output
        self.last_raw_optimal_heading = None  # Last raw heading decision before smoothing
        self.last_decision_time = 0  # Time of last confirmed decision
        self.DECISION_COOLDOWN = 4.0  # Cooldown after confirmed decisions (seconds)

        # Tack confirmation system to prevent erratic behavior
        self.pending_tack_is_port = None  # Tack direction being evaluated
        self.tack_confirmation_count = 0  # Number of consecutive confirmations
        self.TACK_CONFIRMATION_THRESHOLD = 5  # Required confirmations to execute tack

        # Heading smoothing parameters
        self.heading_history = []  # History for moving average smoothing
        self.HEADING_HISTORY_SIZE = 3  # Size of smoothing window
        self.TACK_HYSTERESIS_ANGLE_MARGIN = 8  # Extra margin past layline before tacking
        self.HEADING_SMOOTHING_FACTOR = 0.45  # Smoothing factor for gradual changes
        self.NO_GO_ZONE_BUFFER = 7.0  # Buffer for no-go zone checks

    def find_vmg_optimal_tack_angle(self, wind_speed):
        """
        Find the tack angle that maximizes VMG (Velocity Made Good) upwind.
        
        Args:
            wind_speed: Current wind speed in m/s
            
        Returns:
            float: Optimal tack angle relative to true wind in degrees
        """
        best_vmg = -float('inf')
        optimal_angle_to_true_wind = 50.0
        
        # Test range of upwind sailing angles to find optimum
        for angle_deg in range(35, 71, 5):
            boat_speed = self.get_boat_speed_from_polars(angle_deg, wind_speed)
            vmg = boat_speed * np.cos(np.radians(angle_deg))
            
            if vmg > best_vmg:
                best_vmg = vmg
                optimal_angle_to_true_wind = angle_deg
        
        # Add buffers for wind-dependent drift compensation
        base_buffer = 5.0
        wind_buffer = 0.0
        
        if wind_speed > 6.0:
            # Additional buffer in stronger winds for increased drift
            wind_buffer = (wind_speed - 6.0) * 0.8
        
        # Ensure minimum practical angle and apply buffers
        return max(optimal_angle_to_true_wind, 40.0) + base_buffer + wind_buffer

    def _is_point_in_no_go_zone_buffered(self, boat_lat, boat_lon, point_lat, point_lon, wind_direction, wind_speed, no_go_angle_override=None, buffer=0.0):
        """
        Check if a point is in the no-go zone with optional buffer.
        
        Args:
            boat_lat, boat_lon: Current boat position
            point_lat, point_lon: Point to check
            wind_direction: Wind direction in degrees
            wind_speed: Wind speed in m/s
            no_go_angle_override: Optional custom no-go angle
            buffer: Additional buffer to apply
            
        Returns:
            bool: True if point is in buffered no-go zone
        """
        azimuth = self.calculate_azimuth(boat_lat, boat_lon, point_lat, point_lon)
        
        # Determine no-go zone half-angle
        current_no_go_angle = 0
        if no_go_angle_override is not None:
            current_no_go_angle = no_go_angle_override
        else:
            # Extract half-angle from no-go zone boundaries
            temp_min, temp_max = self.define_no_go_zone(wind_direction, wind_speed)
            if temp_max > temp_min:
                current_no_go_angle = (temp_max - temp_min) / 2.0
            else:  # Zone crosses 0 degrees
                current_no_go_angle = (temp_max + (360 - temp_min)) / 2.0
        
        # Apply buffer to effective no-go zone
        effective_no_go_check_angle = current_no_go_angle + buffer
        
        min_angle_check = (wind_direction - effective_no_go_check_angle + 360) % 360
        max_angle_check = (wind_direction + effective_no_go_check_angle) % 360
        
        return self.is_in_no_go_zone(azimuth, min_angle_check, max_angle_check)

    def _apply_heading_smoothing(self, new_raw_heading):
        """
        Apply enhanced smoothing to heading using moving average and adaptive blending.
        
        Args:
            new_raw_heading: Raw heading decision in degrees
            
        Returns:
            float: Smoothed heading in degrees
        """
        if new_raw_heading is None:
            return self.last_optimal_heading
        
        # Maintain heading history for moving average
        self.heading_history.append(new_raw_heading)
        if len(self.heading_history) > self.HEADING_HISTORY_SIZE:
            self.heading_history.pop(0)
        
        # Calculate circular mean of heading history
        if not self.heading_history:
            current_avg_heading = new_raw_heading
        else:
            sin_sum = sum(np.sin(np.radians(hdg)) for hdg in self.heading_history)
            cos_sum = sum(np.cos(np.radians(hdg)) for hdg in self.heading_history)
            current_avg_heading = np.degrees(np.arctan2(sin_sum, cos_sum)) % 360
        
        # Initialize or update smoothed heading
        if self.last_optimal_heading is None:
            self.last_optimal_heading = current_avg_heading
        else:
            # Calculate shortest angular difference
            angle_diff = ((current_avg_heading - self.last_optimal_heading + 180) % 360) - 180
            
            # Adaptive smoothing based on magnitude of change
            smoothing_factor_to_use = self.HEADING_SMOOTHING_FACTOR
            
            if abs(angle_diff) < 5.0:
                # Small changes - moderate smoothing to reduce oscillations
                smoothing_factor_to_use = 0.25
            elif abs(angle_diff) < 15.0:
                # Medium changes - slightly less smoothing
                smoothing_factor_to_use = 0.35
            elif abs(angle_diff) > 45.0:
                # Large changes (tacks) - faster response
                smoothing_factor_to_use = 0.65
            
            # Apply adaptive smoothing
            self.last_optimal_heading = (self.last_optimal_heading + angle_diff * smoothing_factor_to_use) % 360
        
        return self.last_optimal_heading

    def calculate_direction(self, boat_lat, boat_lon, wpt_lat, wpt_lon,
                            compass, wind_vane_relative, wind_speed=10.0, current_time=None):
        """
        Calculate smoothed optimal heading using layline-based algorithm.
        
        Args:
            boat_lat, boat_lon: Current boat position
            wpt_lat, wpt_lon: Target waypoint position
            compass: Boat's compass heading
            wind_vane_relative: Relative wind direction from wind vane
            wind_speed: Current wind speed
            current_time: Current simulation time
            
        Returns:
            float: Smoothed optimal heading in degrees
        """
        # Calculate raw heading decision
        raw_heading_decision = self._calculate_raw_direction(boat_lat, boat_lon, wpt_lat, wpt_lon,
                                                             compass, wind_vane_relative, wind_speed, current_time)
        self.last_raw_optimal_heading = raw_heading_decision
        
        # Apply smoothing and return result
        smoothed_heading = self._apply_heading_smoothing(raw_heading_decision)
        return smoothed_heading

    def _calculate_raw_direction(self, boat_lat, boat_lon, wpt_lat, wpt_lon,
                            compass, wind_vane_relative, wind_speed=10.0, current_time=None):
        """
        Core layline-based decision logic before smoothing.
        
        Args:
            boat_lat, boat_lon: Current boat position
            wpt_lat, wpt_lon: Target waypoint position
            compass: Boat's compass heading
            wind_vane_relative: Relative wind direction
            wind_speed: Current wind speed
            current_time: Current simulation time
            
        Returns:
            float: Raw optimal heading decision in degrees
        """
        # Calculate key navigation parameters
        vmg_tack_angle = self.find_vmg_optimal_tack_angle(wind_speed)
        azimuth_to_wpt = self.calculate_azimuth(boat_lat, boat_lon, wpt_lat, wpt_lon)
        wind_direction_abs = (compass + wind_vane_relative) % 360
        port_tack_target_hdg = (wind_direction_abs - vmg_tack_angle + 360) % 360
        starboard_tack_target_hdg = (wind_direction_abs + vmg_tack_angle) % 360
        distance_to_wpt = self.calculate_distance(boat_lat, boat_lon, wpt_lat, wpt_lon)
        
        # Initialize tracking variables on first run
        if not hasattr(self, 'initial_lat') or self.initial_lat is None:
            self.initial_lat = boat_lat
            self.initial_lon = boat_lon
            self.initial_time = current_time if current_time is not None else 0.0
            self.initial_tack_chosen = False
        
        # Calculate progress from initial position
        distance_traveled = self.calculate_distance(boat_lat, boat_lon, self.initial_lat, self.initial_lon)
        time_elapsed = (current_time - self.initial_time) if current_time is not None else float('inf')
        
        # Check if direct sailing is possible (outside buffered no-go zone)
        practical_direct_sailing_no_go_half_angle = RuleBasedPathPlanner().NO_GO_ZONE_ANGLE + self.NO_GO_ZONE_BUFFER
        
        if not self._is_point_in_no_go_zone_buffered(boat_lat, boat_lon, wpt_lat, wpt_lon,
                                                wind_direction_abs, wind_speed,
                                                no_go_angle_override=practical_direct_sailing_no_go_half_angle):
            # Clear path to waypoint - reset tack state and sail direct
            self.current_tack_is_port = None
            self.pending_tack_is_port = None
            self.tack_confirmation_count = 0
            if current_time: self.last_decision_time = current_time
            return azimuth_to_wpt
        
        # Apply decision cooldown to prevent rapid changes
        if current_time is not None and self.last_decision_time is not None and \
                (current_time - self.last_decision_time < self.DECISION_COOLDOWN):
            return self.last_raw_optimal_heading
        
        # Initial tack selection logic
        if self.current_tack_is_port is None:
            # For first tack, choose based on minimal heading change from current position
            if not self.initial_tack_chosen:
                port_hdg_diff = abs(((port_tack_target_hdg - compass + 180) % 360) - 180)
                stbd_hdg_diff = abs(((starboard_tack_target_hdg - compass + 180) % 360) - 180)
                self.current_tack_is_port = (port_hdg_diff < stbd_hdg_diff)
                self.initial_tack_chosen = True
                if current_time: self.last_decision_time = current_time
                return port_tack_target_hdg if self.current_tack_is_port else starboard_tack_target_hdg
            
            # Normal case - choose tack that points closer to waypoint
            angle_diff_port = abs(((port_tack_target_hdg - azimuth_to_wpt + 180) % 360) - 180)
            angle_diff_starboard = abs(((starboard_tack_target_hdg - azimuth_to_wpt + 180) % 360) - 180)
            self.current_tack_is_port = (angle_diff_port < angle_diff_starboard)
            if current_time: self.last_decision_time = current_time
            return port_tack_target_hdg if self.current_tack_is_port else starboard_tack_target_hdg
        
        # Prevent premature tacking at beginning of route
        MINIMUM_INITIAL_DISTANCE = 15.0  # Meters to travel before first tack
        MINIMUM_INITIAL_TIME = 5.0  # Seconds before allowing first tack
        
        if distance_traveled < MINIMUM_INITIAL_DISTANCE or time_elapsed < MINIMUM_INITIAL_TIME:
            # Too early to consider tacking
            return port_tack_target_hdg if self.current_tack_is_port else starboard_tack_target_hdg
        
        # Layline crossing detection with enhanced conservative margins
        bearing_to_wpt = self.calculate_azimuth(boat_lat, boat_lon, wpt_lat, wpt_lon)
        relative_wpt_bearing_to_wind = ((bearing_to_wpt - wind_direction_abs + 180) % 360) - 180
        
        # Calculate dynamic layline margins based on conditions
        wind_push_factor = min((wind_speed - 5.0) * 2.5, 20.0) if wind_speed > 5.0 else 0.0
        distance_factor = min(15.0, max(5.0, distance_to_wpt / 10.0))
        
        # Effective layline check angle includes base angle plus margins
        effective_layline_check_angle = vmg_tack_angle + self.TACK_HYSTERESIS_ANGLE_MARGIN + \
                                    max(wind_push_factor, distance_factor)
        
        # Require more confirmations when far from waypoint
        far_threshold = 50.0  # meters
        required_confirmation = self.TACK_CONFIRMATION_THRESHOLD
        if distance_to_wpt > far_threshold:
            required_confirmation = int(self.TACK_CONFIRMATION_THRESHOLD * 1.5)
        
        # Evaluate tack proposal based on layline crossing
        propose_tack_now = False
        newly_proposed_tack_is_port = self.current_tack_is_port
        
        # Check if we've crossed the appropriate layline
        if self.current_tack_is_port:  # Currently on Port, check for tack to Starboard
            if relative_wpt_bearing_to_wind > effective_layline_check_angle:
                propose_tack_now = True
                newly_proposed_tack_is_port = False
        else:  # Currently on Starboard, check for tack to Port
            if relative_wpt_bearing_to_wind < -effective_layline_check_angle:
                propose_tack_now = True
                newly_proposed_tack_is_port = True
        
        # Manage tack confirmation process
        if propose_tack_now:
            if self.pending_tack_is_port is None or self.pending_tack_is_port != newly_proposed_tack_is_port:
                # New tack proposal - start confirmation count
                self.pending_tack_is_port = newly_proposed_tack_is_port
                self.tack_confirmation_count = 1
            elif self.pending_tack_is_port == newly_proposed_tack_is_port:
                # Same proposal - increment confirmation
                self.tack_confirmation_count += 1
            
            # Execute tack if sufficiently confirmed
            if self.tack_confirmation_count >= required_confirmation:
                self.current_tack_is_port = self.pending_tack_is_port
                self.pending_tack_is_port = None
                self.tack_confirmation_count = 0
                if current_time: self.last_decision_time = current_time
        else:
            # No tack proposed - reset pending state
            if self.pending_tack_is_port is not None:
                self.pending_tack_is_port = None
                self.tack_confirmation_count = 0
        
        # Return appropriate heading based on current tack
        return port_tack_target_hdg if self.current_tack_is_port else starboard_tack_target_hdg

    def calculate_laylines(self, wpt_x, wpt_y, wind_direction_abs, wind_speed):
        """
        Calculate laylines extending upwind from waypoint for visualization.
        
        Args:
            wpt_x, wpt_y: Waypoint position in local coordinates
            wind_direction_abs: Absolute wind direction
            wind_speed: Wind speed
            
        Returns:
            tuple: (port_x_coords, port_y_coords, starboard_x_coords, starboard_y_coords)
        """
        # Use VMG-optimal angle for layline calculation
        tack_angle = self.find_vmg_optimal_tack_angle(wind_speed)

        # Calculate layline directions (where boats would approach from)
        port_layline_course_angle = (wind_direction_abs - tack_angle + 360) % 360
        starboard_layline_course_angle = (wind_direction_abs + tack_angle) % 360
        
        line_length = 200  # Visual length in meters
        
        # Draw lines extending upwind from waypoint
        port_dx = line_length * np.sin(np.radians(port_layline_course_angle + 180))
        port_dy = line_length * np.cos(np.radians(port_layline_course_angle + 180))
        port_x_coords = [wpt_x, wpt_x + port_dx]
        port_y_coords = [wpt_y, wpt_y + port_dy]
        
        starboard_dx = line_length * np.sin(np.radians(starboard_layline_course_angle + 180))
        starboard_dy = line_length * np.cos(np.radians(starboard_layline_course_angle + 180))
        starboard_x_coords = [wpt_x, wpt_x + starboard_dx]
        starboard_y_coords = [wpt_y, wpt_y + starboard_dy]
        
        return port_x_coords, port_y_coords, starboard_x_coords, starboard_y_coords

    def calculate_tack_path(self, boat_x, boat_y, boat_lat, boat_lon, wpt_lat, wpt_lon,
                        wind_direction_abs, wind_speed, optimal_heading_smoothed, num_segments=2):
        """
        Generate visualization path that represents realistic sailing route.
        
        Args:
            boat_x, boat_y: Current boat position in local coordinates
            boat_lat, boat_lon: Current boat position in lat/lon
            wpt_lat, wpt_lon: Target waypoint
            wind_direction_abs: Absolute wind direction
            wind_speed: Wind speed
            optimal_heading_smoothed: Current smoothed heading
            num_segments: Number of path segments
            
        Returns:
            tuple: (path_x, path_y) coordinates for visualization
        """
        path_x = [boat_x]
        path_y = [boat_y]
        
        # Use raw target heading for path calculation
        current_raw_target_heading = self.last_raw_optimal_heading if self.last_raw_optimal_heading is not None else optimal_heading_smoothed
        
        # Handle direct sailing case
        if self.current_tack_is_port is None and current_raw_target_heading is not None:
            # Convert waypoint to cartesian coordinates
            earth_radius = 6371000
            meters_per_degree_lat = earth_radius * np.pi / 180
            meters_per_degree_lon_at_boat = meters_per_degree_lat * np.cos(np.radians(boat_lat))
            dx_to_wpt_cart = (wpt_lon - boat_lon) * meters_per_degree_lon_at_boat
            dy_to_wpt_cart = (wpt_lat - boat_lat) * meters_per_degree_lat
            waypoint_x_cartesian = boat_x + dx_to_wpt_cart
            waypoint_y_cartesian = boat_y + dy_to_wpt_cart
            
            # Check if direct path actually enters no-go zone
            min_angle, max_angle = self.define_no_go_zone(wind_direction_abs, wind_speed)
            azimuth = self.calculate_azimuth(boat_lat, boat_lon, wpt_lat, wpt_lon)
            
            if self.is_in_no_go_zone(azimuth, min_angle, max_angle):
                # Show two-segment path instead of problematic direct route
                vmg_angle = self.find_vmg_optimal_tack_angle(wind_speed)
                
                # Choose better tack based on bearing to waypoint
                port_tack_hdg = (wind_direction_abs - vmg_angle + 360) % 360
                starboard_tack_hdg = (wind_direction_abs + vmg_angle) % 360
                
                port_diff = abs(((port_tack_hdg - azimuth + 180) % 360) - 180)
                starboard_diff = abs(((starboard_tack_hdg - azimuth + 180) % 360) - 180)
                
                tack_hdg = port_tack_hdg if port_diff < starboard_diff else starboard_tack_hdg
                
                # Calculate first leg on chosen tack
                heading_rad = np.radians(tack_hdg)
                dist_to_wpt = self.calculate_distance(boat_lat, boat_lon, wpt_lat, wpt_lon)
                first_leg_dist = dist_to_wpt * 0.7
                
                tack_x = boat_x + first_leg_dist * np.sin(heading_rad)
                tack_y = boat_y + first_leg_dist * np.cos(heading_rad)
                
                path_x.append(tack_x)
                path_y.append(tack_y)
                path_x.append(waypoint_x_cartesian)
                path_y.append(waypoint_y_cartesian)
            else:
                # Direct path is clear
                path_x.append(waypoint_x_cartesian)
                path_y.append(waypoint_y_cartesian)
        
        elif current_raw_target_heading is not None:
            # Tacking case - show current leg then leg to waypoint
            dist_to_wpt = self.calculate_distance(boat_lat, boat_lon, wpt_lat, wpt_lon)
            
            # Calculate heuristic length for current tack leg
            first_leg_dist = dist_to_wpt * 0.6 if dist_to_wpt > 20 else dist_to_wpt * 0.4
            
            heading_rad = np.radians(current_raw_target_heading)
            tack_intersection_x_est = boat_x + first_leg_dist * np.sin(heading_rad)
            tack_intersection_y_est = boat_y + first_leg_dist * np.cos(heading_rad)
            
            path_x.append(tack_intersection_x_est)
            path_y.append(tack_intersection_y_est)
            
            # Calculate waypoint position in cartesian coordinates
            earth_radius = 6371000
            meters_per_degree_lat = earth_radius * np.pi / 180
            meters_per_degree_lon_at_boat = meters_per_degree_lat * np.cos(np.radians(boat_lat))
            dx_to_wpt_cart = (wpt_lon - boat_lon) * meters_per_degree_lon_at_boat
            dy_to_wpt_cart = (wpt_lat - boat_lat) * meters_per_degree_lat
            waypoint_x_cartesian = boat_x + dx_to_wpt_cart
            waypoint_y_cartesian = boat_y + dy_to_wpt_cart
            
            path_x.append(waypoint_x_cartesian)
            path_y.append(waypoint_y_cartesian)
        
        return path_x, path_y

# ------------- BoatProperties ------------- #
class BoatProperties:
    """
    Static class containing boat performance characteristics and speed calculations.
    
    Defines the boat's performance envelope based on wind conditions and sail settings.
    """
    
    B_RIG_FACTOR = 0.9  # Overall boat efficiency factor
    
    @staticmethod
    def get_speed(true_wind_speed, true_wind_heading, sail):
        """
        Calculate boat speed based on wind conditions and sail setting.
        
        Args:
            true_wind_speed: True wind speed in m/s
            true_wind_heading: True wind heading relative to boat
            sail: Sail setting (0-1, where 0 is fully sheeted in)
            
        Returns:
            float: Boat speed in m/s
        """
        abs_wind_heading = abs(true_wind_heading)
        sail = abs(sail)

        # Speed factor based on wind angle (polar curve approximation)
        if abs_wind_heading < 45:
            speed_factor = 0.1  # Very slow upwind
        elif abs_wind_heading < 90:
            speed_factor = 0.6 + (abs_wind_heading - 45) / 75  # Close hauled to reach
        elif abs_wind_heading < 150:
            speed_factor = 0.8 + (abs_wind_heading - 90) / 300  # Reaching
        else:
            speed_factor = 0.7 - (abs_wind_heading - 150) / 600  # Running
        
        # Sail efficiency calculation
        max_sail_angle = sail * 90  # Convert sail setting to angle
        optimal_angle = min(90, abs(true_wind_heading))
        actual_angle = min(max_sail_angle, optimal_angle)
        sail_eff = 1.0 - 0.5 * abs(optimal_angle - actual_angle) / 90
        
        # Wind speed factor with diminishing returns at high speeds
        wind_factor = np.sqrt(min(true_wind_speed, 15) / 5)
        base_speed = 4 * speed_factor * sail_eff * wind_factor
        
        return base_speed * BoatProperties.B_RIG_FACTOR

# ------------- SailboatEnvironment ------------- #
class SailboatEnvironment:
    """
    Environmental conditions affecting the sailboat.
    
    Manages wind and water current conditions for the simulation.
    """
    
    def __init__(self, wind_speed=5.0, wind_heading=0):
        """
        Initialize environmental conditions.
        
        Args:
            wind_speed: Initial wind speed in m/s
            wind_heading: Initial wind direction in degrees
        """
        self.wind_speed = wind_speed  # Wind speed in m/s
        self.wind_heading = wind_heading  # Wind direction in degrees (meteorological)
        self.water_speed = 0.25  # Water current speed in m/s
        self.water_heading = -135  # Water current direction in degrees
    
    def get_values(self):
        """
        Get current environmental conditions.
        
        Returns:
            dict: Environmental conditions with wind and water data
        """
        return {
            'wind': {'speed': self.wind_speed, 'heading': self.wind_heading},
            'water': {'speed': self.water_speed, 'heading': self.water_heading}
        }
    
    def set_wind(self, speed, heading):
        """
        Update wind conditions.
        
        Args:
            speed: New wind speed in m/s
            heading: New wind direction in degrees
        """
        self.wind_speed = speed
        self.wind_heading = heading

# ------------- SailboatPhysics ------------- #
class SailboatPhysics:
    """
    Physics calculations for sailboat motion and dynamics.
    
    Handles boat motion, forces, and environmental effects using simplified
    but realistic sailing physics models.
    """
    
    # Physics constants for boat behavior
    ROLL_COEFFICIENT = -8.37  # Heel response to apparent wind
    RUDDER_IMPACT = 25  # Rudder effectiveness factor
    RUDDER_A = 0.028  # Rudder speed-dependent coefficient
    RUDDER_B = 0.393  # Rudder base effectiveness
    ROLL_INERTIA = 0.1  # Heel damping factor
    SPEED_INERTIA = 0.3  # Speed change damping when accelerating
    SPEED_MOMENTUM = 0.7  # Speed change damping when decelerating
    WIND_DRIFT_COEFFICIENT = 0.08  # Wind-induced drift factor
    WATER_DRIFT_COEFFICIENT = 0.5  # Water current effect factor
    RUDDER_SLOWING_FACTOR = 0.2  # Speed loss from rudder drag
    
    @staticmethod
    def calculate_roll(dt, aws, awh, current_roll):
        """
        Calculate boat heel angle based on apparent wind.
        
        Args:
            dt: Time step in seconds
            aws: Apparent wind speed
            awh: Apparent wind heading relative to boat
            current_roll: Current heel angle
            
        Returns:
            float: New heel angle in degrees
        """
        inertialess = SailboatPhysics.ROLL_COEFFICIENT * aws * sin(radians(awh))
        new_roll = inertialess * (1 - SailboatPhysics.ROLL_INERTIA * dt) + current_roll * (SailboatPhysics.ROLL_INERTIA * dt)
        return new_roll
    
    @staticmethod
    def calculate_heading_change(dt, boat_speed, rudder):
        """
        Calculate heading change based on rudder input and boat speed.
        
        Args:
            dt: Time step in seconds
            boat_speed: Current boat speed
            rudder: Rudder position (-1 to +1)
            
        Returns:
            float: Heading change in degrees
        """
        return SailboatPhysics.RUDDER_IMPACT * (SailboatPhysics.RUDDER_A * boat_speed + SailboatPhysics.RUDDER_B * rudder) * dt
    
    @staticmethod
    def calculate_speed(dt, env, boat):
        """
        Calculate new boat speed based on environmental conditions and boat state.
        
        Args:
            dt: Time step in seconds
            env: Environmental conditions
            boat: Current boat state dictionary
            
        Returns:
            float: New boat speed in m/s
        """
        tws = boat['trueWind']['speed']
        twh = boat['trueWind']['heading']
        sail = boat['servos']['sail']
        
        # Calculate base speed from boat properties
        base = BoatProperties.get_speed(tws, twh, sail)
        
        # Apply rudder drag
        rudder_impact = SailboatPhysics.RUDDER_SLOWING_FACTOR * abs(boat['servos']['rudder']) * base
        inertialess_speed = base - rudder_impact
        current_speed = boat['velocity']['speed']
        
        # Apply speed damping based on acceleration/deceleration
        if inertialess_speed > current_speed:
            scale = SailboatPhysics.SPEED_INERTIA  # Accelerating
        else:
            scale = SailboatPhysics.SPEED_MOMENTUM  # Decelerating
        
        return inertialess_speed * (1 - scale) + current_speed * scale

    @staticmethod
    def calculate_wind_drift(dt, env, boat):
        """
        Calculate wind-induced drift on the boat.
        
        Args:
            dt: Time step in seconds
            env: Environmental conditions
            boat: Current boat state
            
        Returns:
            dict: Wind drift vector with heading and speed
        """
        # Calculate drift direction (perpendicular to wind)
        wh2n = (boat['attitude']['heading'] + boat['trueWind']['heading'] + 180) % 360
        
        # Exponential increase in drift with wind speed
        wind_factor = boat['trueWind']['speed']
        if wind_factor > 7.0:
            wind_factor = 7.0 + (boat['trueWind']['speed'] - 7.0) * 1.5
            
        return {
            'headingToNorth': wh2n,
            'speed': SailboatPhysics.WIND_DRIFT_COEFFICIENT * wind_factor
        }
    
    @staticmethod
    def calculate_water_drift(dt, env):
        """
        Calculate water current drift.
        
        Args:
            dt: Time step in seconds
            env: Environmental conditions
            
        Returns:
            dict: Water drift vector with heading and speed
        """
        return {
            'headingToNorth': env['water']['heading'],
            'speed': SailboatPhysics.WATER_DRIFT_COEFFICIENT * env['water']['speed']
        }
    
    @staticmethod
    def calculate_apparent_wind(tws, twh, bs, hdg):
        """
        Calculate apparent wind from true wind and boat motion.
        
        Args:
            tws: True wind speed
            twh: True wind heading
            bs: Boat speed
            hdg: Boat heading
            
        Returns:
            dict: Apparent wind data with speed, heading relative to boat, and absolute heading
        """
        # Convert to vector components
        wind_x = tws * sin(radians(twh))
        wind_y = tws * cos(radians(twh))
        boat_x = bs * sin(radians(hdg))
        boat_y = bs * cos(radians(hdg))
        
        # Calculate relative wind vector
        rel_x = wind_x - boat_x
        rel_y = wind_y - boat_y
        
        # Convert back to speed and direction
        aws = np.hypot(rel_x, rel_y)
        awn = degrees(atan2(rel_x, rel_y))
        heading_to_boat = (awn - hdg) % 360
        if heading_to_boat > 180:
            heading_to_boat -= 360
            
        return {'speed': aws, 'heading': heading_to_boat, 'headingToNorth': awn}
    
    @staticmethod
    def apply_linear_change(dt, current_val, wanted_val, change_speed):
        """
        Apply rate-limited change to a value.
        
        Args:
            dt: Time step in seconds
            current_val: Current value
            wanted_val: Target value
            change_speed: Maximum rate of change per second
            
        Returns:
            float: New value after applying rate limit
        """
        if wanted_val == current_val:
            return current_val
            
        dVal = dt * change_speed
        diff = abs(current_val - wanted_val)
        
        if diff < dVal:
            dVal = diff
            
        if wanted_val < current_val:
            return current_val - dVal
        else:
            return current_val + dVal
    
    @staticmethod
    def calculate_next_position(lat, lon, speed, hdg, drift, dt):
        """
        Calculate new position based on boat motion and environmental drift.
        
        Args:
            lat, lon: Current position
            speed: Boat speed
            hdg: Boat heading
            drift: Wind and water drift vectors
            dt: Time step
            
        Returns:
            tuple: New (latitude, longitude) position
        """
        # Calculate boat movement
        heading_rad = radians(hdg)
        dist = speed * dt
        dx = dist * sin(heading_rad)
        dy = dist * cos(heading_rad)

        # Add environmental drift effects
        if drift:
            # Wind drift
            wd = drift['wind']
            wrad = radians(wd['headingToNorth'])
            wdist = wd['speed'] * dt
            dx += wdist * sin(wrad)
            dy += wdist * cos(wrad)
            
            # Water drift
            wod = drift['water']
            orad = radians(wod['headingToNorth'])
            odist = wod['speed'] * dt
            dx += odist * sin(orad)
            dy += odist * cos(orad)
        
        # Convert movement to lat/lon changes
        earth_circ = 40075000  # Earth circumference in meters
        m_per_deg_lat = earth_circ / 360
        m_per_deg_lon = m_per_deg_lat * cos(radians(lat))
        
        new_lat = lat + dy / m_per_deg_lat
        new_lon = lon + dx / m_per_deg_lon
        return new_lat, new_lon

# ------------- Sailboat ------------- #
class Sailboat:
    """
    Main sailboat simulation class.
    
    Represents a sailboat with physical properties, control systems,
    and sensor simulation including GPS and wind vane noise.
    """
    
    # Control system response rates
    RUDDER_SPEED = 20  # Degrees per second rudder movement
    SAIL_SPEED = 25  # Sail adjustment rate per second
    
    def __init__(self, x=0, y=0, heading=0, speed=0, rudder=0, sail=0):
        """
        Initialize sailboat with default state.
        
        Args:
            x, y: Initial position in local coordinates
            heading: Initial heading in degrees
            speed: Initial speed in m/s
            rudder: Initial rudder position (-1 to +1)
            sail: Initial sail setting (0 to 1)
        """
        # Position and motion state
        self.x = x  # Local x coordinate (meters)
        self.y = y  # Local y coordinate (meters)
        self.heading = heading  # Compass heading (degrees)
        self.speed = speed  # Speed through water (m/s)
        self.roll = 0  # Heel angle (degrees)
        self.pitch = 0  # Pitch angle (degrees, currently unused)
        
        # Control surface positions
        self.actual_rudder = rudder  # Current rudder position
        self.actual_sail = sail  # Current sail position
        self.wanted_rudder = rudder  # Target rudder position
        self.wanted_sail = sail  # Target sail position
        
        # Navigation data
        self.path_x = [x]  # Historical x positions for track display
        self.path_y = [y]  # Historical y positions for track display
        self.apparent_wind = {'speed': 0, 'heading': 0, 'headingToNorth': 0}  # Apparent wind data
        self.true_wind = {'speed': 0, 'heading': 0}  # True wind data relative to boat
        self.latitude = 0  # GPS latitude
        self.longitude = 0  # GPS longitude
        self.direction = heading  # Course over ground
        
        # Sensor noise parameters
        self.gps_error_stddev_m = 1.0  # GPS position error standard deviation (meters)
        self.wind_vane_error_stddev_deg = 5.0  # Wind vane error standard deviation (degrees)

    def get_noisy_gps_reading(self):
        """
        Simulate GPS reading with realistic position noise.
        
        Returns:
            tuple: (noisy_latitude, noisy_longitude) with added GPS error
        """
        true_lat, true_lon = self.latitude, self.longitude

        # Convert position error from meters to degrees
        earth_radius_m = 6371000.0
        lat_error_m = np.random.normal(0, self.gps_error_stddev_m)
        lon_error_m = np.random.normal(0, self.gps_error_stddev_m)

        noisy_lat = true_lat + (lat_error_m / earth_radius_m) * (180.0 / np.pi)
        noisy_lon = true_lon + (lon_error_m / (earth_radius_m * np.cos(np.radians(true_lat)))) * (180.0 / np.pi)

        return noisy_lat, noisy_lon

    def get_noisy_wind_vane_reading(self):
        """
        Simulate wind vane reading with realistic measurement noise.
        
        Returns:
            float: Relative wind direction with added sensor noise
        """
        true_relative_wind_deg = self.true_wind['heading']
        noise = np.random.normal(0, self.wind_vane_error_stddev_deg)
        noisy_relative_wind_deg = true_relative_wind_deg + noise
        
        # Normalize to -180 to +180 degree range
        noisy_relative_wind_deg = (noisy_relative_wind_deg + 180) % 360 - 180
        return noisy_relative_wind_deg
    
    def set_rudder(self, value):
        """
        Set target rudder position with range checking.
        
        Args:
            value: Rudder position (-1 to +1, where -1 is full left)
        """
        if value < -1 or value > 1:
            print(f"Warning: Rudder out of range => {value}")
            value = max(-1, min(1, value))
        self.wanted_rudder = value

    def set_sail(self, value):
        """
        Set target sail position with range checking.
        
        Args:
            value: Sail position (0 to 1, where 0 is sheeted in, 1 is out)
        """
        if value < 0 or value > 1:
            print(f"Warning: Sail out of range => {value}")
            value = max(0, min(1, value))
        self.wanted_sail = value
    
    def update(self, dt, environment):
        """
        Update boat state for one simulation time step.
        
        Args:
            dt: Time step in seconds
            environment: Environmental conditions object
        """
        # Get environmental conditions
        env_values = environment.get_values()
        
        # Calculate apparent wind from true wind and boat motion
        self.apparent_wind = SailboatPhysics.calculate_apparent_wind(
            env_values['wind']['speed'],
            env_values['wind']['heading'],
            self.speed,
            self.heading
        )
        
        # Calculate true wind relative to boat heading
        self.true_wind = {
            'speed': env_values['wind']['speed'],
            'heading': (env_values['wind']['heading'] - self.heading) % 360
        }
        if self.true_wind['heading'] > 180:
            self.true_wind['heading'] -= 360
        
        # Create boat state dictionary for physics calculations
        boat_state = {
            'attitude': {'roll': self.roll, 'pitch': self.pitch, 'heading': self.heading},
            'velocity': {'speed': self.speed, 'direction': self.direction},
            'servos': {'rudder': self.actual_rudder, 'sail': self.actual_sail},
            'apparentWind': self.apparent_wind,
            'trueWind': self.true_wind,
            'gps': {'latitude': self.latitude, 'longitude': self.longitude}
        }
        
        # Update control surfaces with rate limiting
        self.actual_rudder = SailboatPhysics.apply_linear_change(
            dt, self.actual_rudder, self.wanted_rudder, Sailboat.RUDDER_SPEED
        )
        self.actual_sail = SailboatPhysics.apply_linear_change(
            dt, self.actual_sail, self.wanted_sail, Sailboat.SAIL_SPEED
        )
        
        # Calculate new boat state
        new_roll = SailboatPhysics.calculate_roll(dt,
                          self.apparent_wind['speed'],
                          self.apparent_wind['heading'],
                          self.roll)
        
        new_speed = SailboatPhysics.calculate_speed(dt, env_values, boat_state)
        
        heading_change = SailboatPhysics.calculate_heading_change(
            dt, self.speed, self.actual_rudder
        )
        new_heading = (self.heading + heading_change) % 360
        
        # Calculate environmental drift effects
        drift = {
            'wind': SailboatPhysics.calculate_wind_drift(dt, env_values, boat_state),
            'water': SailboatPhysics.calculate_water_drift(dt, env_values)
        }
        
        # Update boat state
        self.roll = new_roll
        self.speed = new_speed
        self.heading = new_heading
        
        # Calculate new position
        new_lat, new_lon = SailboatPhysics.calculate_next_position(
            self.latitude, self.longitude,
            self.speed, self.heading,
            drift, dt
        )
        
        # Update local coordinates based on position change
        earth_circ = 40075000
        m_per_deg_lat = earth_circ / 360
        m_per_deg_lon = m_per_deg_lat * cos(radians(self.latitude))
        dx = (new_lon - self.longitude) * m_per_deg_lon
        dy = (new_lat - self.latitude) * m_per_deg_lat
        
        self.x += dx
        self.y += dy
        self.latitude = new_lat
        self.longitude = new_lon
        
        # Update course over ground if boat is moving
        if abs(dx) > 1e-9 or abs(dy) > 1e-9:
            self.direction = (degrees(atan2(dx, dy)) % 360)
        
        # Maintain path history for visualization
        self.path_x.append(self.x)
        self.path_y.append(self.y)
        if len(self.path_x) > 500:  # Limit history size
            self.path_x = self.path_x[-500:]
            self.path_y = self.path_y[-500:]

# ------------- rotate_and_translate ------------- #
def rotate_and_translate(points, angle, translate=(0,0)):
    """
    Rotate and translate a set of 2D points.
    
    Args:
        points: Array of [x, y] coordinates
        angle: Rotation angle in radians
        translate: Translation vector (x, y)
        
    Returns:
        numpy.array: Transformed points
    """
    c, s = np.cos(angle), np.sin(angle)
    R = np.array([[c, -s], [s, c]])  # Rotation matrix
    rotated = points @ R.T
    rotated[:, 0] += translate[0]
    rotated[:, 1] += translate[1]
    return rotated

# ------------- BoatSchematic ------------- #
class BoatSchematic:
    """
    Visualization of boat schematic showing hull, rudder, and sail positions.
    
    Provides a top-down view of the boat with real-time control surface positions.
    """
    
    def __init__(self, ax):
        """
        Initialize boat schematic visualization.
        
        Args:
            ax: Matplotlib axis for drawing the schematic
        """
        self.ax = ax
        self.ax.set_aspect('equal', 'box')
        self.ax.set_xlim(-0.5, 0.5)
        self.ax.set_ylim(-0.6, 0.6)
        
        # Boat hull outline coordinates
        self.hull_coords = np.array([
            [-0.05, 0.30],   # Bow port
            [0.05, 0.30],    # Bow starboard
            [0.10, -0.30],   # Stern starboard
            [-0.10, -0.30]   # Stern port
        ])
        
        # Control surface parameters
        self.rudder_pivot = np.array([0.0, -0.30])  # Rudder attachment point
        self.rudder_length = 0.1  # Rudder length
        self.main_pivot = np.array([0.0, -0.05])  # Mainsail attachment point
        self.main_length = 0.3  # Mainsail boom length
        self.jib_pivot = np.array([0.0, 0.15])  # Jib attachment point (unused)
        self.jib_length = 0.25  # Jib boom length (unused)
    
    def update(self, heading_rad, rudder_angle_rad, sail_angle_rad):
        """
        Update the schematic display with current boat state.
        
        Args:
            heading_rad: Boat heading in radians
            rudder_angle_rad: Rudder angle in radians
            sail_angle_rad: Sail angle in radians
        """
        # Clear and reset axis
        self.ax.clear()
        self.ax.set_aspect('equal', 'box')
        self.ax.set_xlim(-0.5, 0.5)
        self.ax.set_ylim(-0.6, 0.6)
        self.ax.set_title("Boat Schematic (Top View)")
        self.ax.set_xticks([])
        self.ax.set_yticks([])

        # Draw hull
        hull_global = rotate_and_translate(self.hull_coords, heading_rad)
        hull_plot = np.vstack([hull_global, hull_global[0]])  # Close the polygon
        self.ax.plot(hull_plot[:, 0], hull_plot[:, 1], 'k-', linewidth=2)

        # Draw rudder
        rudder_line = np.array([
            self.rudder_pivot,
            self.rudder_pivot + np.array([0, -self.rudder_length])
        ])
        shift_r = rudder_line - self.rudder_pivot
        rudder_local = rotate_and_translate(shift_r, rudder_angle_rad) + self.rudder_pivot
        rudder_global = rotate_and_translate(rudder_local, heading_rad)
        self.ax.plot(rudder_global[:, 0], rudder_global[:, 1], 'b-', linewidth=2)

        # Draw mainsail boom
        main_line = np.array([
            self.main_pivot,
            self.main_pivot + np.array([0, -self.main_length])
        ])
        shift_m = main_line - self.main_pivot
        main_local = rotate_and_translate(shift_m, sail_angle_rad) + self.main_pivot
        main_global = rotate_and_translate(main_local, heading_rad)
        self.ax.plot(main_global[:, 0], main_global[:, 1], 'r-', linewidth=3)

        # Add legend text
        self.ax.text(-0.45, 0.55, "Bow ↑", fontsize=8, color='gray')
        self.ax.text(-0.45, 0.50, "Rudder=Blue", fontsize=8, color='blue')
        self.ax.text(-0.45, 0.45, "Sail=Red", fontsize=8, color='red')

class SailboatSimulator:
    """
    Main simulation class integrating all components.
    
    Provides complete sailboat simulation with visualization, path planning,
    and interactive controls for testing navigation algorithms.
    """
    
    def __init__(self):
        """Initialize the complete sailboat simulation system."""
        # Core simulation components
        self.boat = Sailboat()  # Main boat object
        self.environment = SailboatEnvironment()  # Environmental conditions
        self.dt = 0.1  # Simulation time step in seconds
        self.simulation_time = 0.0  # Current simulation time

        # Navigation and waypoint management
        self.waypoint = None  # Target waypoint (lat, lon) or None
        self.waypoint_x = 0  # Waypoint x coordinate in local system
        self.waypoint_y = 0  # Waypoint y coordinate in local system
        self.show_waypoint = False  # Whether to display waypoint
        self.auto_pilot = False  # Autopilot engagement state
        self.optimal_heading = None  # Current optimal heading from path planner
        
        # Visualization elements (initialized to None)
        self.no_go_zone = None  # No-go zone visualization
        self.port_layline = None  # Port layline visualization
        self.starboard_layline = None  # Starboard layline visualization

        # Path planning system
        self.available_planners = {
            'Rule-Based': RuleBasedPathPlanner(),
            'Layline Tactician': LaylinePathPlanner(),
        }
        self.planner_name = 'Layline Tactician'  # Currently selected planner
        self.path_planner = self.available_planners[self.planner_name]
        
        # Plot element references for cleanup
        self.waypoint_marker = None
        self.waypoint_line = None
        self.optimal_heading_arrow = None
        self.tacking_path = None
        
        # Initialize user interface
        self.setup_ui()
        
    def _update_no_go_zone_and_laylines(self):
        """Update visualization of no-go zone and laylines based on current conditions."""
        # Clean up previous visualization elements
        for attr_name in ['no_go_zone', 'port_layline', 'starboard_layline']:
            if hasattr(self, attr_name) and getattr(self, attr_name) is not None:
                try:
                    getattr(self, attr_name).remove()
                except ValueError:
                    pass  # Element already removed
                setattr(self, attr_name, None)
        
        # Get current wind direction in global coordinates
        wind_direction = self.environment.wind_heading
        
        # Draw no-go zone as a wedge around current boat position
        min_angle_nav, max_angle_nav = self.path_planner.define_no_go_zone(wind_direction, self.environment.wind_speed)
        
        # Convert navigation angles to matplotlib wedge angles
        wedge_theta1 = (90 - max_angle_nav + 360) % 360
        wedge_theta2 = (90 - min_angle_nav + 360) % 360
        
        # Create no-go zone visualization
        radius = 80  # Display radius for no-go zone
        self.no_go_zone = self.ax_main.add_patch(
            plt.matplotlib.patches.Wedge(
                (self.boat.x, self.boat.y), radius, 
                wedge_theta1, wedge_theta2, 
                color='red', alpha=0.2, zorder=1
            )
        )
        
        # Draw laylines if waypoint is active and planner supports them
        if self.waypoint and self.show_waypoint:
            if isinstance(self.path_planner, LaylinePathPlanner):
                port_x, port_y, starboard_x, starboard_y = self.path_planner.calculate_laylines(
                    self.waypoint_x, self.waypoint_y, 
                    wind_direction, 
                    self.environment.wind_speed
                )
                
                # Draw laylines with distinct styles
                self.port_layline = self.ax_main.plot(
                    port_x, port_y, 'b--', linewidth=1.5, alpha=0.6, zorder=1
                )[0]
                
                self.starboard_layline = self.ax_main.plot(
                    starboard_x, starboard_y, 'g--', linewidth=1.5, alpha=0.6, zorder=1
                )[0]
                
                # Update legend to include laylines if not already present
                legend_has_laylines = False
                if hasattr(self, 'legend_elements'):
                    for element in self.legend_elements:
                        if hasattr(element, 'get_label') and element.get_label() == 'Laylines':
                            legend_has_laylines = True
                            break
                
                    if not legend_has_laylines:
                        self.legend_elements.append(
                            plt.Line2D([0], [0], color='black', linestyle='--', label='Laylines')
                        )
                        self.ax_main.legend(handles=self.legend_elements, loc='upper right', frameon=True,
                                            edgecolor='#95a5a6', facecolor='#ffffff', fontsize=9,
                                            title='Navigation Elements', title_fontsize=10)
    
    def setup_ui(self):
        """Set up the complete matplotlib user interface with interactive elements."""
        # Apply modern plot style
        plt.style.use('seaborn-v0_8-darkgrid')

        # Create main figure with professional styling
        self.fig = plt.figure(figsize=(20, 12), facecolor='#f8f9fa')
        self.fig.suptitle('IOM RC Sailboat Navigation Simulator', fontsize=16, fontweight='bold',
                        color='#2c3e50', y=0.98)

        # Reserve space for controls at bottom
        self.fig.subplots_adjust(left=0.05, right=0.95, top=0.92, bottom=0.30)

        # Create grid layout for main display areas
        gs_plots = self.fig.add_gridspec(1, 3, width_ratios=[0.5, 0.2, 0.3])

        # Main navigation map (left panel)
        self.ax_main = self.fig.add_subplot(gs_plots[0, 0])
        self.ax_main.set_xlim(-100, 100)
        self.ax_main.set_ylim(-100, 100)
        self.ax_main.set_aspect('equal')
        self.ax_main.grid(True, alpha=0.4, linestyle='-', linewidth=0.6)
        self.ax_main.set_facecolor('#eaf5ff')
        self.ax_main.set_xlabel('East-West Position (m)', fontsize=11, color='#34495e')
        self.ax_main.set_ylabel('North-South Position (m)', fontsize=11, color='#34495e')

        # Initialize boat visualization elements
        self.boat_marker, = self.ax_main.plot([], [], 'o', color='#3498db', markersize=10,
                                            markeredgecolor='#2980b9', markeredgewidth=2,
                                            label='Boat', zorder=5)
        self.boat_path, = self.ax_main.plot([], [], '-', color='#3498db', alpha=0.6,
                                        linewidth=2, label='Track', zorder=3)

        # Status information display (middle panel)
        ax_info_display = self.fig.add_subplot(gs_plots[0, 1])
        ax_info_display.axis('off')
        ax_info_display.set_facecolor('#ecf0f1') 
        self.status_text = ax_info_display.text(
            0.05, 0.95, '', transform=ax_info_display.transAxes,
            fontsize=10, verticalalignment='top', fontfamily='monospace',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#ffffff',
                    edgecolor='#bdc3c7', linewidth=1, alpha=0.95)
        )

        # Boat schematic display (right panel)
        self.ax_schematic = self.fig.add_subplot(gs_plots[0, 2])
        self.ax_schematic.set_facecolor('#ffffff')
        self.schematic = BoatSchematic(self.ax_schematic)

        # Create legend for navigation elements
        self.legend_elements = [
            plt.Line2D([0], [0], marker='o', color='#3498db', label='Boat Position',
                    markersize=8, markeredgecolor='#2980b9'),
            plt.Line2D([0], [0], color='#3498db', label='Boat Track', linewidth=2, alpha=0.6),
            plt.Line2D([0], [0], color='#2980b9', label='Heading', linewidth=2),
            plt.Line2D([0], [0], color='#27ae60', label='True Wind', linewidth=2),
            plt.Line2D([0], [0], color='#e74c3c', label='Apparent Wind', linewidth=2),
            plt.Line2D([0], [0], color='cyan', linestyle='-', label='Planned Path', linewidth=2),
            plt.Line2D([0], [0], color='purple', label='Optimal Heading', linewidth=2)
        ]
        self.ax_main.legend(handles=self.legend_elements, loc='upper right', frameon=True,
                            edgecolor='#95a5a6', facecolor='#ffffff', fontsize=9,
                            title='Navigation Elements', title_fontsize=10)

        # Set up control panel and interaction handlers
        self._setup_control_panel()
        self._setup_callbacks()

        # Add mouse click handler for waypoint setting
        self.fig.canvas.mpl_connect('button_press_event', self.on_click)

    def _setup_control_panel(self):
        """Create the control panel with sliders and buttons."""
        # Control panel styling
        panel_color = '#ecf0f1'
        
        # Slider layout parameters
        slider_ax_left = 0.15
        slider_ax_width = 0.75
        slider_ax_height = 0.03
        slider_spacing_vertical = 0.040
        slider_bottom_start = 0.06
        
        # Define all sliders with their parameters
        slider_configs = [
            ('Wind Speed (m/s)', 0.1, 10.0, 5.0, '#3498db'),
            ('Wind Direction (°)', 0, 360, 0, '#27ae60'), 
            ('Sail (0=in, 1=out)', 0.0, 1.0, 0, '#f39c12'),
            ('Rudder', -1.0, 1.0, 0, '#e74c3c')
        ]
        
        # Create sliders
        self.sliders = {}
        for i, (label, min_val, max_val, init_val, color) in enumerate(slider_configs):
            y_pos = slider_bottom_start + i * slider_spacing_vertical
            ax = self.fig.add_axes([slider_ax_left, y_pos, slider_ax_width, slider_ax_height])
            ax.set_facecolor(panel_color)
            
            slider = Slider(ax, label, min_val, max_val, valinit=init_val, 
                        facecolor=color, alpha=0.7)
            slider.label.set_fontsize(10)
            slider.label.set_color('#2c3e50')
            
            # Create unique keys for slider dictionary to avoid conflicts
            if label.startswith('Wind Speed'):
                key = 'wind_speed'
            elif label.startswith('Wind Direction'):
                key = 'wind_direction'
            else:
                key = label.split()[0].lower()
            
            self.sliders[key] = slider
        
        # Button configuration with modern styling
        button_configs = [
            ('Reset Simulation', '#95a5a6'),
            ('Toggle Waypoint', '#f39c12'),
            ('Toggle Autopilot', '#27ae60')
        ]
        
        # Button layout parameters
        button_row_bottom = 0.015  # Y position for button row
        button_height = 0.03  # Height of each button
        button_width = 0.12  # Width of each button
        total_button_span = len(button_configs) * button_width + (len(button_configs) - 1) * 0.02
        buttons_start_left = (1 - total_button_span) / 2  # Center the button row
        
        # Create buttons with consistent styling
        self.buttons = {}
        for i, (label, color) in enumerate(button_configs):
            x_pos = buttons_start_left + i * (button_width + 0.03)
            ax = self.fig.add_axes([x_pos, button_row_bottom, button_width, button_height])
            
            button = Button(ax, label, color=panel_color, hovercolor=color)
            button.label.set_fontsize(10)
            button.label.set_color('#2c3e50')
            button.label.set_fontweight('bold')
            
            # Store button with normalized key name
            self.buttons[label.lower().replace(' ', '_')] = button

    def _setup_callbacks(self):
        """Set up all slider and button event callbacks for interactive control."""
        # Connect slider value changes to update methods
        self.sliders['rudder'].on_changed(self.update_rudder)
        self.sliders['sail'].on_changed(self.update_sail)
        self.sliders['wind_speed'].on_changed(self.update_wind)
        self.sliders['wind_direction'].on_changed(self.update_wind)
        
        # Connect button clicks to action methods
        self.buttons['reset_simulation'].on_clicked(self.reset)
        self.buttons['toggle_waypoint'].on_clicked(self.toggle_waypoint)
        self.buttons['toggle_autopilot'].on_clicked(self.toggle_autopilot)
        
        # Set up animation timer for continuous simulation updates
        self.timer = self.fig.canvas.new_timer(interval=50)  # 20 FPS update rate
        self.timer.add_callback(self.update_simulation)
        self.timer.start()

    def change_planner(self, label):
        """
        Change the active path planning algorithm.
        
        Args:
            label: Name of the planner to switch to (must be in available_planners)
        """
        self.planner_name = label
        self.path_planner = self.available_planners[label]
        print(f"Switched to {label} path planner")
    
    def toggle_waypoint(self, event):
        """
        Toggle waypoint display on/off.
        
        Args:
            event: Button click event (unused but required by matplotlib)
        """
        self.show_waypoint = not self.show_waypoint
        
    def toggle_autopilot(self, event):
        """
        Toggle autopilot engagement on/off.
        
        Args:
            event: Button click event (unused but required by matplotlib)
        """
        self.auto_pilot = not self.auto_pilot
        if self.auto_pilot:
            print("Autopilot engaged - boat will follow recommended heading")
        else:
            print("Autopilot disengaged - manual control")

    def on_click(self, event):
        """
        Handle mouse clicks on the main plot to set waypoints.
        
        Args:
            event: Mouse click event containing coordinates and target axes
        """
        # Only process clicks within the main navigation plot
        if event.inaxes == self.ax_main:
            # Store waypoint in local coordinate system
            self.waypoint_x, self.waypoint_y = event.xdata, event.ydata
            
            # Convert local coordinates to latitude/longitude
            earth_circumference = 40075000  # Earth circumference in meters
            meters_per_degree_lat = earth_circumference / 360
            meters_per_degree_lon = meters_per_degree_lat * cos(radians(self.boat.latitude))
            
            # Calculate waypoint position relative to boat's current GPS position
            waypoint_lat = self.boat.latitude + (self.waypoint_y - self.boat.y) / meters_per_degree_lat
            waypoint_lon = self.boat.longitude + (self.waypoint_x - self.boat.x) / meters_per_degree_lon
            
            # Set waypoint and enable display
            self.waypoint = (waypoint_lat, waypoint_lon)
            self.show_waypoint = True
            print(f"Waypoint set at ({self.waypoint_x:.1f}, {self.waypoint_y:.1f})")

    def reset(self, event=None):
        """
        Reset the simulation to initial state.
        
        Args:
            event: Button click event (optional)
        """
        self.boat = Sailboat()  # Create new boat with default state
        self.simulation_time = 0.0  # Reset simulation time counter
        self.update_wind()  # Apply current wind settings to new boat
        
    def update_rudder(self, val):
        """
        Update boat rudder position from slider input.
        
        Args:
            val: New rudder position (-1 to +1)
        """
        self.boat.set_rudder(val)
    
    def update_sail(self, val):
        """
        Update boat sail position from slider input.
        
        Args:
            val: New sail position (0 to 1)
        """
        self.boat.set_sail(val)
    
    def update_wind(self, val=None):
        """
        Update environmental wind conditions from slider inputs.
        
        Args:
            val: Slider value (unused but required by matplotlib callback)
        """
        self.environment.set_wind(
            self.sliders['wind_speed'].val,
            self.sliders['wind_direction'].val
        )
    
    def update_simulation(self):
        """
        Main simulation update loop called by timer.
        
        Advances physics simulation, updates visualizations, handles autopilot,
        and redraws all display elements.
        
        Returns:
            bool: True to continue animation
        """
        # Advance simulation time
        self.simulation_time += self.dt
        
        # Update boat physics
        self.boat.update(self.dt, self.environment)

        # Check for waypoint arrival and stop boat if reached
        if self.waypoint and self.show_waypoint:
            dist = self.path_planner.calculate_distance(
                self.boat.latitude, self.boat.longitude,
                self.waypoint[0], self.waypoint[1]
            )
            if dist < 5.0:  # 5 meter arrival threshold
                self.auto_pilot = False
                self.boat.set_sail(0)
                self.boat.set_rudder(0)
                self.sliders['sail'].set_val(0)
                self.sliders['rudder'].set_val(0)
                print("Arrived at waypoint")
        
        # Clean up previous waypoint visualization elements
        try:
            if self.waypoint_marker:
                self.waypoint_marker.remove()
            if self.waypoint_line:
                self.waypoint_line.remove()
            if self.optimal_heading_arrow:
                self.optimal_heading_arrow.remove()
            if self.tacking_path:
                for pathline in self.tacking_path:
                    pathline.remove()
        except (ValueError, RuntimeError):
            pass  # Handle cases where elements were already removed
        
        # Reset waypoint visualization references
        self.waypoint_marker = None
        self.waypoint_line = None
        self.optimal_heading_arrow = None
        self.tacking_path = None
        
        # Update no-go zone and laylines visualization
        self._update_no_go_zone_and_laylines()
        
        # Calculate optimal heading using path planner if waypoint is active
        if self.waypoint and self.show_waypoint:
            # Get sensor readings with realistic noise
            current_boat_lat, current_boat_lon = self.boat.get_noisy_gps_reading()
            current_wind_vane_reading = self.boat.get_noisy_wind_vane_reading()
            current_compass_reading = self.boat.heading  # Add compass noise here if needed
            
            # Calculate optimal heading using selected path planner
            self.optimal_heading = self.path_planner.calculate_direction(
                current_boat_lat, current_boat_lon,
                self.waypoint[0], self.waypoint[1],
                current_compass_reading,
                current_wind_vane_reading,  # Boat-relative wind direction
                self.environment.wind_speed,  # Assume accurate wind speed sensor
                self.simulation_time
            )
            
            # Apply autopilot control if engaged
            if self.auto_pilot and self.optimal_heading is not None:
                # Calculate heading error for proportional control
                heading_error = ((self.optimal_heading - self.boat.heading + 180) % 360) - 180
                
                # Simple proportional controller for rudder
                rudder_setting = max(-1, min(1, heading_error / 30.0))
                self.boat.set_rudder(rudder_setting)
                self.sliders['rudder'].set_val(rudder_setting)
                
                # Intelligent sail controller based on wind angle
                wind_angle = abs(self.boat.true_wind['heading'])
                if wind_angle < 50:
                    # Close hauled sailing - adjust for wind strength
                    if self.environment.wind_speed > 8.0:
                        target_sail = 0.15  # Slightly eased in stronger winds
                    else:
                        target_sail = 0.1   # Sheeted tight in normal conditions
                elif wind_angle < 90:
                    target_sail = 0.4  # Close reach
                elif wind_angle < 150:
                    target_sail = 0.7  # Beam to broad reach
                else:
                    target_sail = 0.9  # Running - sheet out for maximum area
                
                self.boat.set_sail(target_sail)
                self.sliders['sail'].set_val(target_sail)
        
        # Update boat position marker
        self.boat_marker.set_data([self.boat.x], [self.boat.y])
        
        # Update boat track history
        self.boat_path.set_data(self.boat.path_x, self.boat.path_y)
        
        # Update wind and heading arrows
        self._update_arrows()
        
        # Update boat schematic display
        heading_rad = -np.radians(self.boat.heading)  # Negative for correct orientation
        rudder_angle_rad = np.radians(45 * self.boat.actual_rudder)  # Scale to ±45 degrees
        
        # Calculate sail angle based on apparent wind and sail setting
        max_sail_angle = self.boat.actual_sail * 90  # 0 to 90 degrees of freedom
        apparent_wind_heading = self.boat.apparent_wind['heading']
        apparent_wind_angle = abs(apparent_wind_heading)
        sail_angle = min(max_sail_angle, apparent_wind_angle)
        sail_angle_rad = np.radians(sail_angle * np.sign(-apparent_wind_heading))
        
        self.schematic.update(
            heading_rad=heading_rad,
            rudder_angle_rad=rudder_angle_rad,
            sail_angle_rad=sail_angle_rad
        )
        
        # Update status display text
        env = self.environment.get_values()
        
        # Build status information display
        status_lines = [
            "═══ BOAT STATUS ═══",
            f"Speed:     {self.boat.speed:.2f} m/s",
            f"Heading:   {self.boat.heading:.1f}°", 
            f"Track:     {self.boat.direction:.1f}°",
            f"Heel:      {self.boat.roll:.1f}°",
            "",
            "═══ CONTROLS ═══",
            f"Rudder:    {self.boat.actual_rudder:+.2f}",
            f"Sail:      {self.boat.actual_sail:.2f}",
            "",
            "═══ WIND DATA ═══",
            f"True:      {env['wind']['speed']:.1f} m/s @ {env['wind']['heading']:.0f}°",
            f"Apparent:  {self.boat.apparent_wind['speed']:.1f} m/s @ {self.boat.apparent_wind['heading']:+.0f}°",
            "",
            f"Algorithm: {self.planner_name}"
        ]
        
        # Add navigation information if waypoint is active
        if self.waypoint and self.show_waypoint:
            dist = self.path_planner.calculate_distance(
                self.boat.latitude, self.boat.longitude,
                self.waypoint[0], self.waypoint[1]
            )
            
            status_lines.extend([
                "",
                "═══ NAVIGATION ═══",
                f"Waypoint:  {dist:.1f}m"
            ])
            
            if self.optimal_heading is not None:
                status_lines.append(f"Course:    {self.optimal_heading:.0f}°")
                
            # Display current control mode
            if self.auto_pilot:
                status_lines.append("Mode:      AUTOPILOT")
            else:
                status_lines.append("Mode:      MANUAL")
                
            # Add tack information for layline planner
            if isinstance(self.path_planner, LaylinePathPlanner):
                if self.path_planner.current_tack_is_port is not None:
                    tack_name = "PORT" if self.path_planner.current_tack_is_port else "STARBOARD"
                    status_lines.append(f"Tack:      {tack_name}")
        
        # Update status text display
        self.status_text.set_text('\n'.join(status_lines))
        
        # Auto-adjust plot limits to follow boat with margin
        margin = 50  # Meters of margin around boat
        self.ax_main.set_xlim(self.boat.x - margin, self.boat.x + margin)
        self.ax_main.set_ylim(self.boat.y - margin, self.boat.y + margin)
        
        # Draw waypoint visualization if enabled
        if self.waypoint and self.show_waypoint:
            # Draw waypoint marker as red star
            self.waypoint_marker = self.ax_main.plot(self.waypoint_x, self.waypoint_y, 'r*', markersize=12)[0]
            
            # Draw line from boat to waypoint
            self.waypoint_line = self.ax_main.plot(
                [self.boat.x, self.waypoint_x], 
                [self.boat.y, self.waypoint_y], 
                'r--', alpha=0.5, linewidth=1
            )[0]
            
            # Draw optimal heading arrow if calculated
            if self.optimal_heading is not None:
                heading_rad = np.radians(self.optimal_heading)
                arrow_len = 20
                dx = arrow_len * np.sin(heading_rad)
                dy = arrow_len * np.cos(heading_rad)
                
                self.optimal_heading_arrow = self.ax_main.arrow(
                    self.boat.x, self.boat.y, dx, dy,
                    head_width=3, head_length=6, fc='purple', ec='purple',
                    alpha=0.7, zorder=4
                )
            
            # Draw planned tacking path if optimal heading exists
            if self.optimal_heading is not None:
                wind_direction = (self.boat.heading + self.boat.true_wind['heading']) % 360
                
                # Get path visualization from path planner
                path_x, path_y = self.path_planner.calculate_tack_path(
                    self.boat.x, self.boat.y,
                    self.boat.latitude, self.boat.longitude,
                    self.waypoint[0], self.waypoint[1],
                    wind_direction, self.environment.wind_speed,
                    self.optimal_heading
                )
                
                # Draw path segments
                self.tacking_path = []
                self.tacking_path.append(self.ax_main.plot(
                    path_x, path_y, 
                    color='cyan', linestyle='-', linewidth=1, alpha=0.7
                )[0])
        
        # Trigger canvas redraw
        self.fig.canvas.draw_idle()
        return True        
        
    def _update_arrows(self):
        """Update all directional arrows in the main plot display."""
        # Remove existing arrows to prevent accumulation
        for artist in self.ax_main.get_children():
            if isinstance(artist, plt.matplotlib.patches.FancyArrow):
                artist.remove()
        
        # 1. Boat heading arrow (blue)
        heading_rad = np.radians(self.boat.heading)
        arrow_len = 10
        dx = arrow_len * np.sin(heading_rad)
        dy = arrow_len * np.cos(heading_rad)
        
        self.ax_main.arrow(
            self.boat.x, self.boat.y, dx, dy,
            head_width=2.5, head_length=5, fc='blue', ec='blue', zorder=3
        )
        
        # 2. True wind arrow (green) - shows wind source direction
        wind_heading_rad = np.radians(self.environment.wind_heading + 180)
        wind_len = 15 * self.environment.wind_speed / 5.0
        wind_x = wind_len * np.sin(wind_heading_rad)
        wind_y = wind_len * np.cos(wind_heading_rad)
        
        self.ax_main.arrow(
            self.boat.x, self.boat.y, wind_x, wind_y,
            head_width=2, head_length=4, fc='green', ec='green', zorder=2
        )
        
        # 3. Apparent wind arrow (red) - shows apparent wind source direction
        app_heading_rad = np.radians(self.boat.heading + self.boat.apparent_wind['heading'] + 180)
        app_len = 12 * self.boat.apparent_wind['speed'] / 5.0
        app_x = app_len * np.sin(app_heading_rad)
        app_y = app_len * np.cos(app_heading_rad)
        
        self.ax_main.arrow(
            self.boat.x, self.boat.y, app_x, app_y,
            head_width=1.5, head_length=3, fc='red', ec='red', zorder=2
        )

    def _update_waypoint(self):
        """
        Update waypoint visualization with proper cleanup to prevent artifacts.
        
        This method is retained for backward compatibility but functionality
        has been integrated into update_simulation() for better performance.
        """
        # Clean up previous visualization elements with robust error handling
        for attr_name, attr_value in [
            ('waypoint_marker', self.waypoint_marker),
            ('waypoint_line', self.waypoint_line),
            ('optimal_heading_arrow', self.optimal_heading_arrow),
            ('tacking_path', self.tacking_path)
        ]:
            try:
                if attr_value is not None:
                    attr_value.remove()
                    setattr(self, attr_name, None)
            except (ValueError, RuntimeError):
                setattr(self, attr_name, None)  # Ensure reset even if removal fails
        
        # Draw waypoint marker as red star
        self.waypoint_marker = self.ax_main.plot(self.waypoint_x, self.waypoint_y, 'r*', markersize=12)[0]
        
        # Draw line from boat to waypoint
        self.waypoint_line = self.ax_main.plot(
            [self.boat.x, self.waypoint_x], 
            [self.boat.y, self.waypoint_y], 
            'r--', alpha=0.5, linewidth=1
        )[0]
        
        # Draw optimal heading arrow if available
        if self.optimal_heading is not None:
            heading_rad = np.radians(self.optimal_heading)
            arrow_len = 20
            dx = arrow_len * np.sin(heading_rad)
            dy = arrow_len * np.cos(heading_rad)
            
            self.optimal_heading_arrow = self.ax_main.arrow(
                self.boat.x, self.boat.y, dx, dy,
                head_width=2.5, head_length=5, fc='purple', ec='purple', zorder=4
            )

        # Draw planned sailing path if optimal heading exists
        if self.optimal_heading is not None:
            # Calculate tacking path using path planner
            tack_points_x, tack_points_y = self.path_planner.calculate_tack_path(                
                self.boat.x, self.boat.y,
                self.boat.latitude, self.boat.longitude,
                self.waypoint[0], self.waypoint[1],
                self.environment.wind_heading, self.environment.wind_speed,
                self.optimal_heading,
                num_segments=5  # Show 5 tack segments for detailed visualization
            )
            
            # Draw the planned sailing course
            self.tacking_path = self.ax_main.plot(
                tack_points_x, tack_points_y, 
                color='cyan', linestyle='-', linewidth=2, alpha=0.7
            )[0]


def main():
    """
    Main entry point for the sailboat simulator application.
    
    Creates and displays the interactive sailboat simulation with
    path planning algorithms and real-time visualization.
    """
    simulator = SailboatSimulator()
    plt.show()


if __name__ == "__main__":
    main()

