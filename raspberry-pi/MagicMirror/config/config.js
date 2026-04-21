
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
			position: "lower_third"
		},
	
{
  module: "MMM-EmbedURL",
  position: "top_right",
  header: "Stundenplan",
  config: {
    updateInterval: 120,
    embedElementType: "iframe",
    attributes: [
        "frameborder=0",
	"width=750",
	"height=500"
    ],
    embed: [
        "http://bszam.webuntis.com/timetable/class?date=2026-04-13&entityId=5119"
      
    ]
  },
},
 


		{
			module: "compliments",
			position: "bottom_left"
		},

    		{
			module: "weather",
			position: "top_left",
			header: "Weather Forecast",
			config: {
				weatherProvider: "openmeteo",
				lang: "de",
				units: "metric",
				type: "forecast",
				lat: 49.44,
				lon: 11.88
			}
		},
	
			{
		  module: "MMM-MotionDetector",
		  position: "top_left",
		  config: {
		    device: "/dev/video0",
		    captureIntervalTime: 1000,
		    threshold: 1500,
		    scoreThreshold: 20,
		    timeout: 10000,
		    turnOffDisplay: true,
		    turnOnDisplay: true,
		    displayCmd: {
		      "on": "xset dpms force on",
		      "off": "xset dpms force off"
		    }
		        //displayCommandOff: "xset dpms force off",   // für X11
        		//displayCommandOn: "xset dpms force on",     // für X11
			//keepDisplayOff: true
		  }
		},

				/* MMM-Remote-Control */

		{
		module: "MMM-Remote-Control",
		config: {
			secureEndpoints: false,
			customCommand: {},
			}
		},


		/* QR-Code */
		{
 		 module: "MMM-QRCode",
 		 position: "bottom_left",  // oder andere freie Position
  			config: {
   				 text: "http://172.16.8.8/remote.html",
   				 size: 180
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
								label: "Temperatur: ",
								suffix: "°C",
								decimals: 1
							},
							{
								topic: "BSZAM/Wetterstation/Luftdruck",
								label: "Luftdruck: ",
								suffix: "hPa",
								decimals: 1
							},
							{
								topic: "BSZAM/Wetterstation/Luftfeuchtigkeit",
								label: "Luftfeuchtigkeit: ",
								suffix: "%",
								decimals: 1
							},
							{
								topic: "BSZAM/Wetterstation/Windgeschwindigkeit",
								label: "Windgeschwindigkeit: ",
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
								label: "Regen: ",
								suffix: "l",
								decimals: 1
							},
							{
								topic: "BSZAM/Wetterstation/Systemstatus",
								label: "Status: ",
								suffix: "Läuft = 1, Läuft nicht = 0",
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
