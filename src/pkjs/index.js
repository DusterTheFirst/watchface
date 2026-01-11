Pebble.addEventListener("ready", (e) => {
    console.log("Hello world! - Sent from your javascript application.");

    // fetch("https://example.com").then(console.log).catch(console.error)

    /**
     * @type {PositionCallback}
     */
    function pos_success(pos) {
        console.log(JSON.stringify(pos));

        const req = new XMLHttpRequest();
        req.onerror = (error) => {
            console.log(JSON.stringify(error));
        };
        req.onload = (event) => {
            console.log(req.statusText);
            const response = JSON.parse(req.responseText);
            console.log(JSON.stringify(response, undefined, 4));

            if (response.error !== undefined) {
                console.log("failed");
                return;
            }

            const current = response.current;
            const daily = response.daily;

            const surface_pressure = Math.round(current.surface_pressure);
            console.log(surface_pressure);
            const temperature = Math.round(current.temperature_2m);
            console.log(temperature);
            const feels_temperature = Math.round(current.apparent_temperature);
            console.log(feels_temperature);
            const sunrise = new Date(response.daily.sunrise);
            const sunrise_hour = sunrise.getHours();
            const sunrise_min = sunrise.getHours();
            console.log(`${sunrise_hour}:${sunrise_min}`);
            const sunset = new Date(response.daily.sunset);
            const sunset_hour = sunset.getHours();
            const sunset_min = sunset.getMinutes();
            console.log(`${sunset_hour}:${sunset_min}`);

            const dictionary = {
                "SURFACE_PRESSURE": surface_pressure,
                "TEMPERATURE": temperature,
                "APPARENT_TEMPERATURE": feels_temperature,
                "SUNRISE_SUNSET": [
                    sunrise_hour,
                    sunrise_min,
                    sunset_hour,
                    sunset_min,
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
            forecast_days: "1",
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

    // Pebble.showSimpleNotificationOnPebble("hello", "pp");
});

Pebble.addEventListener("appmessage", function (e) {
    console.log("AppMessage received!");
    // TODO: do weather
});
