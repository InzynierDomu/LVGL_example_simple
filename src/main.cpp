#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341  _panel;
    lgfx::Bus_SPI        _bus;
    lgfx::Touch_XPT2046  _touch;

public:
    LGFX() {
        {   // Magistrala SPI
            auto cfg = _bus.config();
            cfg.spi_host    = VSPI_HOST;
            cfg.freq_write  = 40000000;
            cfg.pin_sclk    = 18;
            cfg.pin_mosi    = 23;
            cfg.pin_miso    = 19;
            cfg.pin_dc      = 16;
            cfg.use_lock    = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        {   // Panel ILI9341
            auto cfg = _panel.config();
            cfg.pin_cs      = 5;
            cfg.pin_rst     = 17;
            cfg.memory_width  = 240;
            cfg.memory_height = 320;
            cfg.panel_width   = 240;
            cfg.panel_height  = 320;
            cfg.bus_shared  = true;
            _panel.config(cfg);
        }
        {   // Dotyk XPT2046
            auto cfg = _touch.config();
            cfg.x_min    = 200;
            cfg.x_max    = 3800;
            cfg.y_min    = 200;
            cfg.y_max    = 3800;
            cfg.pin_int  = 34;
            cfg.bus_shared = true;
            cfg.spi_host = VSPI_HOST;
            cfg.pin_sclk = 18;
            cfg.pin_mosi = 23;
            cfg.pin_miso = 19;
            cfg.pin_cs   = 15;
            cfg.freq     = 2500000;
            _touch.config(cfg);
            _panel.setTouch(&_touch);
        }
        setPanel(&_panel);
    }
};

static LGFX gfx;
static lv_color_t draw_buf[320 * 24];  // bufor renderowania (1/10 ekranu)

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    gfx.startWrite();
    gfx.setAddrWindow(area->x1, area->y1, w, h);
    gfx.pushPixels((lgfx::rgb565_t *)px_map, w * h);
    gfx.endWrite();

    lv_display_flush_ready(disp);  // informuje LVGL że frame gotowy
}

static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    uint16_t x, y;
    if (gfx.getTouch(&x, &y)) {
        data->state   = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

static lv_obj_t *label_counter;
static int click_count = 0;

static void btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        click_count++;

        char buf[32];
        snprintf(buf, sizeof(buf), "Klikniec: %d", click_count);
        lv_label_set_text(label_counter, buf);
    }
}

static void create_ui(void) {
    lv_obj_t *screen = lv_screen_active();

    // -- Nagłówek --
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Przyklad LVGL 9");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // -- Licznik kliknięć --
    label_counter = lv_label_create(screen);
    lv_label_set_text(label_counter, "Klikniec: 0");
    lv_obj_align(label_counter, LV_ALIGN_CENTER, 0, -30);

    // -- Przycisk --
    lv_obj_t *btn = lv_button_create(screen);
    lv_obj_set_size(btn, 200, 60);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Kliknij mnie!");
    lv_obj_center(btn_label);
}

void setup() {
    Serial.begin(115200);

    // Inicjalizacja ekranu
    gfx.init();
    gfx.setRotation(1);  // poziomo: 320 x 240
    uint16_t cal[] = {847, 3362, 797, 414, 3517, 3522, 3572, 408};
    gfx.setTouchCalibrate(cal);

    // Inicjalizacja LVGL
    lv_init();

    // Rejestracja wyświetlacza w LVGL
    lv_display_t *disp = lv_display_create(320, 240);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, flush_cb);

    // Rejestracja dotyku w LVGL
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(indev, disp);
    lv_indev_set_read_cb(indev, touchpad_read);

    // Budowanie UI
    create_ui();

    Serial.println("UI gotowe!");
}

void loop() {
    static uint32_t last_ms = millis();
    uint32_t now = millis();
    lv_tick_inc(now - last_ms);
    last_ms = now;

    lv_timer_handler();
}
