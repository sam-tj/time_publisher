#include <stdio.h>
#include <unistd.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/string.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#define RCCHECK(fn)                                                                      \
	{                                                                                    \
		rcl_ret_t temp_rc = fn;                                                          \
		if ((temp_rc != RCL_RET_OK))                                                     \
		{                                                                                \
			printf("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc); \
			vTaskDelete(NULL);                                                           \
		}                                                                                \
	}
#define RCSOFTCHECK(fn)                                                                    \
	{                                                                                      \
		rcl_ret_t temp_rc = fn;                                                            \
		if ((temp_rc != RCL_RET_OK))                                                       \
		{                                                                                  \
			printf("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc); \
		}                                                                                  \
	}

//initialization
rcl_publisher_t publisher;
int counter = 1;
time_t now;
struct tm timeinfo;
//char strftime_buf[64];

static void initialize_sntp(void);
static void obtain_time(void);

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
	obtain_time();
	RCLC_UNUSED(last_call_time);
	if (timer != NULL)
	{
		//initialization of msg
		std_msgs__msg__String msg;
		// assign message to publisher
		std_msgs__msg__String__init(&msg);
		const unsigned int PUB_MSG_CAPACITY = 30;
		msg.data.data = malloc(PUB_MSG_CAPACITY);
		msg.data.capacity = PUB_MSG_CAPACITY;
		snprintf(msg.data.data, msg.data.capacity, "Time now  hh: %d mm: %d ss: %d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
		msg.data.size = strlen(msg.data.data);
		RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
		free(msg.data.data);
	}
}

void appMain(void *arg)
{
	if (timeinfo.tm_year < (2016 - 1900))
	{
		printf( "Time is not set yet. Connecting to WiFi and getting time over NTP.");
		initialize_sntp();
	}

	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	// create init_options
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "time_publisher", "", &support));

	// create publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
		"time_publisher"));

	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback));

	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));

	while (1)
	{
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
		usleep(100000);
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher, &node))
	RCCHECK(rcl_node_fini(&node))

	vTaskDelete(NULL);
}

static void initialize_sntp(void)
{
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "192.168.0.11"); // update ip with respect to the ntp server
	sntp_init();
	int retry = 0;
	const int retry_count = 15;
	while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
	{
		printf( "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
	time(&now);
	localtime_r(&now, &timeinfo);
	setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // update time zone according to requirements ( https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv )
	tzset();
	localtime_r(&now, &timeinfo);
	//strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
}

static void obtain_time(void)
{
	time(&now);
	localtime_r(&now, &timeinfo);
}