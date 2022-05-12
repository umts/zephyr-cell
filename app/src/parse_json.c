#include <data/json.h>

/*
def departure_info(directions):
  """ Gets the InternetServiceDesc and time until departure data for each route """
  unique_isds = []
  for direction in directions:
    for departure in direction['Departures']:
      trip = departure['Trip']
      # SDT ex: /Date(1648627309163-0400)\, where 1648627309163 is ms since the epoch.
      # We don't care about the time zone (plus it's wrong), so we just want the seconds. We strip
      # off the leading '/Date(' and trailing '-400)\' + the last 3 digits to do a rough conversion to seconds.
      sdt = math.ceil(int(departure['SDT'][6:-10]))
      isd = trip['InternetServiceDesc']
      # If there is a departure with a unique ISD, and it's in the future, and it's not skipped
      if (isd not in unique_isds) and (sdt > time()) and (departure['StopStatusReportLabel'] != 'Skipped'):
        unique_isds.append(isd)
        # Avail's time feed seems to be off by 4 minuets
        time_diff = (sdt - time()) / 60
        send_departure_time(isd, math.floor(time_diff))
*/

struct route_directions {
  const char *route_direction;
};

struct stop {
  const char *last_updated;
  struct route_directions route_directions;
  int stop_id;
  int stop_record_id;
};

static const struct json_obj_descr route_directions_descr[] = {
  JSON_OBJ_DESCR_PRIM(struct route_directions, route_direction, JSON_TOK_STRING)
};

static const struct json_obj_descr stop_descr[] = {
  JSON_OBJ_DESCR_PRIM(struct stop, last_updated, JSON_TOK_STRING),
  JSON_OBJ_DESCR_OBJECT(struct stop, route_directions, route_directions_descr),
  JSON_OBJ_DESCR_PRIM(struct stop, stop_id, JSON_TOK_NUMBER),
  JSON_OBJ_DESCR_PRIM(struct stop, stop_record_id, JSON_TOK_NUMBER)
};

static const char stop_json[] = {
  #include "../73.json"
};

void parse_json(void) {
  struct stop stop_options;
  int expected_return_code = (1 << ARRAY_SIZE(stop_descr)) - 1;
  int ret = json_obj_parse(stop_json, sizeof(stop_json), stop_descr,
    ARRAY_SIZE(stop_descr), &stop_options);

  if (ret < 0) {
    LOG_ERR("JSON Parse Error: %d", ret);
  }
  else if (ret != expected_return_code) {
    LOG_ERR("Not all values decoded; Expected return code %d but got %d", expected_return_code, ret);
  }
  else {
    LOG_INF("json_obj_parse return code: %d", ret);
    LOG_INF("calculated return code: %d", expected_return_code);
    LOG_INF("Last Updated: %s", stop_options.last_updated);
    LOG_INF("Route Direction: %s", stop_options.route_directions.route_direction);
    LOG_INF("StopID: %d", stop_options.stop_id);
    LOG_INF("StopRecordID: %d", stop_options.stop_record_id);
  }
}
