#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "mqtt_client.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "esp_timer.h"
#include "esp_sntp.h"

#include "esp_adc/adc_oneshot.h"

/* ================= CONFIG ================= */

#define WIFI_SSID "ArduinoWiFi"
#define WIFI_PASS "2-bszam!"
#define MQTT_BROKER "mqtt://172.16.8.13"

#define SDA_PIN 4
#define SCL_PIN 5

#define BME_ADDR 0x76
#define I2C_FREQ 100000

#define RAIN_GPIO 27
#define RAIN_MM_PER_TIP 0.2794f

#define ALTITUDE 380.0

/* ===== Windsensor ===== */
#define WIND_GPIO GPIO_NUM_2
#define WIND_MAX_VOLTAGE 3.8f
#define WIND_MAX_SPEED 32.4f
#define WIND_DIVIDER_FACTOR 1.0f   /* ohne Spannungsteiler */
/* später mit 2x10k: 2.0f */

#define TOPIC_TEMP "BSZAM/Wetterstation/Temperatur"
#define TOPIC_PRESS "BSZAM/Wetterstation/Luftdruck"
#define TOPIC_HUM "BSZAM/Wetterstation/Luftfeuchtigkeit"
#define TOPIC_RAIN "BSZAM/Wetterstation/Regen"
#define TOPIC_WIND "BSZAM/Wetterstation/Windgeschwindigkeit"
#define TOPIC_STATUS "BSZAM/Wetterstation/Status"

/* =========================================== */

static const char *TAG = "WETTERSTATION";

static esp_mqtt_client_handle_t client = NULL;
static bool mqtt_connected = false;

static i2c_master_bus_handle_t bus;
static i2c_master_dev_handle_t dev;

static adc_oneshot_unit_handle_t adc1_handle;
static adc_unit_t wind_adc_unit;
static adc_channel_t wind_adc_channel;
static bool wind_adc_ready = false;

volatile uint32_t rain_counter = 0;
volatile int64_t last_interrupt_time = 0;

int32_t t_fine;

/* ================= BME STRUCT ================= */

typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} bme_calib_t;

static bme_calib_t calib;

/* ================= MQTT EVENT HANDLER ================= */

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    switch(event_id)
    {
        case MQTT_EVENT_CONNECTED:
    mqtt_connected = true;
    ESP_LOGI(TAG, "MQTT Verbindung zum Raspberry Pi hergestellt.");

    esp_mqtt_client_publish(client, TOPIC_STATUS, "1", 0, 1, 1);
    break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGE(TAG, "MQTT VERBINDUNG FEHLGESCHLAGEN!");
            ESP_LOGE(TAG, "Der Raspberry Pi / MQTT Broker ist nicht erreichbar.");
            ESP_LOGE(TAG, "Mögliche Ursachen:");
            ESP_LOGE(TAG, "1. Raspberry Pi ausgeschaltet");
            ESP_LOGE(TAG, "2. Mosquitto Broker läuft nicht");
            ESP_LOGE(TAG, "3. Falsche IP Adresse im Code");
            ESP_LOGE(TAG, "4. ESP32 und Raspberry Pi nicht im selben Netzwerk");
            break;

        case MQTT_EVENT_ERROR:
            mqtt_connected = false;
            ESP_LOGE(TAG, "MQTT Transport Fehler erkannt!");
            ESP_LOGE(TAG, "Broker konnte nicht erreicht werden.");
            break;

        default:
            break;
    }
}

/* ================= REGEN ISR ================= */

static void IRAM_ATTR rain_isr(void* arg)
{
    int64_t now = esp_timer_get_time();

    if (now - last_interrupt_time > 50000)
    {
        rain_counter++;
        last_interrupt_time = now;
    }
}

/* ================= WIFI ================= */

void wifi_init()
{
    ESP_LOGI(TAG, "Starte WLAN Verbindung...");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

/* ================= MQTT ================= */

void mqtt_start()
{
    ESP_LOGI(TAG, "Starte MQTT Verbindung zum Raspberry Pi...");

  esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = MQTT_BROKER,

    .session.last_will.topic = TOPIC_STATUS,
    .session.last_will.msg = "0",
    .session.last_will.msg_len = 1,
    .session.last_will.qos = 1,
    .session.last_will.retain = true,
};

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL)
    {
        ESP_LOGE(TAG, "MQTT Client konnte nicht erstellt werden!");
        return;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL
    ));

    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}

void mqtt_check_reconnect()
{
    if(client == NULL) return;

    if(!mqtt_connected)
    {
        ESP_LOGI(TAG, "MQTT reconnect wird versucht...");
        esp_mqtt_client_reconnect(client);
    }
}

/* ================= TIME ================= */

void time_init()
{
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

/* ================= I2C INIT ================= */

void i2c_init()
{
    ESP_LOGI(TAG, "I2C init");

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = true
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME_ADDR,
        .scl_speed_hz = I2C_FREQ
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev));
}

/* ================= WIND INIT ================= */

void wind_init()
{
    ESP_LOGI(TAG, "Initialisiere Windsensor an GPIO %d ...", WIND_GPIO);

    esp_err_t err = adc_oneshot_io_to_channel(WIND_GPIO, &wind_adc_unit, &wind_adc_channel);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "GPIO %d ist kein gültiger ADC-Pin!", WIND_GPIO);
        wind_adc_ready = false;
        return;
    }

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = wind_adc_unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, wind_adc_channel, &config));

    wind_adc_ready = true;
    ESP_LOGI(TAG, "Windsensor ADC bereit");
}

float read_wind_speed()
{
    if (!wind_adc_ready)
        return 0.0f;

    int raw = 0;

    if (adc_oneshot_read(adc1_handle, wind_adc_channel, &raw) != ESP_OK)
    {
        ESP_LOGE(TAG, "Windsensor konnte nicht gelesen werden!");
        return 0.0f;
    }

    float v_adc = ((float)raw / 4095.0f) * 3.3f;
    float v_sensor = v_adc * WIND_DIVIDER_FACTOR;

    if (v_sensor < 0.0f) v_sensor = 0.0f;
    if (v_sensor > WIND_MAX_VOLTAGE) v_sensor = WIND_MAX_VOLTAGE;

    float wind_speed = (v_sensor / WIND_MAX_VOLTAGE) * WIND_MAX_SPEED;

    if (wind_speed < 0.0f) wind_speed = 0.0f;

    return wind_speed;
}

/* ================= BME INIT ================= */

void bme_init()
{
    uint8_t ctrl_hum[2]  = {0xF2, 0x01};
    uint8_t ctrl_meas[2] = {0xF4, 0x27};

    i2c_master_transmit(dev, ctrl_hum, 2, 100);
    i2c_master_transmit(dev, ctrl_meas, 2, 100);

    uint8_t reg = 0x88;
    uint8_t data[26];

    i2c_master_transmit_receive(dev, &reg, 1, data, 26, 100);

    calib.dig_T1 = data[0] | (data[1] << 8);
    calib.dig_T2 = data[2] | (data[3] << 8);
    calib.dig_T3 = data[4] | (data[5] << 8);

    calib.dig_P1 = data[6] | (data[7] << 8);
    calib.dig_P2 = data[8] | (data[9] << 8);
    calib.dig_P3 = data[10] | (data[11] << 8);
    calib.dig_P4 = data[12] | (data[13] << 8);
    calib.dig_P5 = data[14] | (data[15] << 8);
    calib.dig_P6 = data[16] | (data[17] << 8);
    calib.dig_P7 = data[18] | (data[19] << 8);
    calib.dig_P8 = data[20] | (data[21] << 8);
    calib.dig_P9 = data[22] | (data[23] << 8);

    calib.dig_H1 = data[25];

    reg = 0xE1;
    uint8_t hdata[7];

    i2c_master_transmit_receive(dev, &reg, 1, hdata, 7, 100);

    calib.dig_H2 = hdata[0] | (hdata[1] << 8);
    calib.dig_H3 = hdata[2];
    calib.dig_H4 = (hdata[3] << 4) | (hdata[4] & 0x0F);
    calib.dig_H5 = (hdata[5] << 4) | (hdata[4] >> 4);
    calib.dig_H6 = hdata[6];
}

/* ================= SENSOR READ ================= */

void bme_read(float *t, float *p, float *h)
{
    uint8_t reg = 0xF7;
    uint8_t data[8];

    i2c_master_transmit_receive(dev, &reg, 1, data, 8, 100);

    int32_t adc_P = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    int32_t adc_T = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    int32_t adc_H = (data[6] << 8) | data[7];

    int32_t var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) *
                   ((int32_t)calib.dig_T2)) >> 11;

    int32_t var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
                   ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
                   ((int32_t)calib.dig_T3)) >> 14;

    t_fine = var1 + var2;

    *t = (t_fine * 5 + 128) >> 8;
    *t /= 100.0f;

    int64_t var1_p, var2_p, p_raw;

    var1_p = ((int64_t)t_fine) - 128000;
    var2_p = var1_p * var1_p * (int64_t)calib.dig_P6;
    var2_p += (var1_p * (int64_t)calib.dig_P5) << 17;
    var2_p += ((int64_t)calib.dig_P4) << 35;

    var1_p = ((var1_p * var1_p * (int64_t)calib.dig_P3) >> 8) +
             ((var1_p * (int64_t)calib.dig_P2) << 12);

    var1_p = (((((int64_t)1) << 47) + var1_p) *
              ((int64_t)calib.dig_P1)) >> 33;

    p_raw = 1048576 - adc_P;
    p_raw = (((p_raw << 31) - var2_p) * 3125) / var1_p;

    var1_p = (((int64_t)calib.dig_P9) * (p_raw >> 13) * (p_raw >> 13)) >> 25;
    var2_p = (((int64_t)calib.dig_P8) * p_raw) >> 19;

    p_raw = ((p_raw + var1_p + var2_p) >> 8) +
            (((int64_t)calib.dig_P7) << 4);

    float pressure = (p_raw / 256.0f) / 100.0f;
    *p = pressure / powf(1.0f - (ALTITUDE / 44330.0f), 5.255f);

    int32_t v_x1 = (t_fine - 76800);

    v_x1 = (((((adc_H << 14) - (calib.dig_H4 << 20) -
             (calib.dig_H5 * v_x1)) + 16384) >> 15) *
             (((((((v_x1 * calib.dig_H6) >> 10) *
             (((v_x1 * calib.dig_H3) >> 11) + 32768)) >> 10) +
             2097152) * calib.dig_H2 + 8192) >> 14));

    v_x1 -= (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) *
             calib.dig_H1) >> 4);

    if (v_x1 < 0) v_x1 = 0;
    if (v_x1 > 419430400) v_x1 = 419430400;

    *h = (v_x1 >> 12) / 1024.0f;
}

/* ================= MAIN ================= */

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    wifi_init();
    mqtt_start();
    time_init();

    i2c_init();
    wind_init();

    vTaskDelay(pdMS_TO_TICKS(200));

    bme_init();

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RAIN_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(RAIN_GPIO, rain_isr, NULL);

    while (1)
    {
        mqtt_check_reconnect();

        float t = 0, p = 0, h = 0;
        float wind_speed = 0.0f;

        bme_read(&t, &p, &h);
        wind_speed = read_wind_speed();

        float rain_l_m2 = rain_counter * RAIN_MM_PER_TIP;   /* 1 mm = 1 l/m² */

        bool raining = false;

        if (last_interrupt_time != 0)
        {
            if ((esp_timer_get_time() - last_interrupt_time) < 300000000)
                raining = true;
        }

        ESP_LOGI(TAG,
        "Temp %.2f °C | Druck %.2f hPa | Feuchte %.2f %% | Regen %.2f l/m² (%s) | Wind %.2f m/s",
        t, p, h, rain_l_m2, raining ? "JA" : "NEIN", wind_speed);

        if (client != NULL && mqtt_connected)
        {
            char msg[32];

            snprintf(msg, sizeof(msg), "%.2f", t);
            esp_mqtt_client_publish(client, TOPIC_TEMP, msg, 0, 1, 0);

            snprintf(msg, sizeof(msg), "%.2f", p);
            esp_mqtt_client_publish(client, TOPIC_PRESS, msg, 0, 1, 0);

            snprintf(msg, sizeof(msg), "%.2f", h);
            esp_mqtt_client_publish(client, TOPIC_HUM, msg, 0, 1, 0);

            snprintf(msg, sizeof(msg), "%.2f", rain_l_m2);
            esp_mqtt_client_publish(client, TOPIC_RAIN, msg, 0, 1, 0);

            snprintf(msg, sizeof(msg), "%.2f", wind_speed);
            esp_mqtt_client_publish(client, TOPIC_WIND, msg, 0, 1, 0);
        
            esp_mqtt_client_publish(client, TOPIC_STATUS, "1", 0, 1, 1);

        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}