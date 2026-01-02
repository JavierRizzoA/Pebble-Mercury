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
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors & Patterns"
      },
      {
        "type": "select",
        "label": "Background Pattern 1",
        "messageKey": "BackgroundPattern1",
        "defaultValue": "0",
        "options": [
          {
            "label": "Solid",
            "value": "0"
          },
          {
            "label": "Heavy Dither",
            "value": "1"
          },
          {
            "label": "Light Dither",
            "value": "2"
          },
          {
            "label": "Heavy Vertical",
            "value": "3"
          },
          {
            "label": "Light Vertical",
            "value": "4"
          },
          {
            "label": "Heavy Horizontal",
            "value": "5"
          },
          {
            "label": "Light Horizontal",
            "value": "6"
          },
          {
            "label": "Diagonal",
            "value": "7"
          },
          {
            "label": "Diagonal Mirror",
            "value": "8"
          },
          {
            "label": "Checkerboard",
            "value": "9"
          }
        ]
      },
      {
        "type": "color",
        "label": "Background Color 1",
        "capabilities": [ "COLOR" ],
        "messageKey": "BackgroundColor1",
        "defaultValue": "0055AA"
      },
      {
        "type": "color",
        "label": "Background Color 1",
        "capabilities": [ "BW" ],
        "messageKey": "BWBackgroundColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Secondary Background Color 1",
        "capabilities": [ "COLOR" ],
        "messageKey": "SecondaryBackgroundColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "select",
        "label": "Background Pattern 2",
        "messageKey": "BackgroundPattern2",
        "defaultValue": "0",
        "options": [
          {
            "label": "Solid",
            "value": "0"
          },
          {
            "label": "Heavy Dither",
            "value": "1"
          },
          {
            "label": "Light Dither",
            "value": "2"
          },
          {
            "label": "Heavy Vertical",
            "value": "3"
          },
          {
            "label": "Light Vertical",
            "value": "4"
          },
          {
            "label": "Heavy Horizontal",
            "value": "5"
          },
          {
            "label": "Light Horizontal",
            "value": "6"
          },
          {
            "label": "Diagonal",
            "value": "7"
          },
          {
            "label": "Diagonal Mirror",
            "value": "8"
          },
          {
            "label": "Checkerboard",
            "value": "9"
          }
        ]
      },
      {
        "type": "color",
        "label": "Background Color 2",
        "capabilities": [ "COLOR" ],
        "messageKey": "BackgroundColor2",
        "defaultValue": "FFFFAA"
      },
      {
        "type": "color",
        "label": "Background Color 2",
        "capabilities": [ "BW" ],
        "messageKey": "BWBackgroundColor2",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Secondary Background Color 2",
        "capabilities": [ "COLOR" ],
        "messageKey": "SecondaryBackgroundColor2",
        "defaultValue": "FF5500"
      },
      {
        "type": "color",
        "label": "Text Color 1",
        "capabilities": [ "COLOR" ],
        "messageKey": "TextColor1",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Text Color 1",
        "capabilities": [ "BW" ],
        "messageKey": "BWTextColor1",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Text Color 2",
        "capabilities": [ "COLOR" ],
        "messageKey": "TextColor2",
        "defaultValue": "FF5500"
      },
      {
        "type": "color",
        "label": "Text Color 2",
        "capabilities": [ "BW" ],
        "messageKey": "BWTextColor2",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "toggle",
        "label": "No Pattern Under Text",
        "messageKey": "NoPatternUnderText",
        "description": "When enabled, the background will be solid under text and icons to improve legibility, especially on B&W screens. Not recommended, as it might drain the battery faster.",
        "defaultValue": false
      },
      {
        "type": "color",
        "label": "Hours Hand Color",
        "capabilities": [ "COLOR" ],
        "messageKey": "HoursHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Hours Hand Border Color",
        "capabilities": [ "COLOR" ],
        "messageKey": "HoursHandBorderColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Minutes Hand Color",
        "capabilities": [ "COLOR" ],
        "messageKey": "MinutesHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Minutes Hand Border Color",
        "capabilities": [ "COLOR" ],
        "messageKey": "MinutesHandBorderColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Seconds Hand Color",
        "capabilities": [ "COLOR" ],
        "messageKey": "SecondsHandColor",
        "defaultValue": "FF5500"
      },
      {
        "type": "color",
        "label": "Hours Hand Color",
        "capabilities": [ "BW" ],
        "messageKey": "BWHoursHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Hours Hand Border Color",
        "capabilities": [ "BW" ],
        "messageKey": "BWHoursHandBorderColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Minutes Hand Color",
        "capabilities": [ "BW" ],
        "messageKey": "BWMinutesHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Minutes Hand Border Color",
        "capabilities": [ "BW" ],
        "messageKey": "BWMinutesHandBorderColor",
        "defaultValue": "000000"
      },
      {
        "type": "color",
        "label": "Seconds Hand Color",
        "capabilities": [ "BW" ],
        "messageKey": "BWSecondsHandColor",
        "defaultValue": "FFFFFF"
      },
      {
        "type": "color",
        "label": "Seconds Hand Border Color",
        "capabilities": [ "BW" ],
        "messageKey": "BWSecondsHandBorderColor",
        "defaultValue": "000000"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
]
