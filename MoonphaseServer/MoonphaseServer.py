import ephem
import json
from flask import Flask, jsonify, request
from datetime import datetime


with open('config.json') as config_file:
    config = json.load(config_file)

app = Flask(__name__)


@app.route('/moon-phase', methods=['GET'])
def get_moon_phase():
    lat = request.args.get('latitude')
    lon = request.args.get('longitude')
    date = request.args.get('date')

    observer = ephem.Observer()

    if date:
        try:
            observer.date = date
        except ValueError:
            return jsonify({'Error': 'Invalid date.'}), 400
    else:
        observer.date = datetime.utcnow()

    if lat and lon:
        try:
            observer.lat = str(lat)
            observer.lon = str(lon)
        except ValueError:
            return jsonify({'Error': 'Invalid latitude or longitude.'}), 400

    moon = ephem.Moon(observer)
    illumination = moon.moon_phase

    next_full = ephem.next_full_moon(observer.date)
    next_new = ephem.next_new_moon(observer.date)
    phase_status = 'waxing' if next_full < next_new else 'waning'

    return jsonify({
        'Illumination': round(illumination, 2),
        'Status': phase_status,
        'Location': {
            'Latitude': lat,
            'Longitude': lon,
        } if lat and lon else 'default',
    })


if __name__ == '__main__':
    app.run(host=config.get('host', '0.0.0.0'), port=config.get('port', 2003))
