/* Slot-18 radar prototype shared math/data core; browser global and Node CommonJS. */
(function (root, build) {
  "use strict";
  var api = build();
  if (typeof module !== "undefined" && module.exports) module.exports = api;
  root.RadarCore = api;
})(typeof globalThis !== "undefined" ? globalThis : this, function () {
  "use strict";
  const VERSION = "0.1.0";
  function valid(n) { return typeof n === "number" && Number.isFinite(n); }
  function heading(deg) {
    if (!valid(deg)) throw new Error("Invalid heading");
    return ((deg % 360) + 360) % 360;
  }
  function distance(x, y) {
    if (!valid(x) || !valid(y)) throw new Error("Invalid coordinates");
    return Math.hypot(x, y);
  }
  function rotateToWorld(xRight, yForward, headingDegrees) {
    const a = heading(headingDegrees) * Math.PI / 180;
    if (!valid(xRight) || !valid(yForward)) throw new Error("Invalid XY");
    return { east: xRight * Math.cos(a) + yForward * Math.sin(a),
             north: -xRight * Math.sin(a) + yForward * Math.cos(a) };
  }
  function makeMeasurement(input) {
    if (!input || typeof input !== "object" || !input.sensorId || !input.model)
      throw new Error("Missing sensor identity");
    if (!["track_xy", "presence", "range_speed"].includes(input.capability))
      throw new Error("Unsupported measurement type");
    const m = { schema: 1, source: input.source || "simulation", sensor_id: String(input.sensorId),
      model: String(input.model), capability: input.capability,
      position_known: false, distance_m: null, local_x_right_m: null, local_y_forward_m: null,
      detected: !!input.detected, at_ms: valid(input.atMs) ? input.atMs : 0 };
    if (m.capability === "track_xy" && m.detected) {
      if (!valid(input.xRightM) || !valid(input.yForwardM))
        throw new Error("XY track needs two finite coordinates");
      m.local_x_right_m = input.xRightM;
      m.local_y_forward_m = input.yForwardM;
      m.position_known = true;
      m.distance_m = distance(input.xRightM, input.yForwardM);
    } else if (valid(input.distanceM) && input.distanceM >= 0) {
      m.distance_m = input.distanceM;
    }
    return m;
  }
  function demoReadings(t) {
    const time = valid(t) ? t : 0;
    return [
      makeMeasurement({ sensorId:"track-01", model:"LD2450", capability:"track_xy",
        detected:true, xRightM:2.2*Math.sin(time*0.4), yForwardM:3.2+0.6*Math.cos(time*0.38), atMs:Math.floor(time*1000) }),
      makeMeasurement({ sensorId:"long-01", model:"C4001 SEN0609", capability:"range_speed",
        detected:true, distanceM:11.0+0.8*Math.cos(time*0.26), atMs:Math.floor(time*1000) }),
      makeMeasurement({ sensorId:"static-01", model:"C4002 SEN0691", capability:"presence",
        detected:true, distanceM:2.4, atMs:Math.floor(time*1000) })
    ];
  }
  return { VERSION, heading, distance, rotateToWorld, makeMeasurement, demoReadings };
});
