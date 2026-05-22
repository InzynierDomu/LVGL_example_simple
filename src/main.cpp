#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>

extern const lv_font_t montserrat_14_pl;

/** @brief LovyanGFX device: ILI9341 panel + XPT2046 touch on VSPI (LOLIN32
 * pinout). */
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Touch_XPT2046 _touch;

public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.spi_host = VSPI_HOST;
      cfg.freq_write = 40000000;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 23;
      cfg.pin_miso = 19;
      cfg.pin_dc = 16;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = 5;
      cfg.pin_rst = 17;
      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.bus_shared = true;
      _panel.config(cfg);
    }
    {
      auto cfg = _touch.config();
      cfg.x_min = 200;
      cfg.x_max = 3800;
      cfg.y_min = 200;
      cfg.y_max = 3800;
      cfg.pin_int = 34;
      cfg.bus_shared = true;
      cfg.spi_host = VSPI_HOST;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 23;
      cfg.pin_miso = 19;
      cfg.pin_cs = 15;
      cfg.freq = 2500000;
      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }
    setPanel(&_panel);
  }
};

static LGFX gfx;
static lv_color_t draw_buf[320 * 24];

static lv_obj_t *label_counter;
static int click_count = 0;

/**
 * @brief LVGL display flush — copies rendered pixels to ILI9341 via LovyanGFX.
 * @param disp   LVGL display handle.
 * @param area   Rectangle describing the dirty region.
 * @param px_map Buffer with rendered RGB565 pixel data.
 */
static void flush_cb(lv_display_t *disp, const lv_area_t *area,
                     uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  gfx.startWrite();
  gfx.setAddrWindow(area->x1, area->y1, w, h);
  gfx.pushPixels((lgfx::rgb565_t *)px_map, w * h);
  gfx.endWrite();
  lv_display_flush_ready(disp);
}

/**
 * @brief LVGL input device read — polls XPT2046 touch via LovyanGFX.
 * @param indev LVGL input device handle.
 * @param data  Output: touch state and coordinates.
 */
static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  uint16_t x, y;
  if (gfx.getTouch(&x, &y)) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

/**
 * @brief LV_EVENT_CLICKED handler for the main button — increments click
 * counter.
 * @param e Event object.
 */
static void btn_event_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    click_count++;
    char buf[32];
    snprintf(buf, sizeof(buf), "Klikniec: %d", click_count);
    lv_label_set_text(label_counter, buf);
  }
}

/**
 * @brief Builds the demo UI: title label, click counter and a button.
 */
static void create_ui(void) {
  lv_obj_t *screen = lv_screen_active();
  lv_obj_set_style_text_font(screen, &montserrat_14_pl, 0);

  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "Przykład od Inżyniera domu");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

  label_counter = lv_label_create(screen);
  lv_label_set_text(label_counter, "Kliknięć: 0");
  lv_obj_align(label_counter, LV_ALIGN_CENTER, 0, -30);

  lv_obj_t *btn = lv_button_create(screen);
  lv_obj_set_size(btn, 200, 60);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 50);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_label = lv_label_create(btn);
  lv_label_set_text(btn_label, "Kliknij mnie!");
  lv_obj_center(btn_label);
}

/**
 * @brief Arduino setup — initialises display, LVGL and builds the UI.
 */
void setup() {
  gfx.init();
  gfx.setRotation(1);
  uint16_t cal[] = {847, 3362, 797, 414, 3517, 3522, 3572, 408};
  gfx.setTouchCalibrate(cal);

  lv_init();

  lv_display_t *disp = lv_display_create(320, 240);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, flush_cb);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_display(indev, disp);
  lv_indev_set_read_cb(indev, touchpad_read);

  create_ui();
}

/**
 * @brief Arduino loop — drives LVGL tick and timer handler.
 */
void loop() {
  static uint32_t last_ms = millis();
  uint32_t now = millis();
  lv_tick_inc(now - last_ms);
  last_ms = now;
  lv_timer_handler();
}
