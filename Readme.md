# NTP connection setup for ROS2 

This is an example project which allows to connect to local NTP server like a computer or to a global NTP server and then sends time as message as ros2 topic.

The code can be modified according to application.

------------

Sample video pending [..](.. "..").

------------

### List of components used:
- [ESP 32](https://www.az-delivery.de/en/products/esp32-developmentboard "ESP 32")

------------

### About
This example provides instructions to create NTP server on a linux system (Ubuntu in this case). The esp-32 conencts to the local server and saves the time values as Real time clock. 
This clock can then be used to publish messges with time stamp allowing a synchronized message publishing.
The date and time values are sent over ROS2 via micro-ros.

In ROS2, these values are published to time_publisher topic to demonstrate the output for this example. The value is an String which can be modified accroding to the requirements.

------------

### NTP setup for host system.

------------

1. sudo apt-get update
2. sudo apt-get install ntp
3. sntp --version
4. sudo nano /etc/ntp.conf OR 
        sudo gedit /etc/ntp.conf

Enter nearest ntp server address. To find servers, can refer this link -> https://support.ntp.org/bin/view/Servers/NTPPoolServers

replace server list from -> 
> pool 0.ubuntu.pool.ntp.org iburst 
> 
> pool 1.ubuntu.pool.ntp.org iburst
> 
> pool 2.ubuntu.pool.ntp.org iburst
> 
> pool 3.ubuntu.pool.ntp.org iburst

to required servers. Use iburst to make specific server high priority. 
example -> 
> server 0.europe.pool.ntp.org
> 
> server 1.europe.pool.ntp.org

1. sudo service ntp restart
2. sudo service ntp status

Now there should be an ip address of local network. Should be in series of 192.168.x.x or 10.x.x.x

7. sudo ufw allow from any to any port 123 proto udp

------------

### NTP setup for ESP32 system.

------------

1. Update the IP address in the code in place of NTP server.
2. Update the timezone in the code according to the required location.

------------

### Tutorial
1. Follow the  [tutorial here](https://link.medium.com/pdmyDUIh9nb "tutorial here") to setup the micro-ros environment.
2. Clone this repository and copy the folder 'time_publisher' inside your workspace such that folder structure is as  [YOUR WORKSPACE]/firmware/freertos_apps/apps/time_publisher 
3. Open terminal can follow these commands:
   - ros2 run micro_ros_setup configure_firmware.sh time_publisher -t udp -i [LOCAL MACHINE IP ADDRESS] -p 8888
   - ros2 run micro_ros_setup build_firmware.sh
   - ros2 run micro_ros_setup flash_firmware.sh
4. Once the flashing is successful, run
   - ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
5. Open new terminal window, and check ros topic
   - ros2 topic echo /time_publisher

------------

Available time attributes in time struct.
- int tm_sec;           /* Seconds.	[0-60] (1 leap second) */
- int tm_min;           /* Minutes.	[0-59] */
- int tm_hour;          /* Hours.	[0-23] */
- int tm_mday;          /* Day.		[1-31] */
- int tm_mon;           /* Month.	[0-11] */
- int tm_year;          /* Year	- 1900.  */
- int tm_wday;          /* Day of week.	[0-6] */
- int tm_yday;          /* Days in year.[0-365]	*/
- int tm_isdst;         /* DST.		[-1/0/1]*/
