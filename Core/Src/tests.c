#include "tests.h"

#if TESTS
#include "cmsis_os2.h"
#include "FreeRTOS.h"

#include "lvgl_port_display.h"
#include "input_task.h"
#include "motor.h"
#include "sensor_task.h"
#include "storage_task.h"

#if DRIVER_TEST
static void sensor_current_label_update(lv_obj_t *label, float current)
{
  if (label == NULL)
  {
    return;
  }

  /* Convert amps to milliamps */
  const float current_ma = current * 1000.0f;

  char buf[32];
  snprintf(buf, sizeof(buf), "M:%.0fmA", current_ma);
  lv_label_set_text(label, buf);
}

static void sensor_battery_label_update(lv_obj_t *label, float voltage, uint8_t soc)
{
  if (label == NULL)
  {
    return;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "B:%.1fV/%u%%", voltage, soc);
  lv_label_set_text(label, buf);
}

static void sensor_temperature_label_update(lv_obj_t *label, float temperature)
{
  if (label == NULL)
  {
    return;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "T:%.1f°C", temperature);
  lv_label_set_text(label, buf);
}

static void sensor_display_update(lv_obj_t *current_label,
                                  lv_obj_t *battery_label,
                                  lv_obj_t *temp_label,
                                  const SensorValuesTypeDef *values)
{
  if (values == NULL)
  {
    return;
  }

  sensor_current_label_update(current_label, values->MotorCurrent);
  sensor_battery_label_update(battery_label, values->BatteryVoltage, values->SoC);
  sensor_temperature_label_update(temp_label, values->CurrentTemp);
}

static void update_go_button_label(lv_obj_t *label, bool forward)
{
  if (label == NULL)
  {
    return;
  }

  lv_label_set_text_fmt(label, "Go: %s", forward ? "F" : "R");
}

void Driver_Test(osMessageQueueId_t storage2system_event_queue, osMessageQueueId_t input2vp_event_queue, ConfigAccessTypeDef *config_access, SensorValuesAccessTypeDef *sensor_values_access)
{
  printf("Starting driver test...\n");
  
  /* Validate queue handles */
  if (storage2system_event_queue == NULL)
  {
    printf("ERROR: storage2system_event_queue is NULL\n");
    return;
  }

  if (input2vp_event_queue == NULL)
  {
    printf("ERROR: input2vp_event_queue is NULL\n");
    return;
  }
  
  /* Validate config access handle */
  if (config_access == NULL || config_access->mutex == NULL)
  {
    printf("ERROR: config_access or mutex is NULL\n");
    return;
  }
  
  printf("Driver_Test: Queue handle: %p\n", (void *)storage2system_event_queue);
  
  /* Give StorageTask time to initialize */
  osDelay(pdMS_TO_TICKS(100U));
  
  /* Wait for configuration to be loaded */
  printf("Waiting for config to load...\n");
  Storage2SystemEventTypeDef event;
  const TickType_t config_wait_ticks = pdMS_TO_TICKS(5000U);  /* 5 second timeout */
  
  osStatus_t status = osMessageQueueGet(storage2system_event_queue, &event, NULL, config_wait_ticks);
  printf("osMessageQueueGet for storage2system_event_queue returned status=%d, event=%d\n", status, event);
  
  if (status == osOK)
  {
    if (event == EVT_CFG_LOAD_END)
    {
      printf("Config loaded successfully\n");
    }
  }
  else
  {
    printf("Config load timeout (status=%d) - using default configuration\n", status);
  }

  /* Set temperature calibration offset in config - manually manage mutex */
  ConfigTypeDef config;
  if (osMutexAcquire(config_access->mutex, osWaitForever) == osOK)
  {
    config = config_access->data;
    printf("Current temperature offset: %.1f°C\n", config.TemperatureOffsetC);
    
    config.TemperatureOffsetC = 5.0f;
    config_access->data = config;
    osMutexRelease(config_access->mutex);
    
    printf("Set temperature offset to %.1f°C\n", config.TemperatureOffsetC);
  }
  else {
    printf("Failed to acquire config mutex\n");
  }

  if (!lv_port_lock())
  {
    printf("Failed to acquire LVGL lock\n");
    return;
  }

  lv_obj_t *scr = lv_scr_act();
  lv_obj_clean(scr);
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

  lv_obj_t *encoder_label = lv_label_create(scr);
  lv_obj_set_style_text_color(encoder_label, lv_color_white(), 0);
  lv_label_set_text(encoder_label, "RE:0");
  lv_obj_align(encoder_label, LV_ALIGN_TOP_LEFT, 4, 4);

  lv_obj_t *current_label = lv_label_create(scr);
  lv_obj_set_style_text_color(current_label, lv_color_white(), 0);
  lv_label_set_text(current_label, "M:---mA");
  lv_obj_align_to(current_label, encoder_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);

  lv_obj_t *battery_label = lv_label_create(scr);
  lv_obj_set_style_text_color(battery_label, lv_color_white(), 0);
  lv_label_set_text(battery_label, "B:-.-V/--%%");
  lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -2, 4);

  lv_obj_t *temp_label = lv_label_create(scr);
  lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
  lv_label_set_text(temp_label, "T: --.-°C");
  lv_obj_align_to(temp_label, battery_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -2, 2);
  
  struct button_ui
  {
    lv_obj_t *btn;
    lv_obj_t *label;
    bool active;
  } buttons[3];

  const char *const button_texts[3] = {"Mode", "", "Menu"};
  const lv_coord_t button_width = 37;
  const lv_coord_t button_height = 18;
  const lv_coord_t gap = 3;
  const lv_coord_t margin = 3;

  for (size_t i = 0; i < 3; ++i)
  {
    buttons[i].btn = lv_btn_create(scr);
    lv_obj_set_size(buttons[i].btn, button_width, button_height);
    lv_obj_align(buttons[i].btn, LV_ALIGN_BOTTOM_LEFT, margin + i * (button_width + gap), -margin);
    lv_obj_set_style_bg_color(buttons[i].btn, lv_color_black(), 0);
    lv_obj_set_style_border_width(buttons[i].btn, 1, 0);
    lv_obj_set_style_border_color(buttons[i].btn, lv_color_white(), 0);

    buttons[i].label = lv_label_create(buttons[i].btn);
    lv_label_set_text(buttons[i].label, button_texts[i]);
    lv_obj_set_style_text_color(buttons[i].label, lv_color_white(), 0);
    lv_obj_set_style_text_font(buttons[i].label, &lv_font_montserrat_12, 0);
    lv_obj_center(buttons[i].label);
    buttons[i].active = false;
  }

  bool motor_running = false;
  bool motor_direction_forward = true;
  update_go_button_label(buttons[1].label, motor_direction_forward);

  lv_port_unlock();

  static const Input2VPEventTypeDef button_event_types[] = {
    EVT_LEFT_BTN,
    EVT_MIDDLE_BTN,
    EVT_RIGHT_BTN,
  };
  const size_t button_event_count = sizeof(button_event_types) / sizeof(button_event_types[0]);

  int32_t encoder_value = 0;
  const TickType_t sensor_display_interval = pdMS_TO_TICKS(500U);
  const TickType_t event_wait_ticks = pdMS_TO_TICKS(50U);
  TickType_t last_sensor_tick = osKernelGetTickCount();

  for (;;)
  {
    Input2VPEvent_t event;
    const bool event_ready = (osMessageQueueGet(input2vp_event_queue, &event, NULL, event_wait_ticks) == osOK);

    if (event_ready)
    {
      if (lv_port_lock())
      {
        if (event.type == EVT_CTRL_WHEEL_DELTA)
        {
          encoder_value += event.delta;
          lv_label_set_text_fmt(encoder_label, "RE:%d", (int)encoder_value);
        }
        else
        {
          switch (event.type)
          {
            case EVT_LEFT_BTN:
              if (event.button_action == BUTTON_ACTION_PRESSED)
              {
                motor_direction_forward = !motor_direction_forward;
                if (motor_running)
                {
                  Motor_SetState(motor_direction_forward ? MOTOR_FORWARD : MOTOR_BACKWARD);
                }
                update_go_button_label(buttons[1].label, motor_direction_forward);
              }
              break;
            case EVT_MIDDLE_BTN:
              if (event.button_action == BUTTON_ACTION_PRESSED)
              {
                motor_running = true;
                Motor_SetState(motor_direction_forward ? MOTOR_FORWARD : MOTOR_BACKWARD);
              }
              else
              {
                motor_running = false;
                Motor_SetState(MOTOR_COAST);
              }
              break;
            default:
              break;
          }

          size_t idx = button_event_count;
          for (size_t i = 0; i < button_event_count; ++i)
          {
            if (event.type == button_event_types[i])
            {
              idx = i;
              break;
            }
          }

          if (idx < button_event_count)
          {
            const bool pressed = (event.button_action == BUTTON_ACTION_PRESSED);
            if (pressed != buttons[idx].active)
            {
              buttons[idx].active = pressed;
              const lv_color_t bg = pressed ? lv_color_white() : lv_color_black();
              const lv_color_t txt = pressed ? lv_color_black() : lv_color_white();

              lv_obj_set_style_bg_color(buttons[idx].btn, bg, 0);
              lv_obj_set_style_text_color(buttons[idx].label, txt, 0);
            }
          }
        }
        lv_port_unlock();
      }
    }

    const TickType_t now = osKernelGetTickCount();
    if ((now - last_sensor_tick) >= sensor_display_interval)
    {
      if (sensor_values_access != NULL && sensor_values_access->mutex != NULL)
      {
        if (osMutexAcquire(sensor_values_access->mutex, osWaitForever) == osOK)
        {
          SensorValuesTypeDef values = sensor_values_access->data;
          osMutexRelease(sensor_values_access->mutex);
          
          if (lv_port_lock())
          {
            sensor_display_update(current_label, battery_label, temp_label, &values);
            lv_port_unlock();
          }
        }
      }
      last_sensor_tick = now;
    }
  }
}
#elif ADAPTATION_TEST
void Adaptation_Test(void)
{
  printf("Starting adaptation test...\n");
  /* Adaptation test not implemented yet */
}
#endif
#endif /* TESTS */