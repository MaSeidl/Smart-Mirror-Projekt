
let config = {
	address: "127.0.0.1",
	port: 8080,
	basePath: "/",
	ipWhitelist: [ "127.0.0.1" ],

	useHttps: false,
	language: "de",
	locale: "de-DE",

	logLevel: ["INFO", "LOG", "WARN", "ERROR"],
	timeFormat: 24,
	units: "metric",

	modules: [
		{
			module: "alert",
		},
		{
			module: "updatenotification",
			position: "top_bar"
		},
		{
			module: "clock",
			position: "top_left"
		},
		{
			module: "calendar",
			header: "US Holidays",
			position: "top_left",
			config: {
				calendars: [
					{
						url: "https://calendar.google.com/calendar/ical/de.german%23holiday%40group.v.calendar.google.com/public/basic.ics",
						symbol: "calendar"
					}
				]
			}
		},
		{
			module: "compliments",
			position: "lower_third"
		},
		{
			module: "weather",
			position: "top_right",
			header: "Weather Forecast",
			config: {
				weatherProvider: "openmeteo",
				type: "forecast",
				lat: 40.776676,
				lon: -73.971321
			}
		},
		{
			module: "newsfeed",
			position: "bottom_bar",
			config: {
				feeds: [
					{
						title: "Tagesschau",
						url: "https://www.tagesschau.de/infoservices/alle-meldungen-100~rss2.xml"
					}
				],
				showSourceTitle: true,
				showPublishDate: true
			}
		},
		
		/* 🔽🔽🔽 MMM-MQTT MODUL 🔽🔽🔽 */
		{
			module: "MMM-MQTT",
			position: "bottom_center",
			config: {
				debug: true,
				mqttServers: [
					{
						address: "localhost",
						port: 1883,
						subscriptions: [
							{
								topic: "BSZAM/Wetterstation/Temperatur",
								label: "Temperatur",
								suffix: "°C",
								decimals: 1
							},
							{
								topic: "BSZAM/Wetterstation/Luftdruck",
								label: "Luftdruck",
								suffix: "hPa",
								decimals: 1
							},
							{
								topic: "BSZAM/Wetterstation/Luftfeuchtigkeit",
								label: "Luftfeuchtigkeit",
								suffix: "%",
								decimals: 1
							},
							{
								topic: "BSZAM/Wetterstation/Windstaerke",
								label: "Windstärke",
								suffix: "km/h",
								decimals: 1
							},
							/*{
								topic: "BSZAM/Wetterstation/Windrichtung",
								label: "Windrichtung",
								suffix: "Südosten",
								decimals: 1
							},*/
							{
								topic: "BSZAM/Wetterstation/Regen",
								label: "Regen",
								suffix: "l",
								decimals: 1
							},
							{
								topic: "BSZAM/Wetterstation/Systemstatus",
								label: "Status",
								suffix: "Läuft",
								decimals: 1
							}
						]
					}
				]
			}
		}
	]
};


/*************** DO NOT EDIT THE LINE BELOW ***************/
if (typeof module !== "undefined") { module.exports = config; }
