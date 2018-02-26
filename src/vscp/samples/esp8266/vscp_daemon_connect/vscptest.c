#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "ip_addr.h"
#include "espconn.h"
#include "user_interface.h"
#include "user_config.h"

// ESP-12 modules have LED on GPIO2. Change to another GPIO
// for other boards.
static const int pin = 2;
static volatile os_timer_t some_timer;

struct espconn vscpd_conn;
ip_addr_t vscpd_ip;
esp_tcp vscpd_tcp;

char vscpd_host[] = "demo.vscp.org";
int vscpd_port = 9598;
char vscpd_path[] = "/dweet/for/eccd882c-33d0-11e5-96b7-10bf4884d1f9";
char json_data[ 256 ];
char buffer[ 2048 ];

void user_rf_pre_init( void )
{
}


void data_received( void *arg, char *pdata, unsigned short len )
{
    struct espconn *conn = arg;
    
    os_printf( "%s: %s\n", __FUNCTION__, pdata );
    
    espconn_disconnect( conn );
}


void tcp_connected( void *arg )
{
    int temperature = 55;   // test data
    struct espconn *conn = arg;
    
    os_printf( "%s\n", __FUNCTION__ );
    espconn_regist_recvcb( conn, data_received );

    os_sprintf( json_data, "{\"temperature\": \"%d\" }", temperature );
    os_sprintf( buffer, "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", 
                         vscpd_path, vscpd_host, os_strlen( json_data ), json_data );
    
    os_printf( "Sending: %s\n", buffer );
    espconn_sent( conn, buffer, os_strlen( buffer ) );
}


void tcp_disconnected( void *arg )
{
    struct espconn *conn = arg;
    
    os_printf( "%s\n", __FUNCTION__ );
    wifi_station_disconnect();
}


void dns_done( const char *name, ip_addr_t *ipaddr, void *arg )
{
    struct espconn *conn = arg;
    
    os_printf( "%s\n", __FUNCTION__ );
    
    if ( ipaddr == NULL) 
    {
        os_printf("DNS lookup failed\n");
        wifi_station_disconnect();
    }
    else
    {
        os_printf("Connecting...\n" );
        
        conn->type = ESPCONN_TCP;
        conn->state = ESPCONN_NONE;
        conn->proto.tcp=&vscpd_tcp;
        conn->proto.tcp->local_port = espconn_port();
        conn->proto.tcp->remote_port = vscpd_port;
        os_memcpy( conn->proto.tcp->remote_ip, &ipaddr->addr, 4 );

        espconn_regist_connectcb( conn, tcp_connected );
        espconn_regist_disconcb( conn, tcp_disconnected );
        
        espconn_connect( conn );
    }
}


void wifi_callback( System_Event_t *evt )
{
    os_printf( "%s: %d\n", __FUNCTION__, evt->event );
    
    switch ( evt->event )
    {
        case EVENT_STAMODE_CONNECTED:
        {
            os_printf("connect to ssid %s, channel %d\n",
                        evt->event_info.connected.ssid,
                        evt->event_info.connected.channel);
            break;
        }

        case EVENT_STAMODE_DISCONNECTED:
        {
            os_printf("disconnect from ssid %s, reason %d\n",
                        evt->event_info.disconnected.ssid,
                        evt->event_info.disconnected.reason);
            
            deep_sleep_set_option( 0 );
            // Go to sleep - go to user_init when waking up
            system_deep_sleep( 60 * 1000 * 1000 );  // 60 seconds
            break;
        }

        case EVENT_STAMODE_GOT_IP:
        {
            os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
                        IP2STR(&evt->event_info.got_ip.ip),
                        IP2STR(&evt->event_info.got_ip.mask),
                        IP2STR(&evt->event_info.got_ip.gw));
            os_printf("\n");
            
            espconn_gethostbyname( &vscpd_conn, vscpd_host, &vscpd_ip, dns_done );
            break;
        }
        
        default:
        {
            break;
        }
    }
}

void some_timerfunc( void *arg )
{
  //Do blinky stuff
  if (GPIO_REG_READ (GPIO_OUT_ADDRESS) & (1 << pin) )
  {
    // set gpio low
    gpio_output_set(0, (1 << pin), 0, 0);
  }
  else
  {
    // set gpio high
    gpio_output_set((1 << pin), 0, 0, 0);
  }

  os_printf( "%s\n", __FUNCTION__ );

}

void ICACHE_FLASH_ATTR user_init()
{
  static struct station_config config;
      
  // 0 is uart 0  - Set 115200 as baudrate    
  uart_div_modify( 0, UART_CLK_FREQ / ( 115200 ) );
  os_printf("\r\n");            // 
  os_printf("hello world\n");   // Say hello
  os_printf( "%s\n", __FUNCTION__ );

  wifi_station_set_hostname( "vscpd" );
  wifi_set_opmode_current( STATION_MODE );

  // init gpio subsytem
  gpio_init();

  // configure UART TXD to be GPIO1, set as output
  //PIN_FUNC_SELECT( PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1 ); 
  //gpio_output_set(0, 0, (1 << pin), 0);

  // setup timer (500ms, repeating)
  os_timer_setfn( &some_timer, (os_timer_func_t *)some_timerfunc, NULL );
  os_timer_arm( (struct ETSTimer *)&some_timer, 2000, 1 );

  config.bssid_set = 0;
  os_memcpy( &config.ssid, "grodansparadis", 32 );
  os_memcpy( &config.password, "brattbergavagen17!", 64 );
  wifi_station_set_config( &config );
    
  wifi_set_event_handler_cb( wifi_callback );
}
