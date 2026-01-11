Pebble.addEventListener("ready", (e) => {
    // Pebble.showSimpleNotificationOnPebble("hello", "pp");

    get_weather();
});

Pebble.addEventListener("appmessage", function (e) {
    // console.log("AppMessage received!");
    get_weather();
});

function get_weather() {
    /**
     * @type {PositionCallback}
     */
    function pos_success(pos) {
        const req = new XMLHttpRequest();
        req.onerror = (error) => {
            console.log(JSON.stringify(error));
        };
        req.onload = (event) => {
            const response = JSON.parse(req.responseText);

            if (response.error !== undefined) {
                console.log("failed");
                console.log(JSON.stringify(response, undefined, 4));
                return;
            }

            const current = response.current;
            const daily = response.daily;

            const surface_pressure = Math.round(current.surface_pressure);
            const temperature = Math.round(current.temperature_2m);
            const feels_temperature = Math.round(current.apparent_temperature);

            const sunrise = new Date(daily.sunrise[0]);
            const sunset = new Date(daily.sunset[0]);
            const next_sunrise = new Date(daily.sunrise[0]);

            const dictionary = {
                "SURFACE_PRESSURE": surface_pressure,
                "TEMPERATURE": temperature,
                "APPARENT_TEMPERATURE": feels_temperature,
                "SUNRISE_SUNSET": [
                    sunrise.getHours(),
                    sunrise.getMinutes(),
                    sunset.getHours(),
                    sunset.getMinutes(),
                    next_sunrise.getHours(),
                    next_sunrise.getMinutes(),
                ],
            };
            const success = () =>
                console.log("Weather info sent to Pebble successfully!");
            const failure = () =>
                console.log("Error sending weather info to Pebble!");

            Pebble.sendAppMessage(dictionary, success, failure);
        };

        const params = {
            latitude: pos.coords.latitude,
            longitude: pos.coords.longitude,
            timezone: "auto",
            forecast_days: "2",
            wind_speed_unit: "kn",
            daily: [
                "sunrise",
                "sunset",
                // "daylight_duration",
                // "sunshine_duration",
                // "uv_index_max",
                // "rain_sum",
                // "showers_sum",
                // "snowfall_sum",
                // "precipitation_sum",
                // "precipitation_probability_max",
                // "precipitation_hours",
                "temperature_2m_max",
                "temperature_2m_min",
                // "weather_code",
                "apparent_temperature_max",
                "apparent_temperature_min",
            ],
            // hourly: [
            //     "temperature_2m",
            //     "relative_humidity_2m",
            //     "dew_point_2m",
            //     "precipitation",
            //     "rain",
            //     "showers",
            //     "snowfall",
            //     "snow_depth",
            //     "precipitation_probability",
            //     "apparent_temperature",
            //     "wind_speed_10m",
            //     "wind_speed_180m",
            //     "wind_direction_10m",
            //     "wind_direction_180m",
            //     "wind_gusts_10m",
            //     "temperature_180m",
            //     "pressure_msl",
            //     "surface_pressure",
            //     "weather_code",
            //     "cloud_cover",
            //     "cloud_cover_low",
            //     "cloud_cover_high",
            //     "cloud_cover_mid",
            //     "visibility",
            // ],
            current: [
                "temperature_2m",
                "relative_humidity_2m",
                "is_day",
                "apparent_temperature",
                // "weather_code",
                "cloud_cover",
                "surface_pressure",
                // "pressure_msl",
                "snowfall",
                "showers",
                "rain",
                "precipitation",
                "wind_speed_10m",
                "wind_direction_10m",
                "wind_gusts_10m",
                "visibility",
            ],
        };

        const search_params = Object.entries(params).map(([key, value]) =>
            `${key}=${value}`
        ).join("&");

        const url = `https://api.open-meteo.com/v1/forecast?${search_params}`;
        req.open("GET", url);
        req.send();
    }

    /**
     * @type {PositionErrorCallback}
     */
    function pos_error(error) {
        console.error(JSON.stringify(error));
    }

    /**
     * @type {PositionOptions}
     */
    const options = {
        enableHighAccuracy: true,
    };

    navigator.geolocation.getCurrentPosition(pos_success, pos_error, options);
}
