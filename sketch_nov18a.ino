#include<stdio.h>
#include"freertos/FreeRTOS.h"
#include"freertos/task.h"
#include"driver/gpio.h"
#include"freertos/queue.h"

#define GPIO_INPUT_IO_0          GPIO_NUM_4
#define ESP_INTR_FLAG_DEFAULT   0

#define PIN22 GPIO_NUM_22
#define PIN21 GPIO_NUM_21
#define PIN18 GPIO_NUM_18
#define PIN19 GPIO_NUM_19

void init_puertos();
void corrimiento();
static void interrupt();
static xQueueHandle gpio_evt_queue = NULL;
bool sentido;

static void IRAM_ATTR gpio_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}


static void interrupt(void* arg) {
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      sentido =!sentido;
      // printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level((gpio_num_t)io_num));
    }
  }
}

void setup() {
  gpio_config_t io_conf;
  io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_POSEDGE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = (gpio_pulldown_t)1;
  io_conf.pull_up_en = (gpio_pullup_t)0;
  gpio_config(&io_conf);

  gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_NEGEDGE);
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  xTaskCreate(interrupt, "interrupt", 2048, NULL, 10, NULL);
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
  init_puertos();
  while (1) {
    corrimiento();
  }

}

void init_puertos() {
  gpio_set_direction(PIN22, GPIO_MODE_OUTPUT); 
  gpio_set_direction(PIN21, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN18, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN19, GPIO_MODE_OUTPUT);
}

void corrimiento() {
  const int D[4] ={PIN22,PIN21,PIN19,PIN18};
  const int I[4] ={PIN18,PIN19,PIN21,PIN22};

  if(sentido==true) {
    for(int i=0; i<4; i++){
      gpio_set_level((gpio_num_t)I[i], 1);
      vTaskDelay(250/portTICK_RATE_MS);
      gpio_set_level((gpio_num_t)I[i], 0);
    }
  } else if(sentido==false) {
    for(int i=0; i<4;i++) {
      gpio_set_level((gpio_num_t)D[i], 1);
      (250/portTICK_RATE_MS);
      gpio_set_level((gpio_num_t)D[i], 0);
    }
  }
}

void loop() {}
