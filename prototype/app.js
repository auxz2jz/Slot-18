/* Browser-only v0.1.0: simulated reports, diagnostic events, user-guided tests. */
(function () {
  "use strict";
  const core = window.RadarCore;
  const $ = id => document.getElementById(id);
  const ctx = $("scope").getContext("2d");
  const canvas = $("scope");
  const sessionId = "sim-" + Date.now().toString(36);
  const logKey = "slot18-diag-v1";
  const resultsKey = "slot18-test-v1";
  const startAt = performance.now();
  let events = [];
  try { events = JSON.parse(localStorage.getItem(logKey) || "[]"); if (!Array.isArray(events)) events=[]; } catch (_) { events=[]; }
  let seq = events.reduce((n, e) => Math.max(n, e.seq || 0), 0);
  let headingDeg = 0, running = true, simTime = 0, lastFrame = performance.now(), lastLogAt = 0, measurements = core.demoReadings(0);
  let testIndex = 0, currentResults = [];
  const steps = [
    "T1: Confirm the SIMULATED ONLY badge, green tracking dot, and three sensor readings.",
    "T2: Change direction North / East. Confirm the facing label changes and dot rotates.",
    "T3: Pause and resume. Confirm the displayed simulation state follows the button.",
    "T4: Export diagnostics. Confirm downloaded JSONL includes USER_ACTION and OPERATION_RESULT entries."
  ];
  function event(category, operation, result, extra) {
    const item = {session_id:sessionId,seq:++seq,time_utc:new Date().toISOString(),
      elapsed_ms:Math.round(performance.now()-startAt),version:core.VERSION,
      category:category,operation:operation,result:result,
      ...(extra || {})};
    events.push(item);
    if(events.length>500) events=events.slice(-500);
    try {localStorage.setItem(logKey,JSON.stringify(events));} catch (_) { /* memory-only fallback */ }
    $("diagCount").textContent = events.length + " diagnostic events";
  }
  function screenText() {
    const a=measurements[0], b=measurements[1], c=measurements[2];
    $("ld").textContent = a.position_known ? a.distance_m.toFixed(1)+" m XY" : "no track";
    $("c1").textContent = b.detected ? b.distance_m.toFixed(1)+" m range" : "no presence";
    $("c2").textContent = c.detected ? "PRESENT (2.4m)" : "not detected";
    $("summary").textContent = "3 virtual modules • 0 real modules";
    $("facing").textContent = "Facing "+({0:"North",90:"East",180:"South",270:"West"}[headingDeg] || (headingDeg+"°"));
    $("mode").textContent = running ? "SIMULATION RUNNING" : "SIMULATION PAUSED";
    $("toggle").textContent = running ? "Pause simulation" : "Resume simulation";
  }
  function circle(x,y,r,color,alpha) {
    ctx.globalAlpha=alpha===undefined?1:alpha;ctx.strokeStyle=color;
    ctx.beginPath();ctx.arc(x,y,r,0,Math.PI*2);ctx.stroke();ctx.globalAlpha=1;
  }
  function draw() {
    const w=canvas.width,h=canvas.height;
    ctx.clearRect(0,0,w,h);
    const cx=w/2,cy=h/2,scale=Math.min(w/22,h/22);
    ctx.fillStyle="#071713";ctx.fillRect(0,0,w,h);
    for(let m=2.5;m<=10;m+=2.5) {
      circle(cx,cy,m*scale,"#295d4e",.85);
      ctx.fillStyle="#76bca0";ctx.font="13px system-ui";
      ctx.fillText(m+"m",cx+4,cy-m*scale-5);
    }
    for(let a=0;a<360;a+=45){
      const r=a*Math.PI/180;ctx.strokeStyle="#1b4b3e";ctx.beginPath();
      ctx.moveTo(cx,cy);ctx.lineTo(cx+Math.sin(r)*10*scale,cy-Math.cos(r)*10*scale);ctx.stroke();
    }
    ctx.fillStyle="#95c9b8";ctx.font="bold 17px system-ui";
    ctx.fillText("N",cx-7,23);ctx.fillText("S",cx-7,h-12);
    ctx.fillText("W",20,cy-9);ctx.fillText("E",w-30,cy-9);
    const ar=headingDeg*Math.PI/180;
    ctx.strokeStyle="#7bffd0";ctx.lineWidth=3;ctx.beginPath();ctx.moveTo(cx,cy);
    ctx.lineTo(cx+Math.sin(ar)*27,cy-Math.cos(ar)*27);ctx.stroke();ctx.lineWidth=1;
    circle(cx,cy,5,"#9cfbd8");
    for(const m of measurements) {
      if(!m.position_known || !m.detected) continue;
      const pos=core.rotateToWorld(m.local_x_right_m,m.local_y_forward_m,headingDeg);
      if(Math.hypot(pos.east,pos.north)>10) continue;
      const x=cx+pos.east*scale,y=cy-pos.north*scale;
      circle(x,y,12,"#7af0bd",.6);
      ctx.fillStyle="#6df0b7";ctx.beginPath();ctx.arc(x,y,6,0,Math.PI*2);ctx.fill();
      ctx.font="15px system-ui";ctx.fillStyle="#d4ffe5";
      ctx.fillText("LD2450 · "+m.distance_m.toFixed(1)+" m",x+13,y-9);
    }
    ctx.fillStyle="#c69f65";ctx.font="13px system-ui";
    ctx.fillText("C4001 / C4002: range or presence only (not positioned)",15,h-13);
  }
  function render() {measurements=core.demoReadings(simTime);screenText();draw();}
  function frame(now) {
    const dt=Math.min(0.1,(now-lastFrame)/1000);
    lastFrame=now;if(running)simTime+=Math.max(0,dt);
    render();
    if(now-lastLogAt>10000){lastLogAt=now;event("STATE_TRANSITION","simulation_heartbeat","simulated",{sim_s:Math.round(simTime)});}
    requestAnimationFrame(frame);
  }
  $("headingSelect").addEventListener("change", () => {
    event("USER_ACTION","change_facing","requested");
    try {headingDeg=core.heading(Number($("headingSelect").value));
      render();event("OPERATION_RESULT","change_facing","success",{heading_deg:headingDeg});}
    catch(err){event("ERROR","change_facing","failed",{message:err.message});}
  });
  $("toggle").addEventListener("click", () => {
    event("USER_ACTION","toggle_simulation","requested");
    running=!running;screenText();
    event("STATE_TRANSITION","simulation_running",running?"running":"paused");
    event("OPERATION_RESULT","toggle_simulation","success");
  });
  function download(filename,text,type) {
    const u=URL.createObjectURL(new Blob([text],{type:type}));
    const link=document.createElement("a");link.href=u;link.download=filename;
    document.body.appendChild(link);link.click();link.remove();
    setTimeout(()=>URL.revokeObjectURL(u),3000);
  }
  $("exportBtn").addEventListener("click", () => {
    event("USER_ACTION","export_diagnostics","requested");
    event("OPERATION_START","export_diagnostics","started");
    try {
      event("OPERATION_RESULT","export_diagnostics","ready",{event_count:events.length});
      download("Slot-18-v0.1.0-diagnostics.jsonl",events.map(e=>JSON.stringify(e)).join("\n")+"\n","application/x-ndjson");
    } catch(err){event("ERROR","export_diagnostics","failed",{message:err.message});}
  });
  function showStep(){
    $("testStep").textContent=steps[testIndex]||"All test steps completed. Results recorded locally.";
    $("testProgress").textContent=currentResults.length+" / "+steps.length+" steps assessed";
    $("passBtn").disabled=testIndex>=steps.length;
    $("failBtn").disabled=testIndex>=steps.length;
  }
  $("testBtn").addEventListener("click",()=>{
    event("USER_ACTION","start_guided_test","requested");
    testIndex=0;currentResults=[];$("testPanel").hidden=false;
    showStep();event("OPERATION_RESULT","start_guided_test","success");
  });
  function markTest(result){
    if(testIndex>=steps.length)return;
    event("USER_ACTION","rate_test_step",result,{step:"T"+(testIndex+1)});
    currentResults.push({step:"T"+(testIndex+1),result:result,at:new Date().toISOString()});
    event("TEST_RESULT","T"+(testIndex+1),result);
    testIndex++;showStep();
    if(testIndex===steps.length){
      try{localStorage.setItem(resultsKey,JSON.stringify(currentResults));}catch(_){}
      event("OPERATION_RESULT","guided_test","completed",{passed:currentResults.filter(r=>r.result==="PASS").length,failed:currentResults.filter(r=>r.result==="FAIL").length});
    }
  }
  $("passBtn").addEventListener("click",()=>markTest("PASS"));
  $("failBtn").addEventListener("click",()=>markTest("FAIL"));
  $("closeTestBtn").addEventListener("click",()=>{$("testPanel").hidden=true;event("USER_ACTION","close_guided_test","requested");event("OPERATION_RESULT","close_guided_test","success");});
  event("OPERATION_START","simulator_init","started");
  render();event("OPERATION_RESULT","simulator_init","success",{sensor_count:measurements.length,real_sensor_count:0});
  requestAnimationFrame(frame);
})();
