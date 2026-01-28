#include <pebble.h>

static Window *s_window;

static TextLayer *s_date_layer;
static char s_date_buffer[12]; // "Wed, Sep 31" + NUL
static TextLayer *s_time_layer;
static char s_time_buffer[6]; // "00:00" + NUL

static TextLayer *s_sunrise_sunset_layer;
static char s_sunrise_sunset_buffer[6]; // "00:00" + NUL

static TextLayer *s_temperature_layer;
static char s_temperature_buffer[7]; // "-00°C" + NUL
static TextLayer *s_apparent_temperature_layer;
static char s_apparent_temperature_buffer[7]; // "-00°C" + NUL

static TextLayer *s_surface_pressure_layer;
static char s_surface_pressure_buffer[6]; // "Q1019" + NUL

static Layer *s_graphics_layer;

struct sunrise_sunset {
    time_t updated;
    uint8_t sunrise_h;
    uint8_t sunrise_m;
    uint8_t sunset_h;
    uint8_t sunset_m;
    uint8_t next_sunrise_h;
    uint8_t next_sunrise_m;
};

static struct sunrise_sunset s_sunrise_sunset;

static void show_sunrise_sunset() {
    if (s_sunrise_sunset.updated == 0) {
        return;
    }

    uint16_t sunrise_total_m = s_sunrise_sunset.sunrise_h * 60 + s_sunrise_sunset.sunrise_m;
    uint16_t sunset_total_m = s_sunrise_sunset.sunset_h * 60 + s_sunrise_sunset.sunset_m;

    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);

    uint16_t local_time_total_m = local_time->tm_hour * 60 + local_time->tm_min;

    uint8_t hours;
    uint8_t minutes;

    if (local_time_total_m > sunrise_total_m) {
        if (local_time_total_m > sunset_total_m) {
            hours = s_sunrise_sunset.next_sunrise_h;
            minutes = s_sunrise_sunset.next_sunrise_m;
        } else {
            hours = s_sunrise_sunset.sunset_h;
            minutes = s_sunrise_sunset.sunset_m;
        }
    } else {
        hours = s_sunrise_sunset.sunrise_h;
        minutes = s_sunrise_sunset.sunrise_m;
    }

    snprintf(s_sunrise_sunset_buffer, sizeof(s_sunrise_sunset_buffer), "%02i:%02i", hours, minutes);
    text_layer_set_text(s_sunrise_sunset_layer, s_sunrise_sunset_buffer);
}

static void show_time(struct tm *tick_time) {
    strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %b %e", tick_time);

    text_layer_set_text(s_time_layer, s_time_buffer);
    text_layer_set_text(s_date_layer, s_date_buffer);

    show_sunrise_sunset();
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

    // Get weather update every 30 minutes
    if (tick_time->tm_min % 30 == 0) {
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);
        app_message_outbox_send();
    }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    Tuple *tuple = dict_read_first(iterator);
    while (tuple != NULL) {
        if (tuple->key == MESSAGE_KEY_SURFACE_PRESSURE) {
            if (tuple->type != TUPLE_INT) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED SURFACE_PRESSURE TYPE: %i", tuple->type);
                break;
            }

            if (tuple->length != 4) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED SURFACE_PRESSURE LENGTH: %i", tuple->length);
                break;
            }

            int16_t pressure = tuple->value->int32;

            snprintf(s_surface_pressure_buffer, sizeof(s_surface_pressure_buffer), "Q%04i", pressure);
            text_layer_set_text(s_surface_pressure_layer, s_surface_pressure_buffer);
        } else if (tuple->key == MESSAGE_KEY_TEMPERATURE) {
            if (tuple->type != TUPLE_INT) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED TEMPERATURE TYPE: %i", tuple->type);
                break;
            }

            if (tuple->length != 4) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED TEMPERATURE LENGTH: %i", tuple->length);
                break;
            }

            int16_t temperature = tuple->value->int32;

            snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%3i°C", temperature);
            text_layer_set_text(s_temperature_layer, s_temperature_buffer);
        } else if (tuple->key == MESSAGE_KEY_APPARENT_TEMPERATURE) {
            if (tuple->type != TUPLE_INT) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED APPARENT_TEMPERATURE TYPE: %i", tuple->type);
                break;
            }

            if (tuple->length != 4) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED APPARENT_TEMPERATURE LENGTH: %i", tuple->length);
                break;
            }

            int16_t apparent_temperature = tuple->value->int32;

            snprintf(s_apparent_temperature_buffer, sizeof(s_apparent_temperature_buffer), "%3i°C", apparent_temperature);
            text_layer_set_text(s_apparent_temperature_layer, s_apparent_temperature_buffer);
        } else if (tuple->key == MESSAGE_KEY_SUNRISE_SUNSET) {
            if (tuple->type != TUPLE_BYTE_ARRAY) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED SUNRISE_SUNSET TYPE: %i", tuple->type);
                break;
            }

            if (tuple->length != 6) {
                APP_LOG(APP_LOG_LEVEL_ERROR, "UNEXPECTED SUNRISE_SUNSET LENGTH: %i", tuple->length);
                break;
            }

            s_sunrise_sunset = (struct sunrise_sunset){
                .updated = time(NULL),
                .sunrise_h = tuple->value->data[0],
                .sunrise_m = tuple->value->data[1],
                .sunset_h = tuple->value->data[2],
                .sunset_m = tuple->value->data[3],
                .next_sunrise_h = tuple->value->data[4],
                .next_sunrise_m = tuple->value->data[5],
            };

            show_sunrise_sunset();
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

    tick_timer_service_unsubscribe();

    app_message_deregister_callbacks();
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

    app_event_loop();

    deinit();
}
