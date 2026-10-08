const test = require("node:test");
const assert = require("node:assert/strict");
const radar = require("./core.js");

test("heading wraps at compass boundaries", () => {
  assert.equal(radar.heading(360), 0);
  assert.equal(radar.heading(-90), 270);
  assert.throws(() => radar.heading(NaN));
});

test("local X right and Y forward rotate toward geographic east at heading 90", () => {
  const p = radar.rotateToWorld(0, 3, 90);
  assert.ok(Math.abs(p.east - 3) < 1e-9);
  assert.ok(Math.abs(p.north) < 1e-9);
});

test("XY track is positioned and distance comes from both axes", () => {
  const m = radar.makeMeasurement({ sensorId:"1", model:"LD2450", capability:"track_xy", detected:true, xRightM:3, yForwardM:4 });
  assert.equal(m.position_known, true);
  assert.equal(m.distance_m, 5);
});

test("presence and range must not gain fictitious XY position", () => {
  for (const capability of ["presence", "range_speed"]) {
    const m = radar.makeMeasurement({ sensorId:"x", model:"C400x", capability, detected:true, distanceM:3.2 });
    assert.equal(m.position_known, false);
    assert.equal(m.local_x_right_m, null);
    assert.equal(m.local_y_forward_m, null);
    assert.equal(m.distance_m, 3.2);
  }
});

test("reject missing XY, unsupported type and negative distance", () => {
  assert.throws(() => radar.makeMeasurement({ sensorId:"x", model:"LD2450", capability:"track_xy", detected:true }));
  assert.throws(() => radar.makeMeasurement({ sensorId:"x", model:"LD2450", capability:"raw_wave", detected:true }));
  const m = radar.makeMeasurement({ sensorId:"x", model:"C4001", capability:"range_speed", detected:true, distanceM:-3 });
  assert.equal(m.distance_m, null);
});

test("three demo samples represent different hardware capabilities", () => {
  const a = radar.demoReadings(2);
  assert.deepEqual(a.map(v => v.capability), ["track_xy","range_speed","presence"]);
  assert.deepEqual(a.map(v => v.position_known), [true,false,false]);
});
