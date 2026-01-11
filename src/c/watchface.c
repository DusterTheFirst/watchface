#include <pebble.h>

static Window *s_window;

static TextLayer *s_date_layer;
static char s_date_buffer[12]; // "Wed, Sep 31" + NUL
static TextLayer *s_time_layer;
static char s_time_buffer[6]; // "00:00" + NUL

static TextLayer *s_sunrise_sunset_layer;
static char s_sunrise_sunset_buffer[] = "00:00"; // "00:00" + NUL

static TextLayer *s_temperature_layer;
static char s_temperature_buffer[] = "+00°C"; // "+00°C" + NUL
static TextLayer *s_apparent_temperature_layer;
static char s_apparent_temperature_buffer[] = "+00°C"; // "+00°C" + NUL

static TextLayer *s_surface_pressure_layer;
static char s_surface_pressure_buffer[6]; // "Q1019" + NUL

static Layer *s_graphics_layer;

static void show_time(struct tm *tick_time) {
    strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %b %e", tick_time);

    text_layer_set_text(s_time_layer, s_time_buffer);
    text_layer_set_text(s_date_layer, s_date_buffer);
}

static void draw_graphics(struct Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    int16_t y = bounds.size.h - 55;
    int16_t margin = 10;

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, GPoint(margin, y), GPoint(bounds.size.w - margin, y));
}

static void text_layer_set_roboto_condensed_white(TextLayer *text_layer) {
    text_layer_set_background_color(text_layer, GColorClear);
    text_layer_set_text_color(text_layer, GColorWhite);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
}

static void window_load(Window *window) {
    window_set_background_color(window, GColorBlack);

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_time_layer = text_layer_create(
        GRect(0, bounds.size.h - 60, bounds.size.w, 60));
    s_date_layer = text_layer_create(
        GRect(0, bounds.size.h - (60 + 26), bounds.size.w, 22));
    s_graphics_layer = layer_create(bounds);

    s_sunrise_sunset_layer = text_layer_create(
        GRect(bounds.size.w / 2, 10, bounds.size.w / 2 - 10, 22));

    s_temperature_layer = text_layer_create(
        GRect(10, 10, bounds.size.w / 2 - 10, 10 + 22));
    s_apparent_temperature_layer = text_layer_create(
        GRect(10, 10 + 22, bounds.size.w / 2 - 10, 10 + 22 * 2));

    s_surface_pressure_layer = text_layer_create(
        GRect(bounds.size.w / 2, 10 + 22, bounds.size.w / 2 - 10, 10 + 22 * 2));

    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    text_layer_set_roboto_condensed_white(s_date_layer);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

    layer_set_update_proc(s_graphics_layer, draw_graphics);

    text_layer_set_roboto_condensed_white(s_sunrise_sunset_layer);
    text_layer_set_text_alignment(s_sunrise_sunset_layer, GTextAlignmentRight);

    text_layer_set_roboto_condensed_white(s_temperature_layer);
    text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentLeft);
    text_layer_set_roboto_condensed_white(s_apparent_temperature_layer);
    text_layer_set_text_alignment(s_apparent_temperature_layer, GTextAlignmentLeft);

    text_layer_set_roboto_condensed_white(s_surface_pressure_layer);
    text_layer_set_text_alignment(s_surface_pressure_layer, GTextAlignmentRight);

    time_t current_time = time(NULL);
    show_time(localtime(&current_time));

    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
    layer_add_child(window_layer, s_graphics_layer);
    layer_add_child(window_layer, text_layer_get_layer(s_sunrise_sunset_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_apparent_temperature_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_surface_pressure_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    show_time(tick_time);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    Tuple *tuple = dict_read_first(iterator);
    while (tuple != NULL) {
        APP_LOG(APP_LOG_LEVEL_INFO, "message RX");

        if (tuple->key == MESSAGE_KEY_SURFACE_PRESSURE) {
            if (tuple->type != TUPLE_INT) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED SURFACE_PRESSURE TYPE");
                break;
            }

            if (tuple->length != 4) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED SURFACE_PRESSURE LENGTH: %i", tuple->length);
                break;
            }

            int16_t pressure = tuple->value->int32;

            // TODO: leading zero?
            snprintf(s_surface_pressure_buffer, sizeof(s_surface_pressure_buffer), "Q%i", pressure);
            text_layer_set_text(s_surface_pressure_layer, s_surface_pressure_buffer);
        } else if (tuple->key == MESSAGE_KEY_TEMPERATURE) {
            text_layer_set_text(s_temperature_layer, s_temperature_buffer);
        } else if (tuple->key == MESSAGE_KEY_APPARENT_TEMPERATURE) {
            text_layer_set_text(s_apparent_temperature_layer, s_apparent_temperature_buffer);
        } else if (tuple->key == MESSAGE_KEY_SUNRISE_SUNSET) {
            text_layer_set_text(s_sunrise_sunset_layer, s_sunrise_sunset_buffer);
        } else {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Unexpected tuple key!");
        }

        tuple = dict_read_next(iterator);
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init(void) {
    s_window = window_create();

    window_set_window_handlers(s_window,
                               (WindowHandlers){
                                   .load = window_load,
                                   .unload = window_unload,
                               });

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    const int inbox_size = 128;
    const int outbox_size = 128;
    app_message_open(inbox_size, outbox_size);

    window_stack_push(s_window, true);
}

static void deinit(void) {
    window_destroy(s_window);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

    app_event_loop();

    deinit();
}
