module.exports = [
  {
    "type": "heading",
    "defaultValue": "Mercury"
  },
  {
    "type": "text",
    "defaultValue": "<p>By Javier Rizzo. Inspired by the Hermes Apple Watch watchface. Fonts provided by NiVZ.</p>"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Features"
      },
      {
        "type": "toggle",
        "label": "Seconds hand",
        "messageKey": "EnableSecondsHand",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Date",
        "messageKey": "EnableDate",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Pebble logo",
        "messageKey": "EnablePebbleLogo",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Watch model",
        "messageKey": "EnableWatchModel",
        "defaultValue": true
      },
      {
        "type": "select",
        "label": "Watch Model",
        "messageKey": "ForcedWatchModel",
        "defaultValue": "-1",
        "options": [
          {
            "label": "(Default: Autodetect watch model)",
            "value": "-1"
          },
          {
            "label": "Smartwatch",
            "value": "8"
          },
          {
            "label": "Classic",
            "value": "0"
          },
          {
            "label": "Classic Steel",
            "value": "1"
          },
          {
            "label": "Time",
            "value": "2"
          },
          {
            "label": "Time Steel",
            "value": "3"
          },
          {
            "label": "Time Round",
            "value": "4"
          },
          {
            "label": "2 HR",
            "value": "6"
          },
          {
            "label": "2 SE",
            "value": "7"
          },
          {
            "label": "2 Duo",
            "value": "9"
          },
          {
            "label": "Time 2",
            "value": "10"
          },
          {
            "label": "Unknown",
            "value": "5"
          }
        ]
      },
      {
        "type": "toggle",
        "label": "Moonphase",
        "messageKey": "EnableMoonphase",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "label": "Digital watch",
        "messageKey": "DigitalWatch",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Font"
      },
      {
        "type": "slider",
        "label": "Font",
        "messageKey": "Font",
        "defaultValue": 1,
        "min": 1,
        "max": 3,
        "step": 1
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Fixed Angle"
      },
      {
        "type": "toggle",
        "label": "Fixed Angle",
        "messageKey": "FixedAngle",
        "defaultValue": false
      },
      {
        "type": "slider",
        "label": "Angle",
        "messageKey": "Angle",
        "defaultValue": 40,
        "min": 0,
        "max": 359,
        "step": 1
      }
    ]
  },
  {
    "type": "section",
    "capabilities": [ "COLOR" ],
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "label": "Background Color 1",
        "messageKey": "BackgroundColor1",
        "defaultValue": "0055AA"
      },
      {
        "type": "color",
        "label": "Background Color 2",
        "messageKey": "BackgroundColor2",
        "defaultValue": "FFFFAA"
      },
      {
        "type": "color",
        "label": "Text Color 1",
        "messageKey": "TextColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Text Color 2",
        "messageKey": "TextColor2",
        "defaultValue": "FF5500"
      },
      {
        "type": "color",
        "label": "Hours Hand Color",
        "messageKey": "HoursHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Hours Hand Border Color",
        "messageKey": "HoursHandBorderColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Minutes Hand Color",
        "messageKey": "MinutesHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Minutes Hand Border Color",
        "messageKey": "MinutesHandBorderColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Seconds Hand Color",
        "messageKey": "SecondsHandColor",
        "defaultValue": "FF5500"
      }
    ]
  },
  {
    "type": "section",
    "capabilities": [ "BW" ],
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "label": "Background Color 1",
        "messageKey": "BWBackgroundColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Background Color 2",
        "messageKey": "BWBackgroundColor2",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Text Color 1",
        "messageKey": "BWTextColor1",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Text Color 2",
        "messageKey": "BWTextColor2",
        "defaultValue": "FFFFFF"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
]
