#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "soc/soc_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PHOTORES_ADC_CHANNEL ADC_CHANNEL_0
#define PHOTORES_ADC_UNIT    ADC_UNIT_1
#define PHOTORES_ADC_ATTEN ADC_ATTEN_DB_12

static adc_channel_t photoChannel = PHOTORES_ADC_CHANNEL;
static adc_oneshot_unit_handle_t adc1_handle;
//static int adc_raw[2];
//static int voltage[2];

//adc_cali_handle_t adc1_cali_chan0_handle = NULL;
//static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);

void adc_init(adc_channel_t *channel, uint8_t numChannels)
{
    adc_oneshot_unit_init_cfg_t unitConfig = {
        .unit_id = PHOTORES_ADC_UNIT,
    };
    adc_oneshot_new_unit(&unitConfig, &adc1_handle);

    adc_oneshot_chan_cfg_t channelConfig = {
        .atten = PHOTORES_ADC_ATTEN, //150mV - 2450mV
        .bitwidth = ADC_BITWIDTH_12, //12 bit resolution
    };

    adc_oneshot_config_channel(adc1_handle, photoChannel, &channelConfig);

    //bool do_calibration1_chan0 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN, &adc1_cali_chan0_handle);
}

int photoresistorRead_raw(void)
{
    int raw = 0;
    adc_oneshot_read(adc1_handle, photoChannel, &raw);
    return raw;    
}

int photoresistorRead_mV(void)
{
    int raw = photoresistorRead_raw();
    int mV = raw * 3300/4095;
    return mV;     
}



/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}
*/