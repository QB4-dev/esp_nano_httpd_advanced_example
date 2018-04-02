#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include <user_config.h>

#include "../esp_nano_httpd/esp_nano_httpd.h"
#include "nano_httpd_wifi_util.h"
#include "../html/include/index.h"

static volatile os_timer_t blink_timer;

void blink_fun(void *arg)
{	//Do blinky stuff
    if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2)
    	gpio_output_set(0, BIT2, BIT2, 0);
    else
        gpio_output_set(BIT2, 0, BIT2, 0);
}


void ICACHE_FLASH_ATTR led_demo_cb(struct espconn *conn, void *arg, uint32_t len)
{
	uint32_t freq;
	char *param;
	http_request_t *req = conn->reverse;

	if(req == NULL || req->type != TYPE_GET || req->query == NULL){
	resp_http_error(conn);
	return; //handle GET request
	}

	param=strtok((char*)req->query,"&");
	if( os_memcmp(param,"led_freq=",9) == 0 ){         //led frequency
		freq = atoi(strchr(param,'=')+1);
		if(freq != 0){
			os_timer_disarm(&blink_timer);
			os_timer_setfn(&blink_timer, (os_timer_func_t *)blink_fun, NULL);
			os_timer_arm(&blink_timer, 1000/freq, 1); //set new frequency
			os_printf("new LED frequency set. f=%dHz\n", freq);
		}
	}
	resp_http_ok(conn);
}

// URL config table
const http_callback_t url_cfg[] = {
	{"/", send_html, index_html, sizeof(index_html)},
	{"/wifi", wifi_callback, NULL, 0},
	{0,0,0,0}
};

void ICACHE_FLASH_ATTR user_init()
{
    uart_div_modify(0, UART_CLK_FREQ / 115200);
    gpio_init();

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    os_timer_disarm(&blink_timer);
    os_timer_setfn(&blink_timer, (os_timer_func_t *)blink_fun, NULL);
    os_timer_arm(&blink_timer, 1000, 1);
    
    esp_nano_httpd_register_content(url_cfg);
    esp_nano_httpd_init(STATIONAP_MODE);
}
