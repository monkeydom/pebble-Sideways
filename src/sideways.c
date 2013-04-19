#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0xF3, 0x8F, 0x53, 0xB0, 0x63, 0x37, 0x44, 0x13, 0x87, 0xC1, 0x92, 0xE4, 0x82, 0xE7, 0x25, 0x15 }
PBL_APP_INFO(MY_UUID,
             "sideways", "@monkeydom",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

static Window window;
static Layer time_layer;

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_BIG_0,
  RESOURCE_ID_IMAGE_BIG_1,
  RESOURCE_ID_IMAGE_BIG_2,
  RESOURCE_ID_IMAGE_BIG_3,
  RESOURCE_ID_IMAGE_BIG_4,
  RESOURCE_ID_IMAGE_BIG_5,
  RESOURCE_ID_IMAGE_BIG_6,
  RESOURCE_ID_IMAGE_BIG_7,
  RESOURCE_ID_IMAGE_BIG_8,
  RESOURCE_ID_IMAGE_BIG_9
};

#define TOTAL_TIME_DIGITS 4
static BmpContainer time_digits_images[TOTAL_TIME_DIGITS];
static int time_resource_ids[TOTAL_TIME_DIGITS];

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

void set_container_image(BmpContainer *bmp_container, Layer *container_layer, int previous_resource_id, const int resource_id, GPoint origin) {
  if (previous_resource_id != resource_id) {
    layer_remove_from_parent(&bmp_container->layer.layer);
    bmp_deinit_container(bmp_container);
  
    bmp_init_container(resource_id, bmp_container);
  
    GRect frame = layer_get_frame(&bmp_container->layer.layer);
    frame.origin.x = origin.x;
    frame.origin.y = origin.y;
    layer_set_frame(&bmp_container->layer.layer, frame);
  
    layer_add_child(container_layer, &bmp_container->layer.layer);
  }
}

void update_display(PblTm *current_time) {
  unsigned short display_hour = get_display_hour(current_time->tm_hour);

  // TODO remove leading zero
  set_container_image(&time_digits_images[0], &time_layer, time_resource_ids[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(0, 0));
  
  set_container_image(&time_digits_images[1], &time_layer, time_resource_ids[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(0, 40));

  set_container_image(&time_digits_images[2], &time_layer, time_resource_ids[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(0,  86));
  set_container_image(&time_digits_images[3], &time_layer, time_resource_ids[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(0, 126));
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "sideways");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);
  resource_init_current_app(&APP_RESOURCES);
  
  layer_init(&time_layer, GRect(73,0,70,168));
  layer_add_child(&window.layer, &time_layer);

  // fill containers so they are correctly released on first display
  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    bmp_init_container(RESOURCE_ID_IMAGE_BIG_0, &time_digits_images[i]);
    time_resource_ids[i] = -1;
  }


  PblTm tick_time;
  get_time(&tick_time);
  update_display(&tick_time);
}

void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    bmp_deinit_container(&time_digits_images[i]);
  }
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;

  update_display(t->tick_time);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
        .tick_handler = &handle_minute_tick,
        .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
