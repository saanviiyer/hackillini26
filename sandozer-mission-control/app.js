// ════════════════════════════════════════════════════
// COMPUTER VISION — TEACHABLE MACHINE
// ════════════════════════════════════════════════════

// Vision state per panel
const visionState = {
    obs: { model: null, webcam: null, running: false, classes: [], lastClass: null, lastConf: 0, snapMode: false },
    test: { model: null, webcam: null, running: false, classes: [], lastClass: null, lastConf: 0, snapMode: false },
    vision: { model: null, webcam: null, running: false, classes: [], lastClass: null, lastConf: 0, snapMode: false }
};

// CV terrain labels (desert/forest/mountain/plains) — standalone, no robot terrain influence
function cvTerrainColor(className) {
    const lc = className.toLowerCase();
    if (lc.includes('desert')) return '#e8a000';
    if (lc.includes('forest')) return '#2d7a3a';
    if (lc.includes('mountain')) return '#7a6050';
    if (lc.includes('plains')) return '#5a8a30';
    return '#f5c800';
}

function getModelUrl(prefix) {
    const urlEl = document.getElementById(prefix + '-model-url');
    let url = urlEl ? urlEl.value.trim() : '';
    if (!url.endsWith('/')) url += '/';
    return url;
}

async function initVision(prefix) {
    const vs = visionState[prefix];
    if (vs.running) return;

    const overlay = document.getElementById(prefix + '-start-overlay');
    if (overlay) overlay.style.display = 'none';

    const dot = document.getElementById(prefix + '-vision-dot');
    const activeLabel = document.getElementById(prefix + '-active-label');
    const urlBox = document.getElementById(prefix + '-model-url-box');

    // Hide URL box once started
    if (urlBox) urlBox.style.display = 'none';

    if (activeLabel) { activeLabel.style.display = 'block'; activeLabel.textContent = 'LOADING MODEL…'; }
    if (dot) dot.className = 'vision-status-dot';

    const modelUrl = getModelUrl(prefix);

    try {
        const modelURL = modelUrl + 'model.json';
        const metadataURL = modelUrl + 'metadata.json';

        vs.model = await tmImage.load(modelURL, metadataURL);
        vs.classes = [];
        for (let i = 0; i < vs.model.getTotalClasses(); i++) {
            vs.classes.push(vs.model.getClassLabels()[i]);
        }

        // Setup webcam
        vs.webcam = new tmImage.Webcam(300, 160, true);
        await vs.webcam.setup();
        await vs.webcam.play();

        const container = document.getElementById(prefix + '-webcam-container');
        container.innerHTML = '';
        container.appendChild(vs.webcam.canvas);

        vs.running = true;
        if (dot) dot.className = 'vision-status-dot active';
        if (activeLabel) activeLabel.textContent = 'SCANNING…';

        // Show controls
        const stopBtn = document.getElementById(prefix + '-stop-btn');
        const modeToggle = document.getElementById(prefix + '-mode-toggle');
        if (stopBtn) stopBtn.style.display = 'inline-block';
        if (modeToggle) modeToggle.style.display = 'inline-block';

        // Build readout rows
        buildVisionReadout(prefix);

        // Start loop
        visionLoop(prefix);

    } catch (err) {
        console.warn('[Vision]', err);
        if (dot) dot.className = 'vision-status-dot error';
        if (activeLabel) { activeLabel.style.display = 'block'; activeLabel.textContent = 'CAM BLOCKED / LOAD ERR'; }
        if (overlay) { overlay.style.display = 'flex'; overlay.querySelector('.vision-start-text').textContent = 'RETRY — CAM BLOCKED'; }
        if (urlBox) urlBox.style.display = 'block';
        // Auto-switch to snapshot mode on error
        switchToSnap(prefix);
    }
}

function buildVisionReadout(prefix) {
    const vs = visionState[prefix];
    const readout = document.getElementById(prefix + '-vision-readout');
    if (!readout) return;
    readout.style.display = 'block';
    readout.innerHTML = '';
    vs.classes.forEach((cls, i) => {
        const row = document.createElement('div');
        row.className = 'vision-class-row';
        row.innerHTML = `
      <span class="vision-class-label">${cls}</span>
      <div class="vision-bar-wrap"><div class="vision-bar-fill" id="${prefix}-bar-${i}" style="width:0%"></div></div>
      <span class="vision-pct" id="${prefix}-pct-${i}">0%</span>
    `;
        readout.appendChild(row);
    });
}

async function visionLoop(prefix) {
    const vs = visionState[prefix];
    if (!vs.running || !vs.webcam || !vs.model) return;
    vs.webcam.update();
    await predictFromCanvas(prefix, vs.webcam.canvas);
    requestAnimationFrame(() => visionLoop(prefix));
}

async function predictFromCanvas(prefix, canvas) {
    const vs = visionState[prefix];
    if (!vs.model) return;

    try {
        const predictions = await vs.model.predict(canvas);
        let highest = { className: '', probability: 0 };
        predictions.forEach((p, i) => {
            if (p.probability > highest.probability) highest = p;
            const bar = document.getElementById(prefix + '-bar-' + i);
            const pct = document.getElementById(prefix + '-pct-' + i);
            const pctVal = (p.probability * 100).toFixed(0);
            if (bar) { bar.style.width = pctVal + '%'; bar.className = 'vision-bar-fill'; }
            if (pct) { pct.textContent = pctVal + '%'; pct.className = 'vision-pct'; }
        });

        // Highlight highest
        predictions.forEach((p, i) => {
            const bar = document.getElementById(prefix + '-bar-' + i);
            const pct = document.getElementById(prefix + '-pct-' + i);
            if (p.className === highest.className) {
                if (bar) bar.className = 'vision-bar-fill highest';
                if (pct) pct.className = 'vision-pct highest';
            }
        });

        vs.lastClass = highest.className;
        vs.lastConf = highest.probability;

        const activeLabel = document.getElementById(prefix + '-active-label');
        if (activeLabel) activeLabel.textContent = `${highest.className.toUpperCase()} ${(highest.probability * 100).toFixed(0)}%`;

        // Always update big label on vision screen (works for both webcam and photo upload)
        const bigLabel = document.getElementById('vision-big-label');
        const bigConf = document.getElementById('vision-big-conf');
        if (bigLabel && highest.className) {
            bigLabel.textContent = highest.className.toUpperCase();
            bigLabel.style.color = cvTerrainColor(highest.className);
        }
        if (bigConf && highest.className) {
            bigConf.textContent = (highest.probability * 100).toFixed(0) + '%';
        }
        // Update overlay label on uploaded image preview
        const snapOverlay = document.getElementById('vision-snap-overlay-label');
        if (snapOverlay && highest.className) {
            snapOverlay.textContent = highest.className.toUpperCase() + ' ' + (highest.probability * 100).toFixed(0) + '%';
            snapOverlay.style.display = 'block';
        }
        // Reset dropzone text after classification completes
        const dz = document.getElementById('vision-dropzone');
        if (dz && prefix === 'vision' && !visionState['vision'].running) {
            dz.innerHTML = '<div style="font-size:1.5rem;margin-bottom:4px;">🖼</div><div style="font-family:Orbitron,monospace;font-size:0.75rem;font-weight:700;color:var(--black);margin-bottom:4px;">DROP ANOTHER IMAGE</div><div style="font-family:Share Tech Mono,monospace;font-size:0.62rem;color:var(--muted);">or click to browse</div>';
        }
        // NOTE: CV terrain never updates robot terrain indicators (flat/smooth/bump)

    } catch (e) {
        console.warn('[Predict]', e);
    }
}

function stopVision(prefix) {
    const vs = visionState[prefix];
    vs.running = false;
    if (vs.webcam) { try { vs.webcam.stop(); } catch (e) { } vs.webcam = null; }
    const container = document.getElementById(prefix + '-webcam-container');
    if (container) container.innerHTML = '';
    const dot = document.getElementById(prefix + '-vision-dot');
    if (dot) dot.className = 'vision-status-dot';
    const activeLabel = document.getElementById(prefix + '-active-label');
    if (activeLabel) { activeLabel.textContent = 'IDLE'; }
    const overlay = document.getElementById(prefix + '-start-overlay');
    if (overlay) { overlay.style.display = 'flex'; overlay.querySelector('.vision-start-text').textContent = 'TAP TO START VISION'; }
    const stopBtn = document.getElementById(prefix + '-stop-btn');
    if (stopBtn) stopBtn.style.display = 'none';
    const modeToggle = document.getElementById(prefix + '-mode-toggle');
    if (modeToggle) modeToggle.style.display = 'none';
    const urlBox = document.getElementById(prefix + '-model-url-box');
    if (urlBox) urlBox.style.display = 'block';
    const readout = document.getElementById(prefix + '-vision-readout');
    if (readout) readout.style.display = 'none';
}

function switchToSnap(prefix) {
    stopVision(prefix);
    const snapArea = document.getElementById(prefix + '-snap-area');
    if (snapArea) snapArea.style.display = 'block';
    const feedWrap = document.getElementById(prefix + '-feed-wrap');
    if (feedWrap) feedWrap.style.display = 'none';
    const snapControls = document.querySelector('#' + prefix + '-vision-section .vision-snap-controls');
    if (snapControls) snapControls.style.display = 'none';
}

function handleFileCapture(prefix, input) {
    const file = input.files[0];
    if (!file) return;
    const vs = visionState[prefix];
    const canvas = document.getElementById(prefix + '-snap-canvas');

    // Show a loading state on big label if on vision screen
    if (prefix === 'vision') {
        const bigLabel = document.getElementById('vision-big-label');
        const bigConf = document.getElementById('vision-big-conf');
        if (bigLabel) { bigLabel.textContent = 'LOADING…'; bigLabel.style.color = '#888'; }
        if (bigConf) bigConf.textContent = '…';
    }

    const reader = new FileReader();
    reader.onload = async (e) => {
        const img = new Image();
        img.onload = async () => {
            // Show preview area (vision screen only)
            const previewWrap = document.getElementById(prefix + '-preview-wrap');
            if (previewWrap) previewWrap.style.display = 'block';

            // Draw image to canvas
            canvas.style.display = 'block';
            canvas.width = img.naturalWidth;
            canvas.height = img.naturalHeight;
            const ctx = canvas.getContext('2d');
            ctx.drawImage(img, 0, 0);

            // Update dropzone to show filename
            const dz = document.getElementById('vision-dropzone');
            if (dz && prefix === 'vision') {
                dz.innerHTML = '<div style="font-size:1.5rem;margin-bottom:4px;">✅</div><div style="font-family:Share Tech Mono,monospace;font-size:0.65rem;color:var(--green);">IMAGE LOADED — classifying…</div>';
            }

            // Load model if not already loaded
            if (!vs.model) {
                try {
                    const modelUrl = getModelUrl(prefix);
                    const dot = document.getElementById(prefix + '-vision-dot');
                    if (dot) dot.className = 'vision-status-dot';

                    vs.model = await tmImage.load(modelUrl + 'model.json', modelUrl + 'metadata.json');
                    const total = vs.model.getTotalClasses();
                    vs.classes = [];
                    for (let i = 0; i < total; i++) {
                        // tmImage models expose class labels via getClassLabels() or internal metadata
                        const label = vs.model.getClassLabels ? vs.model.getClassLabels()[i] : `Class ${i}`;
                        vs.classes.push(label || `Class ${i}`);
                    }

                    if (dot) dot.className = 'vision-status-dot active';
                } catch (err) {
                    console.warn('[handleFileCapture] Model load failed:', err);
                    if (prefix === 'vision') {
                        const bigLabel = document.getElementById('vision-big-label');
                        if (bigLabel) { bigLabel.textContent = 'MODEL ERROR'; bigLabel.style.color = 'var(--danger)'; }
                        const bigConf = document.getElementById('vision-big-conf');
                        if (bigConf) bigConf.textContent = '—';
                    }
                    return;
                }
            }

            // Build/rebuild the confidence bar readout
            buildVisionReadout(prefix);
            const readoutEl = document.getElementById(prefix + '-vision-readout');
            if (readoutEl) readoutEl.style.display = 'block';

            // Run prediction
            await predictFromCanvas(prefix, canvas);
        };
        img.onerror = () => console.warn('[handleFileCapture] Image load error');
        img.src = e.target.result;
    };
    reader.readAsDataURL(file);
}

function getLastVisionClass(prefix) {
    const vs = visionState[prefix];
    if (!vs.lastClass) return null;
    return { className: vs.lastClass, confidence: vs.lastConf };
}

// ════════════════════════════════════════════════════
// VISION SCREEN — TAB SWITCHING & UPLOAD HELPERS
// ════════════════════════════════════════════════════
function switchVisionTab(tab) {
    const webcamPanel = document.getElementById('vision-tab-webcam');
    const uploadPanel = document.getElementById('vision-tab-upload');
    const tabWebcam = document.getElementById('tab-webcam');
    const tabUpload = document.getElementById('tab-upload');

    if (tab === 'webcam') {
        webcamPanel.style.display = 'block';
        uploadPanel.style.display = 'none';
        tabWebcam.style.background = 'var(--yellow)';
        tabWebcam.style.color = 'var(--black)';
        tabUpload.style.background = 'var(--beige)';
        tabUpload.style.color = 'var(--muted)';
    } else {
        webcamPanel.style.display = 'none';
        uploadPanel.style.display = 'block';
        tabUpload.style.background = 'var(--yellow)';
        tabUpload.style.color = 'var(--black)';
        tabWebcam.style.background = 'var(--beige)';
        tabWebcam.style.color = 'var(--muted)';
        // Stop webcam if running when switching to upload tab
        stopVision('vision');
    }
}

function handleVisionDrop(event) {
    event.preventDefault();
    const dropzone = document.getElementById('vision-dropzone');
    if (dropzone) dropzone.style.borderColor = '#ccc5b0';
    const files = event.dataTransfer.files;
    if (!files || !files[0]) return;
    if (!files[0].type.startsWith('image/')) {
        alert('Please drop an image file (JPG, PNG, WEBP)');
        return;
    }
    // Simulate file input change
    const dt = new DataTransfer();
    dt.items.add(files[0]);
    const input = document.getElementById('vision-file-input');
    input.files = dt.files;
    handleFileCapture('vision', input);
}

function clearVisionUpload() {
    const canvas = document.getElementById('vision-snap-canvas');
    const preview = document.getElementById('vision-preview-wrap');
    const overlay = document.getElementById('vision-snap-overlay-label');
    const readout = document.getElementById('vision-vision-readout');
    const bigLabel = document.getElementById('vision-big-label');
    const bigConf = document.getElementById('vision-big-conf');
    const dot = document.getElementById('vision-vision-dot');
    const input = document.getElementById('vision-file-input');

    if (canvas) { const ctx = canvas.getContext('2d'); ctx.clearRect(0, 0, canvas.width, canvas.height); }
    if (preview) preview.style.display = 'none';
    if (overlay) overlay.style.display = 'none';
    if (readout) readout.style.display = 'none';
    if (bigLabel) { bigLabel.textContent = '—'; bigLabel.style.color = 'var(--yellow)'; }
    if (bigConf) bigConf.textContent = '—';
    if (dot) dot.className = 'vision-status-dot';
    if (input) input.value = '';

    // Reset visionState
    const vs = visionState['vision'];
    vs.lastClass = null; vs.lastConf = 0;
}

// ════════════════════════════════════════════════════
// CONFIG
// ════════════════════════════════════════════════════
let ELEVEN_VOICE = 'Aa6nEBJJMKJwJkCx8VU2'; // Bella
let latestMissionData = null;
let currentScreen = 'screen-splash';

// ════════════════════════════════════════════════════
// SNOWFLAKE + KV SAVE
// ════════════════════════════════════════════════════
async function saveMissionToBothDBs() {
    const btn = document.getElementById('snowflake-save-btn');
    if (!latestMissionData) return;

    const tractionEl = document.getElementById(currentScreen === 'screen-observe' ? 'obs-traction' : 'test-traction');
    const traction = tractionEl ? tractionEl.value : 'standard';
    const payload = { ...latestMissionData, traction_setting: traction };

    btn.textContent = '⏳ SAVING TO BOTH...';
    btn.style.pointerEvents = 'none';

    try {
        const snowflakePromise = fetch('/api/snowflake-archive', {
            method: 'POST', headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        const kvPromise = fetch('/api/vector-save', {
            method: 'POST', headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        const [snowRes, kvRes] = await Promise.all([snowflakePromise, kvPromise]);
        if (!snowRes.ok || !kvRes.ok) throw new Error('One or more saves failed');
        btn.textContent = '✅ SAVED TO SNOWFLAKE & KV';
        btn.style.background = '#d4f5db';
        btn.style.color = '#1e7e34';
    } catch (err) {
        console.error('[Database Error]', err);
        btn.textContent = '⚠️ ERROR SAVING';
        setTimeout(() => {
            btn.textContent = '❄️ SAVE TO SNOWFLAKE';
            btn.style.background = '';
            btn.style.color = '';
            btn.style.pointerEvents = 'auto';
        }, 3000);
    }
}

// ════════════════════════════════════════════════════
// TRACTION SETTINGS
// ════════════════════════════════════════════════════
const TRACTION_INFO = {
    'dry packed/compact sand': { info: 'Stable surface. Normal speed and turning radius apply.', cls: '', safeLabel: '🔧 Traction: SET' },
    'dry loose sand': { info: '⚠ Loose surface. Reduce speed 30%. Increased wheel slip risk — wide turns recommended.', cls: 'warn-text', safeLabel: '🔧 Traction: CAUTION' },
    'wet moist sand': { info: '⛔ Wet surface. Reduce speed 50%. High skid risk — avoid sharp turns and slopes.', cls: 'danger-text', safeLabel: '🔧 Traction: HAZARD' }
};

function updateTractionInfo(prefix) {
    const sel = document.getElementById(prefix + '-traction');
    const info = document.getElementById(prefix + '-traction-info');
    if (!sel || !info) return;
    const t = TRACTION_INFO[sel.value];
    info.textContent = t.info;
    info.className = 'traction-info' + (t.cls ? ' ' + t.cls : '');
    const siEl = document.getElementById(prefix === 'obs' ? 'si-traction' : 'tsi-traction');
    if (siEl) {
        siEl.textContent = t.safeLabel;
        siEl.className = 'safety-ind ' + (sel.value === 'dry packed/compact sand' ? 'ok' : sel.value === 'dry loose sand' ? 'warn' : 'danger');
    }
}

// ════════════════════════════════════════════════════
// ELEVENLABS TTS
// ════════════════════════════════════════════════════
let audioQueue = [], isSpeaking = false, currentAudio = null;

function terrainLabel(t) { return t === 'flat' ? 'flat' : t === 'smooth' ? 'somewhat smooth' : 'bumpy'; }

function buildNarration(step, sensor, idx, total) {
    const terrain = terrainLabel(sensor.terrain);
    if (idx === 0) return 'Sandozer Mission Control online. Beginning sandbox traversal. Starting at position zero zero. All sensors active.';
    if (idx === total - 1) return `Traversal complete. Final position: row ${step.row}, column ${step.col}. Vertical tilt ${sensor.tilt.toFixed(3)} degrees. Terrain is ${terrain}. Air quality index ${sensor.airQuality}. Mission scan finished.`;
    return `Step ${idx} of ${total - 1}. Moving to row ${step.row}, column ${step.col}. Tilt ${sensor.tilt.toFixed(3)} degrees. Terrain: ${terrain}. Air quality: ${sensor.airQuality}.`;
}

async function speak(text, pillId, labelId, onComplete) {
    audioQueue.push({ text, pillId, labelId, onComplete });
    if (!isSpeaking) drainQueue();
}

async function drainQueue() {
    if (!audioQueue.length) { isSpeaking = false; return; }
    isSpeaking = true;
    const { text, pillId, labelId, onComplete } = audioQueue.shift();
    setVoicePill(pillId, labelId, true);
    try {
        const res = await fetch('/api/tts', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ text, voiceId: ELEVEN_VOICE }) });
        if (!res.ok) throw new Error('TTS error ' + res.status);
        const blob = await res.blob();
        const url = URL.createObjectURL(blob);
        currentAudio = new Audio(url);
        currentAudio.onended = () => { URL.revokeObjectURL(url); setVoicePill(pillId, labelId, false); currentAudio = null; drainQueue(); if (!audioQueue.length && onComplete) setTimeout(onComplete, 150); };
        currentAudio.onerror = () => { setVoicePill(pillId, labelId, false); currentAudio = null; drainQueue(); };
        await currentAudio.play();
    } catch (e) {
        console.warn('[TTS]', e.message);
        setVoicePill(pillId, labelId, false); currentAudio = null; drainQueue();
        if (onComplete) setTimeout(onComplete, 150);
    }
}

function setVoicePill(pillId, labelId, active) {
    const pill = document.getElementById(pillId), label = document.getElementById(labelId);
    if (pill) pill.classList.toggle('active', active);
    if (label) label.textContent = active ? 'SPEAKING…' : 'VOICE IDLE';
}

function stopAllAudio() {
    audioQueue = []; isSpeaking = false;
    if (currentAudio) { currentAudio.pause(); currentAudio.src = ''; currentAudio = null; }
    ['obs-voice-pill', 'test-voice-pill', 'res-voice-pill', 'chat-voice-pill'].forEach(id => { const el = document.getElementById(id); if (el) el.classList.remove('active'); });
    ['obs-voice-label', 'test-voice-label', 'res-voice-label', 'chat-voice-label'].forEach(id => { const el = document.getElementById(id); if (el) el.textContent = 'VOICE IDLE'; });
}

// ════════════════════════════════════════════════════
// EMERGENCY STOP
// ════════════════════════════════════════════════════
function emergencyStop() {
    clearInterval(obsTimer); obsPaused = true; stopAllAudio();
    const pauseBtn = document.getElementById('obs-pause-btn');
    if (pauseBtn) pauseBtn.textContent = '▶';
    const pillId = currentScreen === 'screen-test' ? 'test-voice-pill' : 'obs-voice-pill';
    const labelId = currentScreen === 'screen-test' ? 'test-voice-label' : 'obs-voice-label';
    speak('Emergency stop activated. Robot halted. All movement suspended.', pillId, labelId);
}

function showEStop(show) { document.getElementById('estop-btn').className = show ? 'active' : ''; }

// ════════════════════════════════════════════════════
// WALL PROXIMITY SAFETY
// ════════════════════════════════════════════════════
let lastWallZone = -1;
function checkWallProximity(row, col, stepIdx) {
    const wallDist = Math.min(row, col, 4 - row, 4 - col);
    const wrap = document.getElementById('obs-wall-alert-wrap');
    const screenPfx = (currentScreen === 'screen-observe') ? 'si' : 'tsi';
    const siEl = document.getElementById(screenPfx + '-boundary');
    if (stepIdx === 0) { lastWallZone = wallDist; if (wrap) wrap.innerHTML = ''; if (siEl) { siEl.textContent = '📍 Boundary: SAFE'; siEl.className = 'safety-ind ok'; } return; }
    const pillId = currentScreen === 'screen-observe' ? 'obs-voice-pill' : 'test-voice-pill';
    const labelId = currentScreen === 'screen-observe' ? 'obs-voice-label' : 'test-voice-label';
    const zoneChanged = wallDist !== lastWallZone;
    lastWallZone = wallDist;
    if (wallDist === 0) {
        if (wrap) wrap.innerHTML = `<span class="wall-alert">⛔ AT BOUNDARY</span>`;
        if (siEl) { siEl.textContent = '📍 Boundary: AT WALL'; siEl.className = 'safety-ind danger'; }
        if (zoneChanged) speak('Warning. Robot is at the boundary.', pillId, labelId);
    } else if (wallDist === 1) {
        if (wrap) wrap.innerHTML = `<span class="wall-alert">⚠ APPROACHING BOUNDARY</span>`;
        if (siEl) { siEl.textContent = '📍 Boundary: WARNING'; siEl.className = 'safety-ind danger'; }
        if (zoneChanged) speak('Approaching boundary.', pillId, labelId);
    } else if (wallDist === 2) {
        if (wrap) wrap.innerHTML = `<span class="wall-alert warn-yellow">⚠ NEAR BOUNDARY</span>`;
        if (siEl) { siEl.textContent = '📍 Boundary: CAUTION'; siEl.className = 'safety-ind warn'; }
    } else {
        if (wrap) wrap.innerHTML = '';
        if (siEl) { siEl.textContent = '📍 Boundary: SAFE'; siEl.className = 'safety-ind ok'; }
    }
}

function checkAQSafety(aq, prefix, pillId, labelId, stepIdx) {
    const siEl = document.getElementById(prefix + '-aq');
    if (!siEl) return;
    if (aq > 150) {
        siEl.textContent = '💨 Air: DANGEROUS'; siEl.className = 'safety-ind danger';
        if (stepIdx !== undefined && stepIdx % 3 === 0) speak(`Air quality alert. AQI is ${aq}. Conditions are hazardous.`, pillId, labelId);
    } else if (aq > 100) {
        siEl.textContent = '💨 Air: MODERATE'; siEl.className = 'safety-ind warn';
    } else {
        siEl.textContent = '💨 Air: GOOD'; siEl.className = 'safety-ind ok';
    }
}

function checkTiltSafety(tilt, terrain, prefix) {
    const siEl = document.getElementById(prefix + '-tilt');
    if (!siEl) return;
    if (terrain === 'bump') { siEl.textContent = '↗ Tilt: HIGH RISK'; siEl.className = 'safety-ind danger'; }
    else if (terrain === 'smooth') { siEl.textContent = '↗ Tilt: ELEVATED'; siEl.className = 'safety-ind warn'; }
    else { siEl.textContent = '↗ Tilt: NORMAL'; siEl.className = 'safety-ind ok'; }
}

// ════════════════════════════════════════════════════
// NAVIGATION
// ════════════════════════════════════════════════════
function goTo(screenId) {
    document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));
    document.getElementById(screenId).classList.add('active');
    currentScreen = screenId;
    showEStop(screenId === 'screen-observe' || screenId === 'screen-test');
    if (screenId !== 'screen-observe') stopAllAudio();
    if (screenId === 'screen-observe') initObserve();
    if (screenId === 'screen-test') {
        initTestGrid(); updateTractionInfo('test');
        speak('Hello! What position would you like to navigate to? Say two numbers for row and column, each between 0 and 4.', 'test-voice-pill', 'test-voice-label',
            () => { if (currentScreen === 'screen-test') { continuousListen = true; startGlobalMic(); } });
    }
    // Stop all vision when leaving respective screens
    if (screenId !== 'screen-observe') stopVision('obs');
    if (screenId !== 'screen-test') stopVision('test');
    if (screenId !== 'screen-vision') stopVision('vision');
    if (screenId === 'screen-chat') initChat();
    updateMicContext(screenId);
}

let selectedMode = null;
function selectMode(mode) {
    selectedMode = mode;
    document.getElementById('btn-observe').classList.toggle('selected', mode === 'observe');
    document.getElementById('btn-test').classList.toggle('selected', mode === 'test');
    document.getElementById('btn-vision').classList.toggle('selected', mode === 'vision');
    document.getElementById('btn-chat').classList.toggle('selected', mode === 'chat');
    const cont = document.getElementById('mode-continue');
    cont.style.opacity = '1'; cont.style.pointerEvents = 'auto';
}
function continueMode() {
    if (!selectedMode) return;
    const dest = selectedMode === 'observe' ? 'screen-observe' : selectedMode === 'test' ? 'screen-test' : selectedMode === 'vision' ? 'screen-vision' : 'screen-chat';
    goTo(dest);
}

// ════════════════════════════════════════════════════
// GRID BUILDER
// ════════════════════════════════════════════════════
const CELL = 44, GAP = 3;
function buildMiniGrid(tableEl) {
    tableEl.innerHTML = '';
    for (let r = 0; r < 5; r++) {
        const tr = document.createElement('tr');
        for (let c = 0; c < 5; c++) {
            const td = document.createElement('td');
            td.className = 'mini-cell';
            td.innerHTML = `<span class="mc-label">${r},${c}</span>`;
            tr.appendChild(td);
        }
        tableEl.appendChild(tr);
    }
}

function cellCenter(row, col) { return { x: 3 + 6 + col * (CELL + 3) + CELL / 2, y: 3 + 6 + row * (CELL + 3) + CELL / 2 }; }
function gridTotalSize() { return (3 + 6) + 5 * (CELL + 3); }

const DIRS = ['E', 'S', 'W', 'N'];
const DELTA = { E: [0, 1], S: [1, 0], W: [0, -1], N: [-1, 0] };
const DIR_LABEL = { E: 'East →', S: 'South ↓', W: 'West ←', N: 'North ↑' };

function parseCommands(text) {
    const tokens = text.toLowerCase().match(/[fblr]/g);
    if (!tokens || !tokens.length) return null;
    const path = [];
    let row = 0, col = 0, di = 0;
    path.push({ row, col, dir: DIRS[di], cmd: 'START' });
    for (const cmd of tokens) {
        if (cmd === 'l') { di = (di + 3) % 4; path.push({ row, col, dir: DIRS[di], cmd: 'L' }); }
        else if (cmd === 'r') { di = (di + 1) % 4; path.push({ row, col, dir: DIRS[di], cmd: 'R' }); }
        else {
            const [dr, dc] = DELTA[DIRS[di]];
            row = Math.max(0, Math.min(4, row + (cmd === 'f' ? dr : -dr)));
            col = Math.max(0, Math.min(4, col + (cmd === 'f' ? dc : -dc)));
            path.push({ row, col, dir: DIRS[di], cmd: cmd === 'f' ? 'F' : 'B' });
        }
    }
    return path;
}

// ════════════════════════════════════════════════════
// SENSOR DATA (Simulated)
// ════════════════════════════════════════════════════
const TILT_MAP = (() => {
    const m = [];
    for (let r = 0; r < 5; r++) { m[r] = []; for (let c = 0; c < 5; c++)m[r][c] = Math.round(Math.random() * 300) / 1000; }
    m[0][0] = Math.round(Math.random() * 49) / 1000;
    return m;
})();
const ALTITUDE_MAP = (() => { const m = []; for (let r = 0; r < 5; r++) { m[r] = []; for (let c = 0; c < 5; c++)m[r][c] = Math.round(Math.random() * 150) / 1000; } return m; })();
const AIR_QUALITY_MAP = (() => { const m = []; for (let r = 0; r < 5; r++) { m[r] = []; for (let c = 0; c < 5; c++)m[r][c] = Math.round((40 + Math.random() * 160) / 5) * 5; } return m; })();

function tiltToTerrain(t) { return t < 0.05 ? 'flat' : t <= 0.15 ? 'smooth' : 'bump'; }
function getSensorData(row, col) {
    const tilt = TILT_MAP[row][col], terrain = tiltToTerrain(tilt);
    const alt = ALTITUDE_MAP[row][col], wallDist = Math.min(row, col, 4 - row, 4 - col), airQuality = AIR_QUALITY_MAP[row][col];
    return { tilt, terrain, alt, wallDist, airQuality };
}

function drawPath(svgEl, steps, upTo) {
    const sz = gridTotalSize();
    svgEl.setAttribute('viewBox', `0 0 ${sz} ${sz}`);
    svgEl.setAttribute('width', sz); svgEl.setAttribute('height', sz);
    svgEl.innerHTML = '';
    if (upTo < 1) return;
    const pts = [];
    for (let i = 0; i <= upTo; i++) {
        const { x, y } = cellCenter(steps[i].row, steps[i].col);
        if (!pts.length || pts[pts.length - 1].x !== x || pts[pts.length - 1].y !== y) pts.push({ x, y });
    }
    if (pts.length < 2) return;
    const ptStr = pts.map(p => `${p.x},${p.y}`).join(' ');
    svgEl.appendChild(makePoly(ptStr, '#b0a890', 11));
    svgEl.appendChild(makePoly(ptStr, '#1a1a1a', 7));
    const dash = makePoly(ptStr, '#f5c800', 2); dash.setAttribute('stroke-dasharray', '6 8'); svgEl.appendChild(dash);
    const seen = new Set();
    for (let i = 0; i <= upTo; i++) {
        const { row, col } = steps[i], key = `${row},${col}`;
        if (!seen.has(key)) {
            seen.add(key);
            const { x, y } = cellCenter(row, col);
            const c = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
            c.setAttribute('cx', x); c.setAttribute('cy', y); c.setAttribute('r', i === 0 ? 5 : 4);
            c.setAttribute('fill', i === 0 ? '#f5c800' : '#1a1a1a');
            c.setAttribute('stroke', '#fff'); c.setAttribute('stroke-width', '1.5');
            svgEl.appendChild(c);
        }
    }
}

function makePoly(pts, stroke, width) {
    const el = document.createElementNS('http://www.w3.org/2000/svg', 'polyline');
    el.setAttribute('points', pts); el.setAttribute('stroke', stroke);
    el.setAttribute('stroke-width', width); el.setAttribute('stroke-linecap', 'round');
    el.setAttribute('stroke-linejoin', 'round'); el.setAttribute('fill', 'none');
    return el;
}

function moveRobot(robotEl, row, col) {
    const { x, y } = cellCenter(row, col);
    robotEl.style.left = x + 'px'; robotEl.style.top = y + 'px'; robotEl.style.display = 'block';
}

function setTerrainHighlight(prefix, terrain) {
    ['flat', 'smooth', 'bump'].forEach(t => document.getElementById(prefix + t)?.classList.toggle('active', terrain === t));
}

// ════════════════════════════════════════════════════
// OBSERVE SCREEN
// ════════════════════════════════════════════════════
let obsSteps = [], obsStep = 0, obsPaused = false, obsTimer = null, obsElapsed = 0;
const OBS_INTERVAL = 10000;

function initObserve() {
    clearInterval(obsTimer); stopAllAudio(); lastWallZone = -1; obsStep = 0; obsPaused = false;
    buildMiniGrid(document.getElementById('obs-grid'));
    const sz = gridTotalSize(); const svg = document.getElementById('obs-svg');
    svg.setAttribute('width', sz); svg.setAttribute('height', sz); svg.innerHTML = '';
    document.getElementById('obs-robot').style.display = 'none';
    document.getElementById('obs-log').innerHTML = '';
    document.getElementById('obs-wall-alert-wrap').innerHTML = '';
    updateTractionInfo('obs');
    speak('Observation mode ready. Please set traction, enter path commands, and press Start.', 'obs-voice-pill', 'obs-voice-label',
        () => { if (currentScreen === 'screen-observe') { continuousListen = true; startGlobalMic(); } });
}

function startObserveSequence() {
    let cmds = document.getElementById('obs-commands').value.trim().replace('🎤 ', '');
    if (!cmds) cmds = 'f f f r f f';
    obsSteps = parseCommands(cmds) || [{ row: 0, col: 0, dir: 'E', cmd: 'START' }];
    obsStep = 0; obsPaused = false; lastWallZone = -1;
    buildMiniGrid(document.getElementById('obs-grid'));
    buildObsLog();
    const traction = document.getElementById('obs-traction').value;
    let safetyMsg = `Commencing patrol. Safety traction optimized for ${traction}.`;
    if (traction === 'wet moist sand') safetyMsg += ' High slip risk active.';
    else if (traction === 'dry loose sand') safetyMsg += ' Caution: loose terrain.';
    speak(safetyMsg, 'obs-voice-pill', 'obs-voice-label');
    renderObsStep(0); startObsTimer();
}

function buildObsLog() {
    const log = document.getElementById('obs-log'); log.innerHTML = '';
    obsSteps.forEach((s, i) => {
        const chip = document.createElement('span');
        chip.className = 'log-chip' + (i === 0 ? ' current' : '');
        chip.textContent = `(${s.row},${s.col})`; log.appendChild(chip);
    });
}

function renderObsStep(idx) {
    const s = obsSteps[idx]; if (!s) return;
    const sensor = getSensorData(s.row, s.col);
    moveRobot(document.getElementById('obs-robot'), s.row, s.col);
    drawPath(document.getElementById('obs-svg'), obsSteps, idx);
    document.getElementById('obs-x').textContent = s.col;
    document.getElementById('obs-y').textContent = s.row;
    document.getElementById('obs-alt').textContent = sensor.alt.toFixed(2) + ' m';
    const tiltEl = document.getElementById('obs-tilt');
    tiltEl.textContent = sensor.tilt.toFixed(3) + '°';
    tiltEl.style.color = sensor.terrain === 'flat' ? 'var(--green)' : sensor.terrain === 'smooth' ? '#b87900' : '#c0392b';
    document.getElementById('obs-wall').textContent = sensor.wallDist + ' cell' + (sensor.wallDist !== 1 ? 's' : '');
    document.getElementById('obs-step-badge').textContent = `STEP ${idx}/${obsSteps.length - 1}`;
    const aqEl = document.getElementById('obs-aq'), aq = sensor.airQuality;
    aqEl.textContent = aq + ' AQI — ' + (aq <= 100 ? 'Good' : aq <= 150 ? 'Moderate' : 'Poor');
    aqEl.className = 'obs-val ' + (aq <= 100 ? 'aq-good' : aq <= 150 ? 'aq-moderate' : 'aq-poor');

    // Robot terrain only — CV terrain on Vision screen is separate
    setTerrainHighlight('t-', sensor.terrain);
    checkTiltSafety(sensor.tilt, sensor.terrain, 'si');
    checkAQSafety(aq, 'si', 'obs-voice-pill', 'obs-voice-label', idx);

    document.querySelectorAll('#obs-log .log-chip').forEach((el, i) => {
        el.className = 'log-chip' + (i < idx ? ' done' : i === idx ? ' current' : '');
    });
    const cur = document.querySelector('#obs-log .current');
    if (cur) cur.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
    checkWallProximity(s.row, s.col, idx);
    const text = buildNarration(s, sensor, idx, obsSteps.length);
    speak(text, 'obs-voice-pill', 'obs-voice-label');
    if (idx === obsSteps.length - 1) setTimeout(() => showResult(obsSteps, s.row, s.col), 900);
}

function startObsTimer() {
    clearInterval(obsTimer); obsElapsed = 0; updateObsTimerUI();
    obsTimer = setInterval(() => {
        if (obsPaused) return;
        obsElapsed += 100; updateObsTimerUI();
        if (obsElapsed >= OBS_INTERVAL) {
            obsElapsed = 0;
            if (obsStep < obsSteps.length - 1) { obsStep++; renderObsStep(obsStep); }
            else { clearInterval(obsTimer); document.getElementById('obs-timer-fill').style.width = '0%'; }
        }
    }, 100);
}

function updateObsTimerUI() {
    const rem = Math.max(0, OBS_INTERVAL - obsElapsed);
    document.getElementById('obs-timer-fill').style.width = ((rem / OBS_INTERVAL) * 100) + '%';
    document.getElementById('obs-countdown').textContent = (rem / 1000).toFixed(1) + 's';
}

function toggleObsPause() {
    obsPaused = !obsPaused;
    document.getElementById('obs-pause-btn').textContent = obsPaused ? '▶' : '⏸';
}

// ════════════════════════════════════════════════════
// BFS
// ════════════════════════════════════════════════════
function bfsPath(tr, tc) {
    if (tr === 0 && tc === 0) return [{ row: 0, col: 0 }];
    const visited = Array.from({ length: 5 }, () => Array(5).fill(false));
    const parent = Array.from({ length: 5 }, () => Array(5).fill(null));
    const q = [{ row: 0, col: 0 }]; visited[0][0] = true;
    const dirs = [[0, 1], [1, 0], [0, -1], [-1, 0]];
    while (q.length) {
        const { row, col } = q.shift();
        if (row === tr && col === tc) {
            const path = []; let cur = { row, col };
            while (cur) { path.unshift(cur); cur = parent[cur.row][cur.col]; }
            return path;
        }
        for (const [dr, dc] of dirs) {
            const nr = row + dr, nc = col + dc;
            if (nr >= 0 && nr < 5 && nc >= 0 && nc < 5 && !visited[nr][nc]) { visited[nr][nc] = true; parent[nr][nc] = { row, col }; q.push({ row: nr, col: nc }); }
        }
    }
    return [{ row: 0, col: 0 }];
}

function bfsToSteps(cellPath) {
    return cellPath.map((p, i) => ({ row: p.row, col: p.col, dir: 'E', cmd: i === 0 ? 'START' : 'F' }));
}

// ════════════════════════════════════════════════════
// TEST SCREEN
// ════════════════════════════════════════════════════
function initTestGrid() {
    buildMiniGrid(document.getElementById('test-grid'));
    const sz = gridTotalSize(), svg = document.getElementById('test-svg');
    svg.setAttribute('width', sz); svg.setAttribute('height', sz); svg.innerHTML = '';
    document.getElementById('test-robot').style.display = 'none';
    moveRobot(document.getElementById('test-robot'), 0, 0);
}

function runTest() {
    const targetText = document.getElementById('test-target').value.trim();
    const err = document.getElementById('test-err');
    const m = targetText.match(/(\d)\s*,\s*(\d)/);
    if (!m || +m[1] > 4 || +m[2] > 4) {
        err.style.display = 'block'; err.textContent = 'Invalid target. Use row,col with values 0–4.';
        setTimeout(() => err.style.display = 'none', 3000); return;
    }
    const targetRow = +m[1], targetCol = +m[2];
    const traction = document.getElementById('test-traction').value;
    speak(`Commencing test traversal to row ${targetRow}, column ${targetCol}. Traction optimized for ${traction}.`, 'test-voice-pill', 'test-voice-label');
    animateTest(bfsToSteps(bfsPath(targetRow, targetCol)), targetRow, targetCol);
}

function animateTest(steps, targetRow, targetCol) {
    buildMiniGrid(document.getElementById('test-grid'));
    const sz = gridTotalSize(), svg = document.getElementById('test-svg');
    svg.setAttribute('width', sz); svg.setAttribute('height', sz); svg.innerHTML = '';
    const robot = document.getElementById('test-robot');
    let i = 0; lastWallZone = -1;
    function next() {
        drawPath(svg, steps, i); moveRobot(robot, steps[i].row, steps[i].col);
        const sensor = getSensorData(steps[i].row, steps[i].col);
        checkWallProximity(steps[i].row, steps[i].col, i);
        checkTiltSafety(sensor.tilt, sensor.terrain, 'tsi');
        checkAQSafety(sensor.airQuality, 'tsi', 'test-voice-pill', 'test-voice-label', i);
        if (i < steps.length - 1) { i++; setTimeout(next, 700); }
        else { setTimeout(() => showResult(steps, targetRow, targetCol), 900); }
    }
    next();
}

// ════════════════════════════════════════════════════
// RESULT SCREEN
// ════════════════════════════════════════════════════
function showResult(steps, targetRow, targetCol) {
    const last = steps[steps.length - 1];
    const sensor = getSensorData(last.row, last.col);

    latestMissionData = {
        mission_type: targetRow !== undefined ? 'Test Navigation' : 'Observation Patrol',
        end_position: [last.row, last.col],
        terrain_classification: sensor.terrain,

        tilt_degrees: sensor.tilt,
        air_quality_aqi: sensor.airQuality,
        path_taken: steps.map(s => `[${s.row},${s.col}]`).join(' -> '),
        timestamp: new Date().toISOString()
    };

    const btn = document.getElementById('snowflake-save-btn');
    if (btn) { btn.textContent = '❄️ SAVE TO SNOWFLAKE'; btn.style.background = ''; btn.style.color = ''; btn.style.pointerEvents = 'auto'; }

    goTo('screen-result');
    buildMiniGrid(document.getElementById('res-grid'));
    const sz = gridTotalSize(), resSvg = document.getElementById('res-svg');
    resSvg.setAttribute('width', sz); resSvg.setAttribute('height', sz);
    drawPath(resSvg, steps, steps.length - 1);
    moveRobot(document.getElementById('res-robot'), last.row, last.col);

    document.getElementById('res-pos').textContent = `(${last.row}, ${last.col})`;
    document.getElementById('res-dir').textContent = DIR_LABEL[last.dir];

    const reached = last.row === targetRow && last.col === targetCol;
    const reachEl = document.getElementById('res-reached');
    if (targetRow === undefined) { reachEl.textContent = '✓ Patrol Finished'; reachEl.style.color = 'var(--green)'; }
    else { reachEl.textContent = reached ? '✓ YES — destination reached!' : `✗ NO — ended at (${last.row},${last.col})`; reachEl.style.color = reached ? 'var(--green)' : 'var(--danger)'; }

    const tiltEl = document.getElementById('res-tilt');
    tiltEl.textContent = sensor.tilt.toFixed(3) + '°';
    tiltEl.style.color = sensor.terrain === 'flat' ? 'var(--green)' : sensor.terrain === 'smooth' ? '#b87900' : 'var(--danger)';

    const resAqEl = document.getElementById('res-aq'), aq = sensor.airQuality;
    resAqEl.textContent = aq + ' AQI — ' + (aq <= 100 ? 'Safe' : aq <= 150 ? 'Medium' : 'Dangerous');
    resAqEl.className = 'result-val ' + (aq <= 100 ? 'aq-good' : aq <= 150 ? 'aq-moderate' : 'aq-poor');

    setTerrainHighlight('rt-', sensor.terrain);

    const resLog = document.getElementById('res-log'); resLog.innerHTML = '';
    steps.forEach((s, i) => {
        const chip = document.createElement('span'); chip.className = 'log-chip done';
        chip.textContent = i === 0 ? `START(${s.row},${s.col})` : `→(${s.row},${s.col})`; resLog.appendChild(chip);
    });

    const terrain = terrainLabel(sensor.terrain);
    const terrainDetail = sensor.terrain === 'flat' ? 'No leveling required.' : sensor.terrain === 'smooth' ? 'Moderate leveling recommended.' : 'High priority for leveling.';
    const narration = (reached || targetRow === undefined)
        ? `Mission complete. Destination reached at row ${last.row}, column ${last.col}. Terrain is ${terrain}. ${terrainDetail} Air quality index is ${aq}.`
        : `Mission ended at row ${last.row}, column ${last.col}. Target was not reached. Terrain is ${terrain}.`;
    speak(narration, 'res-voice-pill', 'res-voice-label');
}

// ════════════════════════════════════════════════════
// GEMINI CHATBOT + SNOWFLAKE RAG
// ════════════════════════════════════════════════════
const SANDOZER_SYSTEM = `You are the Sandozer AI assistant — an expert on the Sandozer robot built in partnership with John Deere.
Key facts:
- Hardware: Arduino Elegoo Owlbot Tank Kit with bulldozer-style flat blade
- Navigation: 5×5 grid, BFS shortest-path
- Sensors: MPU-6050 IMU (tilt), HC-SR04 ultrasonic (wall), air quality, altitude
- Vision: Google Teachable Machine CNN for terrain classification (desert/forest/mountain/plains)
- Terrain: Flat (<0.05°), Somewhat Smooth (0.05–0.15°), Bumpy (>0.15°)
- Safety: Boundary alerts ≤2 cells from wall
- Traction Config: Dry Packed (normal), Dry Loose (−30% speed), Wet Moist (−50% speed)
- Database: Snowflake for mission logs + KV vector store for RAG
Answer in 2–4 sentences.`;

let chatHistory = [];
function initChat() { chatHistory = []; }

async function sendChat() {
    const input = document.getElementById('chat-input');
    const msg = input.value.trim(); if (!msg) return; input.value = '';
    appendChatBubble(msg, 'user');
    chatHistory.push({ role: 'user', parts: [{ text: msg }] });
    const useRag = document.getElementById('use-rag-toggle').checked;
    const typingEl = appendChatBubble(useRag ? '🔍 Querying Snowflake...' : 'Thinking…', 'typing');
    try {
        let contextAddendum = '';
        if (useRag) {
            try {
                const ragRes = await fetch('/api/vector-search', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ query: msg }) });
                if (ragRes.ok) { const ragData = await ragRes.json(); if (ragData.context) contextAddendum = `\n\n[SNOWFLAKE CONTEXT]: ${ragData.context}`; }
            } catch (e) { console.warn('Vector search not connected', e); }
        }
        const systemPrompt = SANDOZER_SYSTEM + contextAddendum;
        const requestBody = { systemInstruction: { parts: [{ text: systemPrompt }] }, contents: chatHistory, generationConfig: { maxOutputTokens: 300, temperature: 0.7 } };
        typingEl.textContent = '🧠 Analyzing…';
        const res = await fetch('/api/gemini', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(requestBody) });
        const data = await res.json();
        if (!res.ok || data.error) throw new Error(data.error?.message || 'API Error');
        const reply = data.candidates?.[0]?.content?.parts?.[0]?.text || 'No response.';
        chatHistory.push({ role: 'model', parts: [{ text: reply }] });
        typingEl.className = 'chat-bubble bot'; typingEl.textContent = reply;
        document.getElementById('chat-messages').scrollTop = 99999;
        speak(reply, 'chat-voice-pill', 'chat-voice-label');
    } catch (e) {
        typingEl.className = 'chat-bubble bot'; typingEl.textContent = '⚠ ' + e.message;
    }
}

function appendChatBubble(text, type) {
    const msgs = document.getElementById('chat-messages');
    const wrap = document.createElement('div'); wrap.className = 'chat-bubble-wrap';
    if (type === 'bot' || type === 'typing') { const name = document.createElement('div'); name.className = 'bot-name'; name.textContent = 'SANDOZER AI'; wrap.appendChild(name); }
    const bubble = document.createElement('div'); bubble.className = `chat-bubble ${type}`; bubble.textContent = text;
    wrap.appendChild(bubble); msgs.appendChild(wrap); msgs.scrollTop = msgs.scrollHeight;
    return bubble;
}

// ════════════════════════════════════════════════════
// COMMAND MIC
// ════════════════════════════════════════════════════
let cmdMicActive = false, cmdMicRecognition = null;
function toggleCmdMic() { if (cmdMicActive) stopCmdMic(); else startCmdMic(); }
function startCmdMic() {
    const SR = window.SpeechRecognition || window.webkitSpeechRecognition;
    if (!SR) { alert('Web Speech not supported. Use Chrome.'); return; }
    cmdMicRecognition = new SR(); cmdMicRecognition.lang = 'en-US'; cmdMicRecognition.interimResults = false;
    cmdMicRecognition.onstart = () => { cmdMicActive = true; document.getElementById('obs-cmd-mic').classList.add('listening'); document.getElementById('obs-cmd-mic-status').style.display = 'block'; };
    cmdMicRecognition.onresult = (e) => {
        const raw = e.results[0][0].transcript.toLowerCase().trim();
        const normalized = normalizeDirections(raw);
        if (normalized.length > 0) { document.getElementById('obs-commands').value = normalized; setTimeout(startObserveSequence, 400); }
    };
    cmdMicRecognition.onend = () => { cmdMicActive = false; document.getElementById('obs-cmd-mic').classList.remove('listening'); document.getElementById('obs-cmd-mic-status').style.display = 'none'; };
    cmdMicRecognition.onerror = () => { cmdMicActive = false; document.getElementById('obs-cmd-mic').classList.remove('listening'); document.getElementById('obs-cmd-mic-status').style.display = 'none'; };
    cmdMicRecognition.start();
}
function stopCmdMic() {
    if (cmdMicRecognition) { try { cmdMicRecognition.stop(); } catch (e) { } }
    cmdMicActive = false; document.getElementById('obs-cmd-mic').classList.remove('listening'); document.getElementById('obs-cmd-mic-status').style.display = 'none';
}

// ════════════════════════════════════════════════════
// GLOBAL SPEECH RECOGNITION
// ════════════════════════════════════════════════════
let globalRecognition = null, chatRecognition = null, globalMicActive = false, chatMicActive = false, continuousListen = false;

function normalizeNumbers(t) {
    return t.replace(/\bzero\b/g, '0').replace(/\bone\b/g, '1').replace(/\b(two|too)\b/g, '2').replace(/\bthree\b/g, '3').replace(/\b(four)\b/g, '4').replace(/\boh\b/g, '0');
}
function normalizeDirections(t) {
    return t.toLowerCase().replace(/\b(forward|straight)\b/g, 'f ').replace(/\bback(ward)?\b/g, 'b ').replace(/\bleft\b/g, 'l ').replace(/\bright\b/g, 'r ').replace(/[^fblr\s]/g, '').replace(/\s+/g, ' ').trim();
}
function parseTestCoords(transcript) {
    const t = normalizeNumbers(transcript.toLowerCase());
    let m = t.match(/row\s*(\d).*?col(?:umn)?\s*(\d)/); if (m) return m;
    m = t.match(/(?:navigate|go)\s+to\s+(\d)\D*(\d)/); if (m) return m;
    m = t.match(/(\d)\s*[,.]\s*(\d)/); if (m) return m;
    const digits = [...t.matchAll(/\b(\d)\b/g)];
    if (digits.length >= 2) return [null, digits[0][1], digits[1][1]];
    return null;
}
function updateMicContext(screenId) {
    currentScreen = screenId;
    if (screenId !== 'screen-test' && screenId !== 'screen-observe') { continuousListen = false; stopGlobalMic(); }
}
function buildSpeechRecognition() {
    const SR = window.SpeechRecognition || window.webkitSpeechRecognition;
    if (!SR) return null;
    const rec = new SR(); rec.lang = 'en-US'; rec.interimResults = false; rec.maxAlternatives = 3; return rec;
}
function toggleGlobalMic() { if (globalMicActive) { continuousListen = false; stopGlobalMic(); } else startGlobalMic(); }
function startGlobalMic() {
    if (globalMicActive) return;
    const SR = window.SpeechRecognition || window.webkitSpeechRecognition;
    if (!SR) { alert('Web Speech API not supported. Use Chrome.'); return; }
    globalRecognition = buildSpeechRecognition();
    globalRecognition.onstart = () => { globalMicActive = true; document.getElementById('mic-fab').classList.add('listening'); document.getElementById('mic-status').classList.add('show'); };
    globalRecognition.onresult = (e) => {
        const transcripts = Array.from(e.results[0]).map(alt => alt.transcript.toLowerCase().trim());
        if (currentScreen === 'screen-test') document.getElementById('test-target').value = '🎤 ' + transcripts[0];
        else if (currentScreen === 'screen-observe') document.getElementById('obs-commands').value = '🎤 ' + transcripts[0];
        for (const t of transcripts) { if (handleVoiceCommand(t)) break; }
    };
    globalRecognition.onend = () => {
        globalMicActive = false;
        if (continuousListen && (currentScreen === 'screen-test' || currentScreen === 'screen-observe') && !isSpeaking) { setTimeout(() => { if (continuousListen) startGlobalMic(); }, 350); }
        else { document.getElementById('mic-fab').classList.remove('listening'); document.getElementById('mic-status').classList.remove('show'); }
    };
    globalRecognition.onerror = (e) => {
        globalMicActive = false;
        if (continuousListen && e.error !== 'not-allowed' && e.error !== 'service-unavailable') { setTimeout(() => { if (continuousListen) startGlobalMic(); }, 500); }
        else { document.getElementById('mic-fab').classList.remove('listening'); document.getElementById('mic-status').classList.remove('show'); }
    };
    globalRecognition.start();
}
function stopGlobalMic() {
    continuousListen = false;
    if (globalRecognition) { try { globalRecognition.stop(); } catch (e) { } }
    globalMicActive = false;
    document.getElementById('mic-fab').classList.remove('listening');
    document.getElementById('mic-status').classList.remove('show');
}
function handleVoiceCommand(transcript) {
    const screen = currentScreen;
    if (/\b(emergency stop|stop now|halt|e.?stop)\b/.test(transcript) && (screen === 'screen-observe' || screen === 'screen-test')) { emergencyStop(); return true; }
    if (/\b(back|go back|menu|home)\b/.test(transcript) && screen !== 'screen-splash' && screen !== 'screen-mode') { goTo('screen-mode'); return true; }
    if (screen === 'screen-splash') { if (/\b(start|begin|launch|go|continue)\b/.test(transcript)) { goTo('screen-mode'); return true; } }
    else if (screen === 'screen-mode') {
        if (/\bobserv/.test(transcript)) { selectMode('observe'); continueMode(); return true; }
        if (/\btest\b/.test(transcript)) { selectMode('test'); continueMode(); return true; }
        if (/\bchat\b/.test(transcript)) { selectMode('chat'); continueMode(); return true; }
        if (/\b(continue|proceed|go|confirm|next|enter)\b/.test(transcript)) { continueMode(); return true; }
    }
    else if (screen === 'screen-test') {
        if (/\b(cancel)\b/.test(transcript)) { continuousListen = false; goTo('screen-mode'); return true; }
        const m = parseTestCoords(transcript);
        if (m) { const r = +m[1], c = +m[2]; if (r <= 4 && c <= 4) { continuousListen = false; stopGlobalMic(); document.getElementById('test-target').value = `${r},${c}`; setTimeout(() => runTest(), 400); return true; } }
        return false;
    }
    else if (screen === 'screen-observe') {
        if (/\b(pause|stop|freeze)\b/.test(transcript)) { toggleObsPause(); return true; }
        if (/\b(resume|play|continue|unpause)\b/.test(transcript)) { if (obsPaused) toggleObsPause(); return true; }
        if (/\b(next|skip)\b/.test(transcript)) { if (obsStep < obsSteps.length - 1) { obsStep++; obsElapsed = 0; renderObsStep(obsStep); } return true; }
        if (/\b(start|begin|go|patrol)\b/.test(transcript)) { startObserveSequence(); return true; }
        const parsedDirs = normalizeDirections(transcript);
        if (parsedDirs.length > 0) { document.getElementById('obs-commands').value = parsedDirs; continuousListen = false; stopGlobalMic(); setTimeout(startObserveSequence, 500); return true; }
    }
    else if (screen === 'screen-vision') { return false; }
    else if (screen === 'screen-result') { if (/\b(again|retry|repeat|redo)\b/.test(transcript)) { goTo('screen-test'); return true; } }
    else if (screen === 'screen-chat') { if (!/\b(back|menu|home)\b/.test(transcript)) { document.getElementById('chat-input').value = transcript; sendChat(); return true; } }
    return false;
}

function toggleChatMic() { if (chatMicActive) stopChatMic(); else startChatMic(); }
function startChatMic() {
    const SR = window.SpeechRecognition || window.webkitSpeechRecognition; if (!SR) return;
    chatRecognition = buildSpeechRecognition();
    chatRecognition.onstart = () => { chatMicActive = true; document.getElementById('chat-mic-btn').classList.add('listening'); };
    chatRecognition.onresult = (e) => { document.getElementById('chat-input').value = e.results[0][0].transcript.trim(); sendChat(); };
    chatRecognition.onend = () => { chatMicActive = false; document.getElementById('chat-mic-btn').classList.remove('listening'); };
    chatRecognition.onerror = () => { chatMicActive = false; document.getElementById('chat-mic-btn').classList.remove('listening'); };
    chatRecognition.start();
}
function stopChatMic() { if (chatRecognition) { try { chatRecognition.stop(); } catch (e) { } } chatMicActive = false; document.getElementById('chat-mic-btn').classList.remove('listening'); }

// ════════════════════════════════════════════════════
// INIT
// ════════════════════════════════════════════════════
window.addEventListener('load', () => {
    ['obs-grid', 'test-grid', 'res-grid'].forEach(id => buildMiniGrid(document.getElementById(id)));
    const sz = gridTotalSize();
    ['obs-svg', 'test-svg', 'res-svg'].forEach(id => {
        const el = document.getElementById(id);
        el.setAttribute('width', sz); el.setAttribute('height', sz);
    });
    updateTractionInfo('obs');
    updateTractionInfo('test');
});

